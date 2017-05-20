all: montador

montador: compile_montador
	./prog -o "arquivos_teste_moodle/bin.asm" bin

ligador: compile_ligador
	./ligador "executaveis/fat_mod_A.o" "executaveis/fat_mod_B.o" fat.e

compile_montador:
	g++ main.cpp -o prog -std=c++14

compile_ligador:
	g++ ligador.cpp -o ligador -std=c++14


