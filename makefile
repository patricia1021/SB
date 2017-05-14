all: compile run

compile:
	g++ main.cpp -o prog -std=c++14

run:
	./prog -o "testes/teste2.asm" coisa

