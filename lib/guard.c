/* guard.c -- see guard.h. Part of lib/, so compiler-specific detection is allowed here.
 * Compiled without the dsalloc shim: these are real malloc/free calls. */
#include "guard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FENCE 32
#define FENCE_BYTE 0xA5
#define MAX_LIVE 64

#if defined(__SANITIZE_ADDRESS__)
#define GUARD_ASAN 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer)
#define GUARD_ASAN 1
#endif
#endif

#ifdef GUARD_ASAN
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#endif

static struct {
    char *p;
    size_t cap;
} live[MAX_LIVE];

static int slot_of(const char *p)
{
    for (int i = 0; i < MAX_LIVE; i++) {
        if (live[i].p == p && p != NULL)
            return i;
    }
    fprintf(stderr, "guard: %p is not a live guard_new() buffer (harness bug)\n", (const void *)p);
    exit(2);
}

char *guard_new(size_t cap)
{
    int s = -1;
    for (int i = 0; i < MAX_LIVE; i++) {
        if (live[i].p == NULL) {
            s = i;
            break;
        }
    }
    if (s < 0) {
        fprintf(stderr, "guard: more than %d live buffers (harness bug: guard_free them)\n", MAX_LIVE);
        exit(2);
    }
#ifdef GUARD_ASAN
    char *p = malloc(cap > 0 ? cap : 1);
#else
    char *base = malloc(cap + 2 * FENCE);
    char *p = base ? base + FENCE : NULL;
#endif
    if (p == NULL) {
        fprintf(stderr, "guard: out of memory in the harness\n");
        exit(2);
    }
#ifdef GUARD_ASAN
    if (cap == 0) {
        p[0] = GUARD_FILL;
        __asan_poison_memory_region(p, 1);
    }
#else
    memset(p - FENCE, FENCE_BYTE, FENCE);
    memset(p + cap, FENCE_BYTE, FENCE);
#endif
    live[s].p = p;
    live[s].cap = cap;
    guard_reset(p);
    return p;
}

char *guard_copy(const void *src, size_t n)
{
    char *p = guard_new(n);
    if (n > 0)
        memcpy(p, src, n);
    return p;
}

void guard_reset(char *p)
{
    size_t cap = live[slot_of(p)].cap;
    if (cap > 0)
        memset(p, GUARD_FILL, cap);
}

size_t guard_cap(const char *p)
{
    return live[slot_of(p)].cap;
}

int guard_ok(const char *p)
{
    int s = slot_of(p);
#ifdef GUARD_ASAN
    (void)s;
    return 1;
#else
    const unsigned char *lo = (const unsigned char *)p - FENCE;
    const unsigned char *hi = (const unsigned char *)p + live[s].cap;
    for (int i = 0; i < FENCE; i++) {
        if (lo[i] != FENCE_BYTE || hi[i] != FENCE_BYTE)
            return 0;
    }
    return 1;
#endif
}

void guard_free(char *p)
{
    int s = slot_of(p);
#ifdef GUARD_ASAN
    if (live[s].cap == 0)
        __asan_unpoison_memory_region(p, 1);
    free(p);
#else
    free(p - FENCE);
#endif
    live[s].p = NULL;
    live[s].cap = 0;
}
