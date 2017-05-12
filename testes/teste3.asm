; Teste 3

;SECTION TEXT
INPUt #
LoaD #
div zero ; divisão por 0
store r
input var
load var
soma: add var
store r
jmpz som ; pulo para rótulo inválido, deveria ser soma
stop
SECTION DATAS ; seção inválida
#: SPACE
zero: const 0
r: space
var: SpACe
var: const 2
