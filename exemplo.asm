FOO:     MACRO &FOO, &BAR ; OI :)
         LOAD &FOO
         ADD &BAR
         STORE &FOO
         ENDMACRO
BAR:     MACRO &1 ; Olá :)
         ADD &1
         MULT &1
         FOO &1, &1
         ENDMACRO
ESPACO:  SPACE
INICIO:  LOAD _DOIS! ; Tudo bem?
         BAR _DOIS!
         ADD CI_NCO
         ; Comentário feliz :)
         COPY _DOIS! CI_NCO
         STORE _DOIS!
         FOO _DOIS!, CI_NCO ; Com você? 
         STOP
_DOIS!:    
SPACE 2
CI_NCO:   
CONST 5