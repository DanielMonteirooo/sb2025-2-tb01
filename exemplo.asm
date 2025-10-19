FOO:     MACRO &1, &2 ; OI :)
         LOAD &1
         ADD &2
         STORE &1
         ENDMACRO
BAR:     MACRO &1 ; Olá :)
         ADD CINCO
         MULT &1
         STORE &1
         ENDMACRO
INICIO:  LOAD DOIS ; Tudo bem?
         BAR DOIS
         ADD CINCO
         STORE DOIS
         FOO DOIS, CINCO ; Com você? 
         STOP
DOIS:    SPACE 2
CINCO:   CONST 5