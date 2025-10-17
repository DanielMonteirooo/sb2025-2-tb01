FOO:     MACRO &1, &2 ; OI :)
         LOAD &1
         ADD &2
         STORE &1
         ENDMACRO
BAR:     MACRO &1 ; Olá :)
         ADD UM
         MUL &1
         STORE &1
         ENDMACRO
INICIO:  LOAD NUM ; Tudo bem?
         BAR NUM
         ADD UM
         STORE NUM
         FOO NUM, UM ; Com você? 
         STOP
NUM:     SPACE 1
UM:      CONST 1