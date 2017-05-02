#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "constantes.hpp"
#include "helper.cpp"

#ifndef MONTADOR_CPP
#define MONTADOR_CPP

int monta_arquivo(fstream &fonte, string filename);

int primeira_passagem(fstream &fonte, map<string, int> &simbol_table, map<string, int> &instruction_table);

void inicializa_tabela_instrucao(map<string, int> &t);

int check_validade_tokens(vector<string> &tokens);

void log_error(int tipo_erro, int subtipo_erro, int linha);

int get_instruction(map<string, int> &t, string s);

int existe_label(map<string, int> &simbol_table, string token);

int monta_arquivo(fstream &fonte, string filename){
	map<string, int> simbol_table;
	map<string, int> instruction_table;
	inicializa_tabela_instrucao(instruction_table);
	primeira_passagem(fonte, simbol_table, instruction_table);
}



int primeira_passagem(fstream &fonte, map<string, int> &simbol_table, map<string, int> &instruction_table){
	vector<string> tokens; // armazena os elementos separados da linha
	int count_pos = 0;
	int count_line = 1;

	string line;
	string token;

	const char *delimiter = " ";
	cout << BLU <<"montagem iniciada" <<endl << RESET;

	if(fonte.is_open()){
		cout << "ENTREI NO LOOP" << endl;
		fonte.clear();
		fonte.seekg(0, ios::beg);
		while(!fonte.eof()){
			getline(fonte, line);
			cout << YEL << line << endl << RESET;
			to_uppercase(line); // passa para maiusculo
			split(line, delimiter, tokens); // separa os elementos da linha em tokens
			token = tokens[0].substr(0, tokens[0].length()); // primeiro token da linha

			if(!check_validade_tokens(tokens)){
				log_error(1, 2, count_line);
				count_line+=1;
				continue;
			}
			if(eh_label(tokens[0])){
				if(existe_label(simbol_table, token)){
					log_error(1, 1, count_line);
					count_line+=1;
					continue;
				}
				else {
					/* adiciona_label(simbol_table, count_pos); */
				}
			}
			else {
				int i;
				cout << "TOKEN: " << token <<endl;
				if((i = get_instruction(instruction_table, token)) != 0){
					cout << "instrucao encontrada: " << instruction_table[token] << endl;
				}
				else {
					cout << " NAO EH INSTRUCAO "<< endl;
				}
			}
		}
	}
}


int check_validade_tokens(vector<string> &tokens){
	return 1;
}

void inicializa_tabela_instrucao(map<string, int> &t){
	t["ADD"] = ADD;
	t["SUB"] = SUB;
	t["MULT"] = MULT;
	t["DIV"] = DIV;
	t["JUMP"] = JUMP;
	t["JUMPN"] = JUMPN;
	t["JUMPP"] = JUMPP;
	t["JUMPZ"] = JUMPZ;
	t["COPY"] = COPY;
	t["LOAD"] = LOAD;
	t["STORE"] = STORE;
	t["INPUT"] = INPUT;
	t["OUTPUT"] = OUTPUT;
	t["STOP"] = STOP;
}

int get_instruction(map<string, int> &t, string s){
	auto it = t.find(s);
	if( it != t.end()){
		return it->second;
	} else return 0;
}

int existe_label(map<string, int> &simbol_table, string token){
	auto it = simbol_table.find(token);
	if( it != simbol_table.end()){
		return 1;
	} else return 0;
}

void log_error(int tipo_erro, int subtipo_erro, int linha){
	cout << "============================================== LOGEI um ERRO ================================================" << endl;
	return;
}

#endif /* ifndef MONTADOR_CPP*/

