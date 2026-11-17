/* s00-e03-literal-write: harness. The defect is in prefix.c, not here. */
#include <stdio.h>
#include <string.h>

#include "course.h"
#include "prefix.h"
#include "strcheck.h"
#include "upcase.h"

/* upcase on storage that may be written: this is the module working as documented. */
static void upcase_case(const char *before, const char *after)
{
    char *buf = guard_copy(before, strlen(before) + 1);

    upcase(buf);
    check_str(buf, after, "upcase upper-cases the letters and nothing else");
    check_guard(buf, "upcase writes nothing outside the string");
    guard_free(buf);
}

static void prefix_case(int level, const char *want)
{
    size_t need = strlen(want) + 1;
    char *out = guard_new(need);

    check(prefix_write(out, need, level) == 0, "prefix_write succeeds in a buffer of the exact size");
    check_str(out, want, "prefix_write writes the level name in capitals, a colon and a space");
    check_guard(out, "prefix_write writes nothing outside the buffer");
    guard_free(out);

    /* Writing a prefix must not change the name the module hands out. */
    check_str(prefix_name(level), prefix_name(level), "prefix_name returns a string");
}

int main(void)
{
    upcase_case("hello", "HELLO");
    upcase_case("", "");
    upcase_case("Mixed Case 42!", "MIXED CASE 42!");
    upcase_case("caf\xC3\xA9", "CAF\xC3\xA9"); /* bytes above 0x7F are left alone */

    check_str(prefix_name(PREFIX_INFO), "info", "the info level is named \"info\"");
    check_str(prefix_name(PREFIX_WARN), "warning", "the warning level is named \"warning\"");
    check_str(prefix_name(PREFIX_ERROR), "error", "the error level is named \"error\"");

    prefix_case(PREFIX_WARN, "WARNING: ");
    prefix_case(PREFIX_INFO, "INFO: ");
    prefix_case(PREFIX_ERROR, "ERROR: ");

    /* The level names are this module's own data, and reading them twice must give the
     * same bytes both times, whatever anyone has printed in between. */
    check_str(prefix_name(PREFIX_WARN), "warning", "prefix_name still says \"warning\" afterwards");
    check_str(prefix_name(PREFIX_INFO), "info", "prefix_name still says \"info\" afterwards");

    {   /* A buffer one byte too small: nothing is written and -1 comes back. */
        char *out = guard_new(8);
        check(prefix_write(out, 8, PREFIX_WARN) == -1, "a 9-character prefix does not fit in 8 bytes");
        check_bytes(out, "########", 8, "a prefix that does not fit leaves the buffer untouched");
        check(prefix_write(out, 0, PREFIX_WARN) == -1, "cap == 0 does not fit either");
        check_bytes(out, "########", 8, "cap == 0 leaves the buffer untouched");
        guard_free(out);
    }

    return checks_done("s00-e03-literal-write");
}
