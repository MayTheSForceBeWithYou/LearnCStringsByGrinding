# s03-e07-struct-neighbour: answer

## The defect

(File and line. Quote the line.)

## The mechanism

(Draw the 24 bytes of the Account before and after account_set_user(&a,
"mallory_________admin"). Which bytes of which field did the call write?)

## How the tool showed it

(It did not. Explain why AddressSanitizer reports nothing here, although it reported the
overflow in s03-e05 and would report this strcpy if `user` were a 16-byte array of its own.
Which check caught it instead?)

## The rule you take away

(One sentence about what "ASan is clean" does and does not tell you.)
