LOAD    N
FAT:   SUB     ONE
       JMPZ    FIM
       STORE   AUX
       MULT     N
       STORE   N
       LOAD    AUX
       JMP     FAT
FIM:   LOAD    N
       SUB     FINAL
       JMPZ    OK
ERRO:  JMP     ERRO  ; Trava aqui se o resultado estiver errado
OK:    STOP
AUX:   SPACE
N:     CONST   5
FINAL: CONST   120
ONE:   CONST   1