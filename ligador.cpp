/*Fazer um cóodigo (ligador.c) que receba por linha de comando o nome de até 4
arquivos (a extensão \.o" deve estar presente em todos menos o último arquivo, o
último deve ser extensão \.e"). O programa deve fazer a ligação entre os dois ou três
primeiros módulos gerando o arquivo ligado de saída. O arquivo de saida deve ser em
formato TEXTO contendo OPCODES e operandos sem quebra de linha, nem endereço
indicado, sepradados por espaço, SEM O INDICADOR DE SEÇãO \CODE", sem a
indicação de absoluto e relativo, sem tabelas (ou seja, somente uma linha de números).
O ligador deve verificar durante o processo de ligação, se ficaram símbolos não
definidos.*/ 

#include <stdio.h>

#ifndef MONTADOR_CPP
#define MONTADOR_CPP

void ligador (int argc, char*argv[])
{
  FILE *open, *write;
  int i;
  char c;
  
  if (argc > 4)
  {
    printf ("Quantidade de arquivos maior que o necessário."\n);
    exit(1);
  }
  /*if (argv[3]...)
  printf ("A extensão do último arquivo deve ser \.e"\n);
  //para saber se a extenso do último arquivo é .e
*/
  //abre o arquivo aonde vai ser escrito
  write = fopen ("arquivo_ligado.c", "w");
  fclose(write);
  //laço para abrir os argvs e escrever no
  for(i = 0; i < argc; i++)
	{
    open = fopen(argv[i], "r");
	  if(open == NULL)
		  printf("Arquivo vazio\n",);
    
    write = fopen("arquivo_ligado.c", "r+");
	  while((c = getc(open)) != EOF)
		  putchar(c);

	  fclose(open);
  }
  return(0);
}
