/* utf8locale.h -- select a UTF-8 locale the same way on every machine.
 *
 * (Not named locale.h: lib/ is on the include path, and that name would hide the
 * standard <locale.h>.)
 *
 * Harnesses that test multibyte or wide-character functions call this first. It never
 * reads the environment (setlocale(LC_ALL, "") would make results depend on the
 * student's shell), and tries, in order: "C.UTF-8" (glibc), "en_US.UTF-8" (macOS,
 * most Linux installations), "UTF-8" (macOS, LC_CTYPE only).
 */
#ifndef COURSE_UTF8LOCALE_H
#define COURSE_UTF8LOCALE_H

/* Returns the name of the locale now in effect. If no UTF-8 locale exists, prints how
 * to install one and exits with EXIT_NO_LOCALE (5). */
const char *course_utf8_locale(void);

#endif
