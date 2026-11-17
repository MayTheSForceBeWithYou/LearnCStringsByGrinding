# s04-e04-strncat-size: answer

## The defect

(File and line. Quote the line, and write next to it what strncat's third parameter means
according to its card and its manual page.)

## The mechanism

(For the case `label_append(dst, 8, "gh")` with dst holding "abcdef": how many bytes does
strncat write, where does the last one go, and how far past the buffer is it? Give the
formula for the correct third argument and explain each term, including why there is a -1.)

## How the tool showed it

(The ASan report's error kind, access kind and size; which frame is the interceptor and which
is the first line of course code; and what "0 bytes after 8-byte region" says about which
buffer. Also: why does the plain build fail with FAIL lines rather than a crash, and what did
check_guard add that check_str did not?)

## The rule you take away

(One sentence about the n of strncat.)
