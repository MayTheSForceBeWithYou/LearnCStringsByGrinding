/* copypol.h -- s03-e06-copy-policy: three bounded copies, three policies.
 *
 * DESIGN exercise: the prototypes and the return codes below are fixed, and the
 * harness tests them. Everything marked CONTRACT is yours to write, BEFORE you write
 * any code: replace each "CONTRACT: ..." line with the sentence that settles it.
 * The README lists the questions each contract must answer.
 */
#ifndef COPYPOL_H
#define COPYPOL_H

#include <stddef.h>

/* In all three: `cap` is the total size of dst in bytes, terminator included. */

#define COPY_OK 0         /* the whole source was copied; dst is a string        */
#define COPY_TRUNCATED 1  /* copy_trunc only: dst holds a shortened string       */
#define COPY_NO_ROOM (-1) /* nothing useful could be stored (see each function)  */

/* Policy "truncate and say so": store as much of the string src as fits.
 *
 * Fixed:    COPY_OK if src fits (strlen(src) < cap).
 *           COPY_TRUNCATED if it does not and cap > 0: dst holds the first cap - 1
 *           characters of src and a terminator.
 *           COPY_NO_ROOM if cap == 0: dst is not touched.
 * CONTRACT: which bytes of dst may this function write, and which does it leave alone?
 * CONTRACT: what does it require of src and of dst (NULL? overlap? unterminated src?)
 * CONTRACT: when is truncation acceptable for a caller, and what must a caller do
 *           with COPY_TRUNCATED?
 */
int copy_trunc(char *dst, size_t cap, const char *src);

/* Policy "all or nothing": store the string src whole, or store nothing.
 *
 * Fixed:    COPY_OK if src fits (strlen(src) < cap).
 *           COPY_NO_ROOM if it does not: if cap > 0, dst is the empty string
 *           (dst[0] == '\0'); if cap == 0, dst is not touched.
 * CONTRACT: on COPY_NO_ROOM, what happens to the bytes of dst after dst[0]?
 * CONTRACT: what does it require of src and of dst?
 * CONTRACT: why the empty string on failure, rather than leaving dst as it was?
 */
int copy_strict(char *dst, size_t cap, const char *src);

/* Policy "all or nothing", for a source whose length is already known: store the n
 * characters at src, followed by a terminator.
 *
 * Fixed:    src need NOT be terminated. Exactly n bytes of src are read; src[n] is
 *           never read.
 *           COPY_OK if n < cap: dst holds those n characters and a terminator.
 *           COPY_NO_ROOM otherwise: if cap > 0, dst is the empty string; if
 *           cap == 0, dst is not touched.
 * CONTRACT: what if one of the n bytes at src is a NUL?
 * CONTRACT: what does it require of src and of dst?
 * CONTRACT: name one situation in which the caller has n but no terminator.
 */
int copy_n(char *dst, size_t cap, const char *src, size_t n);

#endif
