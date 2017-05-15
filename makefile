all: compile run

compile:
	g++ main.cpp -o prog -std=c++14

run:
	./prog -o "arquivos_teste_moodle/triangulo.asm" coisa

