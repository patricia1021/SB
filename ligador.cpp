/*Fazer um cóodigo (ligador.c) que receba por linha de comando o nome de até 4
  arquivos (a extensão \.o" deve estar presente em todos menos o último arquivo, o
  último deve ser extensão \.e"). O programa deve fazer a ligação entre os dois ou três
  primeiros módulos gerando o arquivo ligado de saída. O arquivo de saida deve ser em
  formato TEXTO contendo OPCODES e operandos sem quebra de linha, nem endereço
  indicado, sepradados por espaço, SEM O INDICADOR DE SEÇãO \CODE", sem a
  indicação de absoluto e relativo, sem tabelas (ou seja, somente uma linha de números).
  O ligador deve verificar durante o processo de ligação, se ficaram símbolos não
  definidos.*/ 


#ifndef LIGADOR_CPP
#define LIGADOR_CPP
#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <ctype.h>
#include <string>
#include "constantes.hpp"
#include "helper.cpp"

using namespace std;


class Modules{
	public:
		fstream files;
		int correction_factor = 0;
		map<string, int> definition_table;
		map<string, vector<int>> use_table;
		vector<int> realocation_table;

};

class ConfLig {
	public:
		vector<Modules> modules;
		map<string, int> global_df;
};

int verifica_argumentos_ligador (int argc, char *argv[]);

int verifica_extensao_ponto_o(string &s);

int verifica_extensao_ponto_e(string &s);

int check_and_open_files(ConfLig &c, int argc, char *argv[]);

int liga_programa(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	// chama o ligador
	liga_programa(argc, argv);
	return 0;
}

int liga_programa (int argc, char*argv[])
{
	ConfLig c;
	if(verifica_argumentos_ligador(argc, argv) && check_and_open_files(c, argc, argv){

	}
	return 0;
}

int check_and_open_files(ConfLig &c, int argc, char *argv[]){
	cout << "ENTREI OPEN FILES" << endl;
	return 1;
}
/*********************************************************************************
 * Funcao que verifica a quantidade e a validade dos arguetos passados ao programa
 ********************************************************************************/
int verifica_argumentos_ligador (int argc, char *argv[]){
	if(argc < 3 || argc > 5)
	{
		cout << RED << "Quantidade de arquivos maior que o necessário.\n\n Execute o programa novamente com o numero correto de argumentos\n" << RESET;
		return 0;
	}
	// checar se os primeiros arquivos são .o e o último é .e para todos os casos
	else {
		for(int i = 1; i  < (argc - 1); i++){
			string s = string(argv[i]);
			if(!verifica_extensao_ponto_o(s)){
				cout << RED <<"extencao do argumento "<<(i)<<" incorreta" <<RESET <<endl;
				return 0;
			}
		}
		string st = string(argv[argc-1]);
		if(!verifica_extensao_ponto_e(st)){
			cout << RED <<"extencao do arquivo de saida dever ser \".e\" " <<RESET <<endl;
			return 0;
		}
	}
	cout << "OK" << endl;
	return 1;
}

int verifica_extensao_ponto_o(string &s){
	return (s.substr(s.length()-2, s.length()).find(".o") != string::npos);
}

int verifica_extensao_ponto_e(string &s){
	return (s.substr(s.length()-2, s.length()).find(".e") != string::npos);
}

#endif
