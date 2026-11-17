/* s00-e01-literal-sizes: how big is it, and how long is it?
 *
 * Fill in answers.txt BEFORE you run this. Fourteen declarations; for each one the
 * program evaluates `sizeof` and, where the object really is a string, `strlen`, and
 * compares the result with your prediction. It tells you which lines are wrong, never
 * what the right value is.
 *
 * Two of the fourteen are not strings at all. Asking strlen about them would read past
 * the object, so the program never calls it: those two answers are the word `undefined`,
 * and they are graded by a hash of what you wrote.
 */
#include <stdio.h>
#include <string.h>

#include "answers.h"

/* Answers "<name>.sizeof" with the size of the object, in bytes. */
static void sz(const char *name, size_t size)
{
    char key[40];
    snprintf(key, sizeof key, "%s.sizeof", name);
    answer_int(key, (long)size);
}

/* Answers "<name>.strlen" with the length of the string s. */
static void len(const char *name, const char *s)
{
    char key[40];
    snprintf(key, sizeof key, "%s.strlen", name);
    answer_int(key, (long)strlen(s));
}

int main(int argc, char **argv)
{
    char a[] = "abc";
    const char *p = "abc";
    char b[3] = {'a', 'b', 'c'};      /* three characters, no terminator */
    char c[8] = "abc";
    char d[] = "ab" "cd";
    char e[] = "a\0b";
    char f[] = "";
    char g[] = {'h', 'i'};            /* two characters, no terminator */
    char h[] = {'h', 'i', '\0'};
    char m[3][8] = {"ab", "cd", "ef"};
    const char *v[] = {"a", "bb", "ccc"};
    const char *q = &"hello"[2];
    char r[] = "caf\xC3\xA9";
    char s[10] = "hi";

    answers_load(argc > 1 ? argv[1] : "answers.txt");

    sz("a", sizeof a);
    len("a", a);
    sz("p", sizeof p);
    len("p", p);
    sz("b", sizeof b);
    answer_hash("b.strlen", UINT32_C(0xa347316e)); /* not a string: strlen is undefined */
    sz("c", sizeof c);
    len("c", c);
    sz("d", sizeof d);
    len("d", d);
    sz("e", sizeof e);
    len("e", e);
    sz("f", sizeof f);
    len("f", f);
    sz("g", sizeof g);
    answer_hash("g.strlen", UINT32_C(0xc6dec497)); /* not a string: strlen is undefined */
    sz("h", sizeof h);
    len("h", h);
    sz("m", sizeof m);
    len("m", m[1]);
    sz("v", sizeof v);
    len("v", v[2]);
    sz("q", sizeof q);
    len("q", q);
    sz("r", sizeof r);
    len("r", r);
    sz("s", sizeof s);
    len("s", s);

    return answers_done("s00-e01-literal-sizes");
}
