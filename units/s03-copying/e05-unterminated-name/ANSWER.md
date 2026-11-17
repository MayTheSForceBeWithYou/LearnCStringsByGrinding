# s03-e05-unterminated-name: answer

## The defect

(File and line. Quote the line.)

## The mechanism

(Which bytes does the call write for the name "Maximilienne"? Which byte is missing? Which
later call reads past the field, and why does it not stop?)

## How the tool showed it

(The first line of the ASan report, and what each of these tells you: the error kind, READ or
WRITE, the size, the frame at the top of the stack, the "is located" line, the region size.
Why does the plain build not crash?)

## The rule you take away

(One sentence you would say to someone reviewing a `strncpy` call.)
