#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "constantes.hpp"
#include "helper.cpp"

#ifndef PREPROCESSA_CPP
#define PREPROCESSA_CPP

typedef map<string,int> EquMap;

int preprocessa_arquivo(fstream &fonte, fstream &saida_pre, char *argv[]);

int check_EQU(string line, map<string, int> &equ_map);

int check_IF(string line, fstream &fonte, fstream &saida, map<string, int> &equ_map);

void proceed(string &line, fstream &saida, EquMap &em);

string filtra_comentario(string line);

int get_operand_p(string s, vector<string> &v, EquMap &em);

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
					proceed(line, saida_pre, equ_map); // somente transcreve a proxima linha
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
					return 1;
					saida << nextline << endl;
				}
				return 1;
			}
			else
				cout << RED << "Erro de preprocessamento, " <<"Simbolo "<< tokens[1] << " nao foi definido previamente, linha: [[   " <<line << "   ]]"<<endl <<RESET;

		}
		else
			cout << RED << SYNTAX_ERROR << "numero incorrento de argumemntos para a diretiva IF" << endl;
		return 1;
	}
	return 0;
}

/***************************************************************************
 * transcreve a linha para o arquivo de saida
 * *************************************************************************/
void proceed(string &line, fstream &saida, EquMap &em){
	vector<string> tokens;
	vector<string> copy_args;
	vector<string> operands;

	const char *tok_del = " ";
	const char *copy_del = ",";
	const char *op_sep = "+";

	int se_label = 0;
	string token;
	string result = "";
	split(line, tok_del, tokens);
	if(eh_label(tokens[0])){
		se_label = 1;
		token = tokens[0].substr(0, tokens[0].length()-1);
		auto it = em.find(token);
		if(it!=em.end()){
			result+=to_string(it->second)+": ";
		}
		else{
			result+=tokens[0]+" ";
		}
	}
	for(int i = se_label; unsigned(i) < tokens.size();i++){
		split(tokens[i], copy_del, copy_args);
		if(copy_args.size() > 1){
			vector<string> comma;
			for(auto el:copy_args){
				string aux;
				vector<string> ops;
				get_operand_p(el, ops, em);
				string aux2 = join(ops, op_sep);
				aux = (ops.size() > 1 ? aux2.substr(0, aux2.length()-1) : ops[0]);
				comma.push_back(aux);
			}
			result+=join(comma,copy_del);
			result = result.substr(0,result.length()-1) + " ";
			continue;
		}
		split(tokens[i], op_sep, operands);
		if(operands.size() > 1){
			vector<string> ops;
			get_operand_p(tokens[i], ops, em);
			string aux2 = join(ops, op_sep);
			result+= (ops.size() > 1 ? aux2.substr(0, aux2.length()-1)+" ": ops[0] +" ");
			continue;
		}
		auto it = em.find(tokens[i]);
		if(it!=em.end()){
			result+=to_string(it->second)+" ";
		}
		else{
			result+=tokens[i]+" ";
		}
	}
	saida << reduce(result) << endl;
}

int get_operand_p(string s, vector<string> &v, EquMap &em){
	vector<string> vec;
	const char *del = "+";
	split(s, del, vec);
	for(auto el: vec){
		auto it = em.find(el);
		if(it != em.end()){
			v.push_back(to_string(it->second));
		}else{
			v.push_back(el);
		}
	}
	return 1;
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
