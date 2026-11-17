# s01-e06-narrow-length: answer

## The defect

(File and line. Quote the line. Say which of the two constants in `name.h` the test should
have been against, and why the other one is the wrong bound.)

## The mechanism

(For a 300-character name: what does strlen return, what ends up in `len`, and why that
number? Give the arithmetic. Then say what `name_len` and `name_get` report afterwards, and
which bytes of `buf` are now unreachable through the record's own API.)

## How the tool showed it

(It did not. Say why AddressSanitizer, UBSan and the compiler all stay silent: every read and
every write is inside an object, and the conversion that loses the value is not an error in C.
Then say which check caught it, and what a program with no such check would have shipped.)

## The rule you take away

(One sentence about storing a length in a type narrower than size_t.)
