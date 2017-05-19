#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "constantes.hpp"
#include "helper.cpp"

#ifndef PREPROCESSA_CPP
#define PREPROCESSA_CPP

int preprocessa_arquivo(fstream &fonte, fstream &saida_pre, char *argv[]);

int check_EQU(string line, map<string, int> &equ_map);

int check_IF(string line, fstream &fonte, fstream &saida, map<string, int> &equ_map);

void proceed(string &line, fstream &saida);

string filtra_comentario(string line);

/*********************************************************************************
 * funcao com a logica principal de execucao das diretivas de precompilacao 
 * EQU e IF
 * *******************************************************************************/
int preprocessa_arquivo(fstream &fonte, fstream &saida_pre, char *argv[]){
	saida_pre.open(string(argv[2])+".pre", fstream::in | fstream::out | fstream::trunc);
	map<string, int> equ_map;
	if(saida_pre.is_open()){
		string line;
		cout << BLU << "Preprocessamento Iniciado" << endl << RESET;
		while(!fonte.eof()){
			getline(fonte, line);
			to_uppercase(line);
			line = (reduce(line)); // retira tabs e espacoes em branco desnecessarios
			line = filtra_comentario(line); // retira comentarios
			if(line.empty()) continue; // caso seja linha em branco, naa faz nada
			if(!check_EQU(line, equ_map)){ // identifica e trata a diretiva EQU
				if(!check_IF(line, fonte, saida_pre, equ_map)){ // identifica e trata a diretiva IF
					proceed(line, saida_pre); // somente transcreve a proxima linha
				}
			}
		}
		cout << BLU << "Preprocessamento finalizado" << endl << RESET;
		return 1;
	} else {
		cout << "Nao foi possivel criar arquivo preprocessado, favor checar suas permissoes de sistema\n";
		return 0;
	}

}
/*****************************************************************************
 * verifica a existencia do EQU na linha atual e caso exista, adiciona o valor
 * ao map do EQU
 * ***************************************************************************/
int check_EQU(string line, map<string, int> &equ_map){
	vector<string> tokens;
	string token;
	const char *delimiter = " ";
	split(line, delimiter, tokens); // separa todas as palavras da linha
	token = tokens.front();
	int num_args;
	if(eh_label(token)){ // se não for um label entao não pode ser a diretiva EQU
		num_args = tokens.size() - 2;
		if(tokens.size() > 1) {
			if(tokens[1] == EQU){
				if(num_args == 1){ // EQU espera 1 argumento
					equ_map[token.substr(0,token.length()-1)] = stoi(tokens[2]); // insere no map
				}
				else
					cout << SYNTAX_ERROR << "numero incorreto de argumentos para a diretiva EQU" <<endl;
				return 1;
			}
		}
	}
	/* cout << token <<endl; */
	return 0;
}
/*****************************************************************************
 * funcao que verifica a existencia do IF na linha atual e executa o procedimento
 * adequado caso true
 * **************************************************************************/
int check_IF(string line, fstream &fonte, fstream &saida, map<string, int> &equ_map){
	vector<string> tokens;
	string token;
	const char *delimiter = " ";
	split(line, delimiter, tokens); // separa todas as palavras da linha
	token = tokens.front();
	string nextline;
	if(token == IF){
		if(tokens.size() == 2){
			if(equ_map.count(tokens[1]) != 0){
				getline(fonte, nextline);
				to_uppercase(nextline);
				nextline = (reduce(nextline)); // retira tabs e espacoes em branco desnecessarios
				nextline = filtra_comentario(nextline); // retira comentarios
				if(equ_map[tokens[1]] > 0){
					saida << nextline << endl;
				}
				return 1;
			}
			else
				cout << RED << SEMANTIC_ERROR <<"Simbolo "<< tokens[1] << " nao foi definido previamente"<<endl <<RESET;
		}
		else
			cout << RED << SYNTAX_ERROR << "numero incorrento de argumemntos para a diretiva IF" << endl;
	}
	return 0;
}

/***************************************************************************
 * transcreve a linha para o arquivo de saida
 * *************************************************************************/
void proceed(string &line, fstream &saida){
	saida << line << endl;
}

/******************************************************************************
 * funcao que retira os comentarios da linha
 * ***************************************************************************/
string filtra_comentario(string line){
	size_t pos = line.find(";");
	if(pos != string::npos){
		line = line.substr(0,pos);
	}
	return line;
}

#endif
