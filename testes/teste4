; Teste 4
; Falta de begin e end nos módulos; instrução errada; pulo para rótulo inválido ; instrução no local errado

SECTION TEXT
mod_a: ; sem begin
area: extern
mod_b: extern
public b
public h
input b
input h
jmp modulo_b ;pulo para rótulo inválido
output area

SECTION DATA
b: space
h: space
stop ; instrução no local errado
end ; instrução no local errado

SECTION TEXT
mod_b: begin
b: extern
h: extern
public soma
public mod_b
load b
mult h ;instrução errada
div dois
store area

SECTION DATA
dois: const 2
area: space
; sem end
