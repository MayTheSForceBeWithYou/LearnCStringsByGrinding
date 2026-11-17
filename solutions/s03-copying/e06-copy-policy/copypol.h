/* copypol.h -- s03-e06-copy-policy: three bounded copies, three policies.
 *
 * Reference contracts. Yours will be worded differently; what matters is that each
 * question has one answer and the code agrees with it.
 */
#ifndef COPYPOL_H
#define COPYPOL_H

#include <stddef.h>

/* In all three: `cap` is the total size of dst in bytes, terminator included.
 * In all three: dst and src must be valid, non-NULL pointers (not checked), and the
 * buffers must not overlap. src is borrowed and never modified. Nothing is allocated. */

#define COPY_OK 0         /* the whole source was copied; dst is a string        */
#define COPY_TRUNCATED 1  /* copy_trunc only: dst holds a shortened string       */
#define COPY_NO_ROOM (-1) /* nothing useful could be stored (see each function)  */

/* Policy "truncate and say so": store as much of the string src as fits.
 *
 * Returns:  COPY_OK if src fits (strlen(src) < cap).
 *           COPY_TRUNCATED if it does not and cap > 0: dst holds the first cap - 1
 *           characters of src and a terminator.
 *           COPY_NO_ROOM if cap == 0: dst is not touched.
 * Writes:   min(strlen(src), cap - 1) + 1 bytes starting at dst[0]: the characters and
 *           one terminator. Bytes of dst after the terminator are never written (no
 *           padding). With cap > 0, dst is a string on return, whatever the result.
 * Requires: src is a string (it is read up to its terminator to measure it).
 * Use when: a shortened value is still a usable value: a log line, a label, a title
 *           for display. A caller must not ignore COPY_TRUNCATED when the value is an
 *           identifier (a path, a user name, a key): a cut identifier names something
 *           else. Such callers want copy_strict.
 */
int copy_trunc(char *dst, size_t cap, const char *src);

/* Policy "all or nothing": store the string src whole, or store nothing.
 *
 * Returns:  COPY_OK if src fits (strlen(src) < cap).
 *           COPY_NO_ROOM if it does not: if cap > 0, dst is the empty string
 *           (dst[0] == '\0'); if cap == 0, dst is not touched.
 * Writes:   on COPY_OK, strlen(src) + 1 bytes. On COPY_NO_ROOM with cap > 0, exactly
 *           one byte, dst[0]; the bytes after it keep their old values and must not
 *           be relied on.
 * Requires: src is a string.
 * Why "":   a caller that forgets to test the result then works with an empty string,
 *           which is visibly wrong, rather than with the previous contents of dst,
 *           which look valid and belong to an earlier request. The old value is NOT
 *           preserved: a caller that needs it must copy into a scratch buffer first.
 */
int copy_strict(char *dst, size_t cap, const char *src);

/* Policy "all or nothing", for a source whose length is already known: store the n
 * characters at src, followed by a terminator.
 *
 * Returns:  COPY_OK if n < cap: dst holds those n characters and a terminator.
 *           COPY_NO_ROOM otherwise (including any n so large that n + 1 would
 *           overflow): if cap > 0, dst is the empty string; if cap == 0, dst is not
 *           touched.
 * Reads:    exactly n bytes of src. src need NOT be terminated, and src[n] is never
 *           read. With n == 0 src is not read at all.
 * Writes:   on COPY_OK, n + 1 bytes. On COPY_NO_ROOM with cap > 0, dst[0] only.
 * NUL in the n bytes: copied like any other byte. dst then holds n + 1 bytes of which
 *           a str* function will see only the part before the first NUL. The caller
 *           asked for n bytes and gets n bytes; searching for a terminator would make
 *           this a different (and slower) function.
 * Use when: the text is a slice of something larger and has no terminator of its own:
 *           a token found with strcspn, a field of a fixed-width record, the part of a
 *           path before the last '/', a (pointer, length) string view.
 */
int copy_n(char *dst, size_t cap, const char *src, size_t n);

#endif
