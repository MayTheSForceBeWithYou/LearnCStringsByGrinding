# s02-e05-struct-compare: answer

## The defect

(File and line. Quote the line. Say which of the three functions in the file is wrong, and
why the other use of a mem function in the same file is correct.)

## The mechanism

(Draw the eight bytes of a Point: which byte is `kind`, which four are `value`, and which are
neither. Then say what the two points in the first check hold in each of those bytes, and what
memcmp therefore returns. Why are the padding bytes allowed to differ at all -- what does the
standard say about them?)

## How the tool showed it

(No sanitizer report, and no compiler warning: say why each tool has nothing to say. Which
checks caught it, and what makes the last one -- the table search -- the shape this defect
takes in real programs?)

## The rule you take away

(One sentence about comparing structs.)
