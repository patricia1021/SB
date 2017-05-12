/*Fazer um cóodigo (ligador.c) que receba por linha de comando o nome de até 4
arquivos (a extensão \.o" deve estar presente em todos menos o último arquivo, o
último deve ser extensão \.e"). O programa deve fazer a ligação entre os dois ou três
primeiros móodulos gerando o arquivo ligado de saída. O arquivo de saida deve ser em
formato TEXTO contendo OPCODES e operandos sem quebra de linha, nem endereço
indicado, sepradados por espaço, SEM O INDICADOR DE SEÇãO \CODE", sem a
indicação de absoluto e relativo, sem tabelas (ou seja, somente uma linha de números).
O ligador deve verificar durante o processo de ligação, se ficaram símbolos não
definidos.*/ 

#ifndef MONTADOR_CPP
#define MONTADOR_CPP

void ligador (int argc, char*argv[])
{
FILE *arq;
  
  if (argc >= 4)
    printf ("Quantidade de arquivos maior que o necessário."\n);

   arq = fopen(,);
}
