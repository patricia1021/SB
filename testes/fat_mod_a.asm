; Teste 4
; Falta de begin e end nos módulos; instrução errada; pulo para rótulo inválido ; instrução no local errado

SECTION TEXT
mod_a: ; sem begin
area: extern
mod_b: extern
d: extern
; var inventada que não tem no outro mod
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

