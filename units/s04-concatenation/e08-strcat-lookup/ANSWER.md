# s04-e08-strcat-lookup: the sentences

For every key in `answers.txt`: the page and the section you found the answer in, and the
sentence (or formula) that supports it, quoted exactly. Say which system's pages you are
quoting, and note any key your pages do not answer.

Quoted from: (Linux man-pages version, or macOS release)

## strcat.returns

Page and section:

>

## strncat.result

Page and section:

>

## strncat.bytes_written

Page and section:

>

## strncat.short_src

Page and section:

>

## strcat.need

Page and section, with the formula quoted:

>

## strncat.need

Page and section, with the formula quoted:

>

## strncat.max_bytes

Page and section:

>

## strncat.reports_truncation

Page and section:

>

## strlcat.in_posix_2008

Page and section:

>

## Two questions with no key (answer in a sentence each)

1. The Linux `strcat(3)` page says `strcat(dst, src)` "is equivalent to" a one-line expression
   using other functions. Quote it, and say what that equivalence tells you about the cost of
   `strcat` in a loop.

2. `strncat`'s third argument bounds the source, not the destination. Write the expression a
   caller must pass to append safely into a buffer of `cap` bytes that already holds a string,
   and explain each term.
