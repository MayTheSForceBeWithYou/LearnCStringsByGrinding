# s01-e03-empty-wraps: answer

## The defect

(File and line. Quote the line. Which of the two functions is wrong, and which is the
correct one to compare it with?)

## The mechanism

(What is strlen("") ? What is that value minus one, in the type the expression has, and why?
How many iterations does the loop then run, and which byte does it read first that it should
not? Name the type and its rule, not just "it goes negative".)

## How the tool showed it

(The report's error kind, the access kind and size, the region size, and what "0 bytes after
1-byte region" says about which buffer was passed. Why does the plain build die without
printing a FAIL line for this case, and what does its exit status 139 mean?)

## The rule you take away

(One sentence about subtraction and unsigned types.)
