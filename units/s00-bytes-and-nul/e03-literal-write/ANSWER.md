# s00-e03-literal-write: answer

## The defect

(File and line. Quote the line. Say which of the two files is at fault and which is the
innocent one.)

## The mechanism

(Where do the bytes of "warning" live, and what put them there? What exactly does upcase try
to do to them, and what stops it? Name the storage, not just "it crashes".)

## How the tool showed it

(Which signal, which access kind, and what the top two frames of the report say. Explain why
frame #0 is in upcase.c although upcase is correct, and what the plain build's exit status
139 means. Also: which check in main.c would have failed even on a system where the write
had been allowed to succeed?)

## The rule you take away

(One sentence about string literals and writable storage.)
