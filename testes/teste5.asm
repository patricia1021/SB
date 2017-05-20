; Teste 5 

;erros: declaraço ausente de L1, declaraço repetida de L2, diretiva inválida EQUAL; dois rótulos (var e val) na mesma linha;
; mais de um argumento em uma instrução; não tem stop


L2: EQU 0
L2: EQU 0
L3: EQUAL 1
TAM: equ 2

SECTION TEXT
IF L1
LOAD N
IF L2
LOAD N+2
var2: var1: load N ; 2 rts
ADD val,n+1; mais de um arg
Add n+8
JMP Var
;falta stop
SECTION DATA
N: SPACE 3
Var1: space
var2: space
val: const tam
