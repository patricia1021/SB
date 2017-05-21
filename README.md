# SB
Trabalho da matéria Software Básico no 1/2017 

GRUPO:
Igor Sodre - 130114553
Patricia

Sistema operacional utilizado: linux

OBS: os erros sao mostrados no arquivo preprocessado gerado pelo programa;

a execucao do programa segue o podrao utilizado na especificao do trabalho. ele revebe como entrada um arquivo ".asm" ou um arquivo ".pre" e gera um arquivo ".o"

==============================             MONTADOR                 ====================================
Compilação do montador:
Utilize o comando abaixo para compilar o montador:
	$ g++ main.cpp -o NOME -std=c++14

para execucao do programa use:
	$ ./NOME -o ARQUIVO_ENTRADA.asm ARQUIVO_SAIDA

e para preprocessamento utilize:
	$ ./NOME -p ARQUIVO ENTRADA.asm ARQUIVO_SAIDA


==============================            LIGADOR                   ===================================
o ligador recebe como argumentos 2 ou 3 arquivos para serem ligados e 1 nome do arquivo de saida e gera um 1 arquivo executavel

Compilação do ligador:
utilize o comando abaixo para compilar o ligador:
	$ g++ ligador.cpp -o NOME -std=c++14 -Wall


executaor o ligaor:
	$ ./NOME ARQUIVO_1.o ARQUIVO_2.o ARQUIVO_SAIDA.e
