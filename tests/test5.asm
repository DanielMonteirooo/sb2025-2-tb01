LOAD    LIMIT          
       LOAD    ZERO           
       STORE   CUR            
       LOAD    ONE            
       STORE   PREV           
       STORE   TEMP           
       
FIB:   LOAD    CUR            
       ADD     PREV           
       STORE   TEMP           
       LOAD    PREV           
       STORE   CUR            
       LOAD    TEMP           
       STORE   PREV           
       SUB     LIMIT          
       JMPP    END            
       JMP     FIB            
       
END:   LOAD    CUR
       SUB     VALUE
       JMPZ    OK
ERRO:  JMP     ERRO  ; Trava aqui se o resultado estiver errado
OK:    STOP

PREV:  SPACE                  
CUR:   SPACE                  
TEMP:  SPACE                  
LIMIT: CONST  10              
VALUE: CONST  8               
ZERO:  CONST  0               
ONE:   CONST  1