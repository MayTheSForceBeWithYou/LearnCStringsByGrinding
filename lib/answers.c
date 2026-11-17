/* answers.c -- see answers.h. Plain C17. */
#include "answers.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytes.h"

#define MAX_KEYS 128
#define KEY_LEN 64
#define VAL_LEN 512

static struct {
    char key[KEY_LEN];
    char val[VAL_LEN];
    int asked;
} ans[MAX_KEYS];
static int n_ans;
static int n_asked, n_ok;
static int load_failed;

static char *trim(char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
        n--;
    s[n] = '\0';
    return s;
}

int answers_load(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        printf("answers: cannot open %s: %s\n", path, strerror(errno));
        printf("answers: run the program from the exercise directory (make run / make check)\n");
        load_failed = 1;
        return -1;
    }
    char line[KEY_LEN + VAL_LEN + 16];
    int lineno = 0, rc = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        lineno++;
        char *s = trim(line);
        if (*s == '\0' || *s == '#')
            continue;
        char *eq = strchr(s, '=');
        if (eq == NULL) {
            printf("answers: %s:%d: expected \"key = value\"\n", path, lineno);
            rc = -1;
            continue;
        }
        *eq = '\0';
        char *key = trim(s), *val = trim(eq + 1);
        if (n_ans == MAX_KEYS || strlen(key) >= KEY_LEN || strlen(val) >= VAL_LEN) {
            printf("answers: %s:%d: key or value too long\n", path, lineno);
            rc = -1;
            continue;
        }
        int at = n_ans;
        for (int i = 0; i < n_ans; i++) {
            if (strcmp(ans[i].key, key) == 0)
                at = i; /* a repeated key: the last line wins */
        }
        strcpy(ans[at].key, key);
        strcpy(ans[at].val, val);
        if (at == n_ans)
            n_ans++;
    }
    fclose(f);
    if (rc != 0)
        load_failed = 1;
    return rc;
}

/* Returns the student's value for key, "" if blank, NULL if the key is not in the file. */
static const char *lookup(const char *key)
{
    for (int i = 0; i < n_ans; i++) {
        if (strcmp(ans[i].key, key) == 0) {
            ans[i].asked = 1;
            return ans[i].val;
        }
    }
    return NULL;
}

static void verdict(const char *key, const char *given, int right)
{
    n_asked++;
    const char *word;
    if (given == NULL)
        word = "missing (the key is not in answers.txt)";
    else if (*given == '\0')
        word = "blank";
    else if (right) {
        word = "ok";
        n_ok++;
    } else
        word = "WRONG";
    printf("  %-28s %s\n", key, word);
}

void answer_str(const char *key, const char *truth)
{
    const char *given = lookup(key);
    verdict(key, given, given != NULL && strcmp(given, truth) == 0);
}

void answer_int(const char *key, long truth)
{
    const char *given = lookup(key);
    int right = 0;
    if (given != NULL && *given != '\0') {
        char *end;
        errno = 0;
        long v = strtol(given, &end, 10);
        right = (errno == 0 && end != given && *end == '\0' && v == truth);
    }
    verdict(key, given, right);
}

void answer_bytes(const char *key, const void *p, size_t n)
{
    char truth[VAL_LEN];
    if (bytes_notation(p, n, truth, sizeof truth) >= sizeof truth) {
        fprintf(stderr, "answers: %s: value too long for the grader (harness bug)\n", key);
        exit(2);
    }
    answer_str(key, truth);
}

uint32_t answers_hash(const char *s)
{
    while (isspace((unsigned char)*s))
        s++;
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1]))
        n--;
    uint32_t h = UINT32_C(2166136261);
    for (size_t i = 0; i < n; i++) {
        h ^= (uint32_t)tolower((unsigned char)s[i]);
        h *= UINT32_C(16777619);
    }
    return h;
}

/* The key is mixed in, so that "no" under one key and "no" under another hash differently. */
static uint32_t keyed_hash(const char *key, const char *given)
{
    char both[KEY_LEN + VAL_LEN + 2];
    snprintf(both, sizeof both, "%s=%s", key, given);
    return answers_hash(both);
}

void answer_hash(const char *key, uint32_t hash)
{
    const char *given = lookup(key);
    verdict(key, given, given != NULL && *given != '\0' && keyed_hash(key, given) == hash);
    if (given != NULL && *given != '\0' && getenv("ANSWERS_SHOW_HASH") != NULL)
        printf("  %-28s hash of your answer: UINT32_C(0x%08lx)\n", "", (unsigned long)keyed_hash(key, given));
}

int answers_done(const char *id)
{
    int unknown = 0;
    for (int i = 0; i < n_ans; i++) {
        if (!ans[i].asked) {
            printf("  %-28s unknown key (check the spelling)\n", ans[i].key);
            unknown++;
        }
    }
    if (!load_failed && unknown == 0 && n_ok == n_asked) {
        printf("%s: all checks passed\n", id);
        return 0;
    }
    printf("%s: %d of %d answers correct\n", id, n_ok, n_asked);
    return 1;
}
