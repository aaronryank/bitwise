# commands

    |  command  |  arguments  |  description                                                                  |
    -----------------------------------------------------------------------------------------------------------
    |  IN       |  3          |  read a character into arg1 if arg2 nonzero, store result in arg3             |
    |  OUT      |  3          |  print arg1 as ASCII if arg2 nonzero, store result in arg3                    |
    |  AND      |  3          |  compute bitwise AND of arg1 and arg2, store result in arg3                   |
    |  OR       |  3          |  compute bitwise OR of arg1 and arg2, store result in arg3                    |
    |  XOR      |  3          |  compute bitwise EXCLUSIVE OR of arg1 and arg2, store result in arg3          |
    |  SL       |  3          |  shift arg1 left by arg2 places, store result in arg3                         |
    |  SR       |  3          |  shift arg1 right by arg2 places, store result in arg3                        |
    |  LSR      |  3          |  logical-shift arg1 right by arg2 places, store result in arg3                |
    |  MOV      |  3          |  move arg2 into arg1 if arg3 is nonzero                                       |
    |  JMP      |  2          |  jump arg1 lines in the code (or to the point in label arg1) if arg2 nonzero  |
    |  NOT      |  2          |  compute logical NOT of arg1, store result in arg2                            |
    |  LABEL    |  1          |  create label with name arg1 at current point                                 |
    |  FRAME    |  0          |  new application frame                                                        |
    |  DEFRAME  |  0          |  back to previous application frame                                           |

# syntax

    COMMAND ARG1 ARG2 ARG3

Argument types:

 - `1` - register 1
 - `&1` - literal 1
 - `*1` - frame register 1
 - `@1` - label 1
 - `-1` - immediately discarded 1

# functions

    .FUNC_NAME:
    ....
    RET RETURN_VALUE

Call as you would any builtin function.

# notes

JMP functionality:

    |  arg1  |  description            |
    |  -1    |  re-reads current line  |
    |  0     |  exits                  |
    |  1     |  skips next line        |

FRAME and DEFRAME create a new set of temporary registers, called "frame registers". So this code:

    MOV *1 &48 &1
    FRAME
    MOV *1 &49 &1
    OUT *1 &1
    DEFRAME
    OUT *1 &1

will print `10`, while this code:

    MOV *1 &48 &1
    MOV *1 &49 &1
    OUT *1 &1
    OUT *1 &1

will print `11`.
