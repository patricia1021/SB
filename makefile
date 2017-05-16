all: compile_montador run

compile_montador:
	g++ main.cpp -o prog -std=c++14

run:
	./prog -o "arquivos_teste_moodle/fat_mod_B.asm" coisa

