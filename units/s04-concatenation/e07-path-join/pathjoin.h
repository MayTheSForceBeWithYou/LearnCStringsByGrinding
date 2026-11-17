/* pathjoin.h -- s04-e07-path-join: joining path parts on the heap.
 *
 * Both functions return one owned string, which the caller frees with free(), or NULL
 * if an allocation failed. Nothing is allocated and left behind on a failure: these
 * functions are all-or-nothing.
 *
 * The separator rule is the one every path joiner needs and most get wrong: exactly
 * one '/' between two parts, whatever the parts end and start with. So "a/" + "b",
 * "a" + "/b" and "a/" + "/b" all give "a/b".
 */
#ifndef PATHJOIN_H
#define PATHJOIN_H

#include <stddef.h>

/* Joins two parts with exactly one separator between them.
 *
 * `dir` and `name` are borrowed strings, neither NULL. The exact rule:
 *
 *   - trailing '/' characters of a part are dropped, and so are the leading ones of
 *     every part except the first, so "a/" + "b", "a" + "/b" and "a//" + "//b" all
 *     give "a/b";
 *   - the FIRST part keeps its leading slashes, so an absolute path stays absolute:
 *     "/" + "b" is "/b" and "/usr/" + "bin" is "/usr/bin";
 *   - a part with nothing left after that trimming contributes nothing and no
 *     separator: "" + "b" is "b", "a" + "" is "a", "" + "" is "", and "" + "/" is "";
 *   - a separator is written before a part's text only when something has already been
 *     written and it did not end in '/'.
 *
 * Returns one owned string, or NULL if allocation failed. */
char *path_join(const char *dir, const char *name);

/* Joins n parts the same way, left to right. n may be 0, which gives the empty string.
 * `parts` is an array of n borrowed strings, none NULL.
 * Returns one owned string, or NULL if allocation failed. */
char *path_join_n(const char *const *parts, size_t n);

#endif
