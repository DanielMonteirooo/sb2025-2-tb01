; teste

       LOAD    N
FAT:   SUB     ONE
       JMPZ   FIM
       STORE   AUX
       MULT    N
       STORE   N
       LOAD    AUXILIAR  ; nome errado
       JMP    FAT
FIM:   LOAD N
       SUB FINAL
       JMPZ  OK
OK:    STOP
AUX:   SPACE
N:     CONST  5
FINAL: CONST 120
ONE:   CONST   1
