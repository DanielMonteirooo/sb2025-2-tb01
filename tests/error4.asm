; teste

       LOAD    N
FAT:   SUB     ONE
       JMPZ   FIM
       STORE  ADD ; palavra reservada
       MULT    N
       STORE  N
       LOAD   ADD
       JMP    FAT
FIM:   LOAD N
       SUB FINAL
       JMPZ  OK
       OK:    STOP
ADD:   SPACE
N:     CONST  5
FINAL: CONST 120
ONE:   CONST   1
