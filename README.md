# SB
Trabalho da matéria Software Básico no 1/2017 

## GRUPO:
Igor Sodré - 130114553
Patricia - 130015989

Sistema operacional utilizado: Linux

OBS: os erros são mostrados no arquivo preprocessado gerado pelo programa;

A execucao do programa segue o padrao utilizado na especificao do trabalho. Ele recebe como entrada um arquivo ".asm" ou um arquivo ".pre" e gera um arquivo ".o"
<br />
==============================             MONTADOR                 ====================================<br />
Compilação do montador:
Utilize o comando abaixo para compilar o montador:
	$ g++ main.cpp -o NOME -std=c++14
<br />
<br />
Para execução do programa, utilize:
	$ ./NOME -o ARQUIVO_ENTRADA.asm ARQUIVO_SAIDA
<br />
<br />
Para preprocessamento, utilize:
	$ ./NOME -p ARQUIVO ENTRADA.asm ARQUIVO_SAIDA
<br />
==============================            LIGADOR                   ===================================<br />
O ligador recebe como argumentos 2 ou 3 arquivos para serem ligados e 1 nome do arquivo de saída e gera um 1 arquivo executável.
<br />
<br />
Compilação do ligador:
Utilize o comando abaixo para compilar o ligador:
	$ g++ ligador.cpp -o NOME -std=c++14 -Wall
<br />
<br />
Para executar o ligador, utilize:
	$ ./NOME ARQUIVO_1.o ARQUIVO_2.o ARQUIVO_SAIDA.e
