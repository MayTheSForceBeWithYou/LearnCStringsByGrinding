/* dsalloc.c -- see dsalloc.h. Compiled WITHOUT the forced include and with
 * DSALLOC_NO_MACROS, so malloc/free here are the real allocator's (ASan's in the
 * sanitizer build). Plain C11 + POSIX; single-threaded, like every course program.
 */
#define DSALLOC_NO_MACROS
#include "dsalloc.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define DS_EXIT_LEAK 4
#define DS_MAX_SITES 16

typedef struct {
    void *p;            /* NULL = empty slot, DS_TOMB = deleted slot */
    size_t n;
    const char *file;
    int line;
} ds_entry;

static char ds_tomb_byte;
#define DS_TOMB ((void *)&ds_tomb_byte)

static ds_entry *ds_tab;
static size_t ds_cap;           /* slots, power of two */
static size_t ds_used;          /* live + tombstones */
static size_t ds_live;
static size_t ds_bytes;

static unsigned long ds_calls;
static unsigned long ds_fail_at;      /* from FAIL_AT; 0 = never */
static unsigned long ds_fail_call;    /* from ds_fail_nth; absolute call number, 0 = never */
static int ds_count_at_exit;
static int ds_check_leaks = 1;
static int ds_ready;

static void ds_die(const char *msg)
{
    fprintf(stderr, "dsalloc: %s\n", msg);
    fflush(NULL);
    abort();
}

static size_t ds_hash(const void *p)
{
    uintptr_t x = (uintptr_t)p;
    x ^= x >> 17;
    x *= (uintptr_t)0x9E3779B97F4A7C15ull;
    x ^= x >> 29;
    return (size_t)x;
}

/* Index of p's slot, or of the slot where p would go (first tombstone or empty). */
static size_t ds_find(const void *p, int *found)
{
    size_t mask = ds_cap - 1, i = ds_hash(p) & mask, tomb = SIZE_MAX;
    for (;;) {
        void *q = ds_tab[i].p;
        if (q == NULL) {
            *found = 0;
            return tomb != SIZE_MAX ? tomb : i;
        }
        if (q == DS_TOMB) {
            if (tomb == SIZE_MAX)
                tomb = i;
        } else if (q == p) {
            *found = 1;
            return i;
        }
        i = (i + 1) & mask;
    }
}

static void ds_rehash(size_t new_cap)
{
    ds_entry *old = ds_tab;
    size_t old_cap = ds_cap;
    ds_tab = calloc(new_cap, sizeof *ds_tab);
    if (!ds_tab)
        ds_die("cannot grow the live-block table (out of memory)");
    ds_cap = new_cap;
    ds_used = 0;
    for (size_t i = 0; i < old_cap; i++) {
        if (old[i].p && old[i].p != DS_TOMB) {
            int found;
            size_t j = ds_find(old[i].p, &found);
            ds_tab[j] = old[i];
            ds_used++;
        }
    }
    free(old);
}

static void ds_track(void *p, size_t n, const char *file, int line)
{
    if ((ds_used + 1) * 10 >= ds_cap * 7)
        ds_rehash(ds_cap ? (ds_live * 2 >= ds_cap / 2 ? ds_cap * 2 : ds_cap) : 64);
    int found;
    size_t i = ds_find(p, &found);
    if (found)
        ds_die("allocator returned a pointer that is already live (heap corrupted?)");
    if (ds_tab[i].p == NULL)
        ds_used++;
    ds_tab[i].p = p;
    ds_tab[i].n = n;
    ds_tab[i].file = file;
    ds_tab[i].line = line;
    ds_live++;
    ds_bytes += n;
}

/* Removes p; returns 1 if it was tracked. Untracked pointers came from libc
 * (getline, ...) or from a call that bypassed the macros: they are freed but not counted. */
static int ds_untrack(void *p)
{
    if (!ds_cap)
        return 0;
    int found;
    size_t i = ds_find(p, &found);
    if (!found)
        return 0;
    ds_bytes -= ds_tab[i].n;
    ds_live--;
    ds_tab[i].p = DS_TOMB;
    return 1;
}

static int ds_site_cmp(const ds_entry *a, const ds_entry *b)
{
    int c = strcmp(a->file ? a->file : "", b->file ? b->file : "");
    return c ? c : (a->line > b->line) - (a->line < b->line);
}

static void ds_at_exit(void)
{
    if (ds_count_at_exit)
        fprintf(stderr, "dsalloc: %lu allocation calls\n", ds_calls);
    if (!ds_check_leaks || ds_live == 0)
        return;
    fflush(stdout);
    fprintf(stderr, "dsalloc: LEAK: %zu block(s), %zu byte(s) still allocated at exit\n",
            ds_live, ds_bytes);
    /* Group by allocation site; print at most DS_MAX_SITES sites. */
    ds_entry sites[DS_MAX_SITES];
    size_t counts[DS_MAX_SITES], nsites = 0, more = 0;
    for (size_t i = 0; i < ds_cap; i++) {
        ds_entry *e = &ds_tab[i];
        if (!e->p || e->p == DS_TOMB)
            continue;
        size_t k = 0;
        while (k < nsites && ds_site_cmp(&sites[k], e) != 0)
            k++;
        if (k == nsites) {
            if (nsites == DS_MAX_SITES) {
                more++;
                continue;
            }
            sites[k] = *e;
            sites[k].n = 0;
            counts[k] = 0;
            nsites++;
        }
        sites[k].n += e->n;
        counts[k]++;
    }
    for (size_t k = 0; k < nsites; k++)
        fprintf(stderr, "dsalloc:   %zu byte(s) in %zu block(s) allocated at %s:%d\n",
                sites[k].n, counts[k], sites[k].file ? sites[k].file : "?", sites[k].line);
    if (more)
        fprintf(stderr, "dsalloc:   ... and %zu more block(s) from other sites\n", more);
    fflush(NULL);
    _exit(DS_EXIT_LEAK);
}

static void ds_init(void)
{
    if (ds_ready)
        return;
    ds_ready = 1;
    const char *s = getenv("FAIL_AT");
    if (s && *s)
        ds_fail_at = strtoul(s, NULL, 10);
    s = getenv("FA_COUNT");
    ds_count_at_exit = s && strcmp(s, "1") == 0;
    s = getenv("DSALLOC_LEAKS");
    if (s && strcmp(s, "0") == 0)
        ds_check_leaks = 0;
    if (atexit(ds_at_exit) != 0)
        ds_die("atexit failed");
}

/* Runs before main: registers the exit check early, so it runs after the
 * program's own atexit handlers (which may free things) and before LSan's. */
__attribute__((constructor)) static void ds_ctor(void) { ds_init(); }

/* Counts one allocation call; returns 1 if this call must fail. */
static int ds_should_fail(void)
{
    ds_init();
    ds_calls++;
    if ((ds_fail_at && ds_calls == ds_fail_at) || (ds_fail_call && ds_calls == ds_fail_call)) {
        if (ds_calls == ds_fail_call)
            ds_fail_call = 0;
        errno = ENOMEM;
        return 1;
    }
    return 0;
}

void *ds_malloc_at(size_t n, const char *file, int line)
{
    if (ds_should_fail())
        return NULL;
    void *p = malloc(n);
    if (p)
        ds_track(p, n, file, line);
    return p;
}

void *ds_calloc_at(size_t n, size_t size, const char *file, int line)
{
    if (ds_should_fail())
        return NULL;
    void *p = calloc(n, size);
    if (p)
        ds_track(p, n * size, file, line);  /* calloc checked n * size for overflow */
    return p;
}

void *ds_realloc_at(void *p, size_t n, const char *file, int line)
{
    if (p && n == 0) {
        fprintf(stderr, "dsalloc: realloc(p, 0) at %s:%d: not allowed in this course "
                        "(implementation-defined in C17, undefined in C23); free(p) instead\n",
                file, line);
        fflush(NULL);
        abort();
    }
    if (ds_should_fail())
        return NULL;                /* the old block stays alive and tracked */
    if (!p) {
        void *q = malloc(n);
        if (q)
            ds_track(q, n, file, line);
        return q;
    }
    int found = 0;
    size_t slot = ds_cap ? ds_find(p, &found) : 0;
    void *q = realloc(p, n);
    if (!q)
        return NULL;                /* still alive, still tracked as before */
    if (found) {                    /* p is gone now; retire its slot without touching p */
        ds_bytes -= ds_tab[slot].n;
        ds_live--;
        ds_tab[slot].p = DS_TOMB;
        ds_track(q, n, file, line);
    }
    return q;
}

char *ds_strdup_at(const char *s, const char *file, int line)
{
    size_t n = strlen(s) + 1;
    char *d = ds_malloc_at(n, file, line);
    if (d)
        memcpy(d, s, n);
    return d;
}

char *ds_strndup_at(const char *s, size_t n, const char *file, int line)
{
    size_t len = 0;
    while (len < n && s[len])
        len++;
    char *d = ds_malloc_at(len + 1, file, line);
    if (d) {
        memcpy(d, s, len);
        d[len] = '\0';
    }
    return d;
}

void ds_free(void *p)
{
    if (!p)
        return;
    ds_untrack(p);
    free(p);
}

size_t ds_live_blocks(void) { return ds_live; }
size_t ds_live_bytes(void) { return ds_bytes; }
unsigned long ds_alloc_calls(void) { return ds_calls; }

void ds_fail_nth(unsigned long n)
{
    ds_init();
    ds_fail_call = n ? ds_calls + n : 0;
}
