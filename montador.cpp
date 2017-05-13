#include <iostream>
#include <fstream>
#include <memory>
#include <map>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include "constantes.hpp"
#include "helper.cpp"

#ifndef MONTADOR_CPP
#define MONTADOR_CPP

class Ts{
	public:
		Ts(int val, int outside = 0){
			this->val = val;
			this->outside = outside;
		}
		int val;
		int outside;
};

typedef map<string, shared_ptr<Ts>> SimbolTable;

class Config {
	public:
		SimbolTable simbol_table;
		map<int, int> memory;
		map<string, int> instruction_table;
		map<string, int> definition_table;
		map<string, int> use_table;
		int section_text_count = 0;
		int section_data_count = 0;
};

int monta_arquivo(fstream &fonte, string filename);

int primeira_passagem(fstream &fonte, Config &c);

int segunda_passagem(fstream &fonte, Config &c);

void inicializa_tabela_instrucao(map<string, int> &t);

void inicializa_tabela_tamanhos_instrucao(map<int,int> &t);

int check_validade_tokens(vector<string> &tokens);

int check_section_text(string &s, int &counter);

int check_section_data(string &s, int &counter);

void log_error(int tipo_erro, int subtipo_erro, int linha);

int get_instruction(map<string, int> &t, string s);

int existe_label(SimbolTable &simbol_table, string token);

void adiciona_label(SimbolTable &simbol_table, string simbol, int posicao);

int eh_diretiva(string &s);

int exec_diretiva(string &diretiva, vector<string> &argumentos, Config &c, int count_pos, int count_line);

int set_extern(string label, SimbolTable &simbol_table);

int set_public(string label, Config &c);

int set_definitions(Config &c);

int monta_arquivo(fstream &fonte, string filename){
	Config c;

	inicializa_tabela_instrucao(c.instruction_table);

	cout << BLU <<"==================================     MONTAGEM INICIADA    ===========================================" << endl << RESET;
	primeira_passagem(fonte, c);
}



int primeira_passagem(fstream &fonte, Config &c){
	vector<string> tokens; // armazena os elementos separados da linha
	map<int, int> inst_size_table;
	inicializa_tabela_tamanhos_instrucao(inst_size_table);

	int count_pos = 0;
	int count_line = 1;
	string line;
	string token;
	string operacao;

	const char *delimiter = " ";

	if(fonte.is_open()){
		fonte.clear();
		fonte.seekg(0, ios::beg);
		while(!fonte.eof()){
			getline(fonte, line);

			if(line.empty() // linha vazia nao faz nada
					|| check_section_text(line, c.section_text_count) // comeco da secaon texto só pula
					|| check_section_data(line, c.section_data_count)){ // comecao da secao dados
				count_line++;
				continue;
			}

			//DEBUGG
			cout << "line cout: " << count_line << " - pos_count: "<<count_pos << endl;
			cout << YEL << line << endl << RESET;

			to_uppercase(line); // passa para maiusculo
			split(line, delimiter, tokens); // separa os elementos da linha em tokens
			token = tokens[0].substr(0, tokens[0].length() -1); // primeiro token da linha
			operacao = tokens[0];

			if(!check_validade_tokens(tokens)){
				log_error(ERRO_LEXICO, WRONG_TOKEN_FORMAT, count_line); // se tokens invalidas throw lexical error
				count_line++;
				continue;
			}
			if(eh_label(tokens[0])){
				operacao = tokens[1];
				if(existe_label(c.simbol_table, token)){
					// DEBUG
					cout << "JA EXISTE LABEL" << endl;
					log_error(ERRO_SEMANTICO, TOKEN_ALREADY_EXISTS, count_line); // se o label ja existe na TS throw semantic error
					count_line++;
					continue;
				}
				else {
					adiciona_label(c.simbol_table, token, count_pos); // caso nao exista label, adiciona na TS com a posicao atual
				}
			}

			int i;
			if((i = get_instruction(c.instruction_table, operacao)) != 0){
				//DEBUG
				cout << "linha " << count_line <<" instrucao encontrada: " << c.instruction_table[operacao] << endl;
				c.memory[count_pos] = 0;
				count_pos += inst_size_table[i];
			}
			else if(eh_diretiva(operacao)) {
				int dir_count = exec_diretiva(operacao, tokens, c, count_pos, count_line);
				if(dir_count < 0){
					//DEBUG
					cout << "ERRO DE DIRETIVA" << endl;
					log_error(1, 1, count_line); // erro de diretiva, tratado nan funcao exec_diretiva
				}
				else
					count_pos+= dir_count;
			}
			else {
				//DEBUG
				cout << "INSTRUCAO NEM DIRETIVA ENCONTRADA" << endl;
				log_error(ERRO_SINTATICO, INSTRUCTION_NOT_FOUND, count_line); // instrucao / diretiva nao encontrada
			}
			count_line++;
		}

		set_definitions(c);
		// DEBUG
		cout << MAG << " ======================== RESULTADO PRIMEIRA PASSAGEM ========================================" << endl;
		cout <<MAG<< "TABELA DE SIMBOLOS:" << RESET << endl;
		for(auto &it:c.simbol_table){
			cout << it.first << " - " << it.second->val << " - extern? " << it.second->outside << endl;
		}
		cout <<MAG<< "MEMORIA" << RESET << endl;
		for(auto &it:c.memory){
			cout << it.first << " - " << it.second << endl;
		}
		cout << MAG << "TABELA DEFINICOES" <<RESET <<endl;
		for(auto &it:c.definition_table){
			cout << it.first << " - " << it.second << endl;
		}
		cout << MAG << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << RESET << endl;
	}
}


int segunda_passagem(fstream &fonte, Config &c){

}


/********************************************************************************
 * verifica validade das tokens da linha
 * ******************************************************************************/
int check_validade_tokens(vector<string> &tokens){
	return 1;
}


/********************************************************************************
 * popula tabela de dfinicoes com os valores corretos
 * ****************************************************************************/
int set_definitions(Config &c){
	for(auto &it: c.definition_table){
		auto aux = c.simbol_table.find(it.first);
		if(aux != c.simbol_table.end()){
			c.definition_table[it.first] = aux->second->val;
		}
	}
}

/********************************************************************************
 * verifca se linha a é o comeco da sessao de texto
 * ******************************************************************************/
int check_section_text(string &s, int &counter){
	if(s.find(SECTION_TEXT)!= string::npos){
		counter++;
		return 1;
	}
	else return 0;
}

/********************************************************************************
 * verifica se a linha eh o comeco da sesao de dados
 * ******************************************************************************/
int check_section_data(string &s, int &counter){
	if(s.find(SECTION_DATA)!= string::npos){
		counter++;
		return 1;
	}
	else return 0;
}
/********************************************************************************
 * nome da funcao auto-explicativo
 * ******************************************************************************/
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

/********************************************************************************
 * nome da funcao auto-explicativo
 * ******************************************************************************/
void inicializa_tabela_tamanhos_instrucao(map<int,int> &t){
	t[ADD] = 2;
	t[SUB] = 2;
	t[MULT] = 2;
	t[DIV] = 2;
	t[JUMP] = 2;
	t[JUMPN] = 2;
	t[JUMPP] = 2;
	t[JUMPZ] = 2;
	t[COPY] = 3;
	t[LOAD] = 2;
	t[STORE] = 2;
	t[INPUT] = 2;
	t[OUTPUT] = 2;
	t[STOP] = 1;
}

/********************************************************************************
 * retorna o opcode da instrucao caso ela exista ou retorna 0 caso nao exista
 * ******************************************************************************/
int get_instruction(map<string, int> &t, string s){
	auto it = t.find(s);
	if( it != t.end()){
		return it->second;
	} else return 0;
}

/********************************************************************************
 * adiciona token na tabela de simbolos
 * ******************************************************************************/
void adiciona_label(SimbolTable &simbol_table, string simbol, int posicao){
	simbol_table[simbol] = shared_ptr<Ts>(new Ts(posicao));
}

/********************************************************************************
 * verifica a existencia do token na tabela de diretibas
 * ******************************************************************************/
int eh_diretiva(string &s){
	vector<string> diretivas { SPACE, CONST, EXTERN, PUBLIC};
	return in_array(s, diretivas);
}

/********************************************************************************
 * verifica a corretuda das diretivas e retorna o numero de possicoes que a diretiva
 * ocupa
 * ******************************************************************************/
int exec_diretiva(string &diretiva, vector<string> &argumentos, Config &c, int count_pos, int count_line){
	int result;
	int arg_size = argumentos.size();
	if(diretiva == SPACE){
		if(arg_size > 3){
			log_error(ERRO_SINTATICO, WRONG_ARG_NUM, count_line);
			return WRONG_ARG_NUM; // SPACE ou possui 1 ou nenum argumentos
		}
		else if(arg_size == 3){
			// DEBUG
			cout << CYN << "TRATANDO SPACE, argumento: " << argumentos[2] << RESET << endl;
			if(is_number(argumentos[2]))
				return stoi(argumentos[2]);
			else {
				log_error(ERRO_SEMANTICO, WRONG_ARG_TYPE, count_line);
				return WRONG_ARG_NUM; // SPACE ou possui 1 ou nenum argumentos
			}
			/* memory[count_pos] = 0; */
			return 2;
		}
		else{
			// DEBUG
			cout << CYN << "TRATANDO SPACE, sem argumentos." << RESET << endl;
			c.memory[count_pos] = 0;
			return 1; // space sem argumentis ocupa 1 espaco em memoria
		}
	}
	else if(diretiva == CONST){
		if(arg_size != 3){
			return WRONG_ARG_NUM; // CONST sempre possui 1 argumento
		} else if(is_hex_string(argumentos[2])){
			c.memory[count_pos] = strtol(argumentos[2].c_str(), NULL, 16);
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else if(is_number(argumentos[2])){
			c.memory[count_pos] = stoi(argumentos[2]);
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else return WRONG_TOKEN_FORMAT;
	}
	else if(diretiva == BEGIN){
		return 0;
	}
	else if(diretiva == EXTERN){
		if(arg_size != 2){
			return WRONG_ARG_NUM; // EXTERN POSSUI UM LABEL E A DIRTETIVA
		}
		set_extern(argumentos[0], c.simbol_table);
		return 0;
	}
	else if(diretiva == PUBLIC) {
		if(arg_size != 2){
			return WRONG_ARG_NUM; // PUBLIC POSSUI 1 ARGUMENTO
		}
		set_public(argumentos[1], c);
		return 0;
	}
	return 0;
}

/********************************************************************************
 * funcao que verifica se o primeiro token a linha é um label
 * ******************************************************************************/
int existe_label(SimbolTable &simbol_table, string token){
	auto it = simbol_table.find(token);
	if( it != simbol_table.end()){
		return 1;
	} else return 0;
}

/********************************************************************************
 * marca label na tablea de simbolos com externo
 * ******************************************************************************/
int set_extern(string label, SimbolTable &simbol_table){
	auto it = simbol_table.find(label.substr(0, label.length() -1));
	// DEBUG
	cout << "ACHEI EXTERN: "<< label << endl;
	if(it != simbol_table.end()){
		it->second->outside = 1;
		it->second->val = 0;
	}
}
/********************************************************************************
 * se o label for public, insere na tabela de definicaoes
 * ******************************************************************************/
int set_public(string label, Config &c){
	// DEBUG
	cout << "ACHEI PUBLIC: "<< label << endl;
	c.definition_table[label] = 0;
}

/********************************************************************************
 * loga os error encotrados no codigo
 * ******************************************************************************/
void log_error(int tipo_erro, int subtipo_erro, int linha){
	cout << "============================================== LOGEI um ERRO ================================================   linha: "<<linha << endl;
	return;
}

#endif /* ifndef MONTADOR_CPP*/

