# Sweet

A simple programming language

## Grammar

```
program         := <statement>*
statement       := <assignStatement>
                 | <labelStatement>
                 | <gotoStatement>
                 | <ifStatement>
                 | <printStatement>
assignStatement := <variable> "=" <expression> ";"
labelStatement  := "label" <variable> ";"
gotoStatement   := "goto" <variable> ";"
ifStatement     := "if" "(" <expression> ")" <statement>
printStatement  := "print" <expression> ";"
expression      := <primary> <operator> <primary>
                 | <primary>
primary         := <variable> | <literal>
operator        := "+" | "-" | "/" | "*" | "<" | "<=" | ">" | ">=" | "=="
variable        := [a-zA-Z_][0-9a-zA-Z_]+
literal         := [0-9]+
```

## Example

### print from 0 to 10 (inclusive)
```
a = 0;
label start;
print a;
if (a <= 10) goto start;
```