# s02-e04-memset-args: answer

## The defect

(File and line. Quote the line, and write out memset's parameter list in order next to it.)

## The mechanism

(What does the call actually ask memset to do: which byte value, and how many bytes? What is
in the slot afterwards, and why do the first three checks fail in the way they do? Say what
would have happened if the two arguments had been 12 and 65 rather than 24 and 0 -- that is,
why a swap is not always a no-op.)

## How the tool showed it

(No sanitizer report: say why. Then say what gcc prints when the same swap is written with
the arguments in plain sight, and why it says nothing about the version in this file. The
lesson's *What the tools say* has the warning and its flag.)

## The rule you take away

(One sentence about memset's argument order, or about what a wrapper function hides.)
