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

// estrutura que eh colocada na tablea de simbolos
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

// class com variaveis de configuracao da montagem
class Config {
	public:
		// estruturas de dados
		SimbolTable simbol_table;
		map<int, int> memory;
		map<string, int> instruction_table;
		map<string, int> definition_table;
		map<string, int> use_table;
		map<int, int> inst_size_table;

		// flags
		int eh_modulo = 0;
		int num_ends = 0;
		int num_errors = 0;
		int section_data_count = 0;
		int section_text_count = 0;

		// curent line being read from the code
		string line;

		// contados de linha e de endereco
		int count_pos = 0;
		int count_line = 1;

		int err_type;
		int err_subtype;
};

class Operand {
	public:
		string label;
		int offset = 0;
};

int monta_arquivo(fstream &fonte, string filename);

int primeira_passagem(fstream &fonte, Config &c);

int segunda_passagem(fstream &fonte, Config &c);

void inicializa_tabela_instrucao(map<string, int> &t);

void inicializa_tabela_tamanhos_instrucao(map<int,int> &t);

int check_validade_tokens(vector<string> &tokens);

int check_section_text(string &s, int &counter);

int check_section_data(string &s, int &counter);

int check_sections_order(Config &c);

void log_error(Config &c);

int check_error(Config &c);

int get_instruction(map<string, int> &t, string s);

int existe_label(SimbolTable &simbol_table, string token);

void adiciona_label(SimbolTable &simbol_table, string simbol, int posicao);

int eh_diretiva(string &s);

int exec_diretiva(string &diretiva, vector<string> &argumentos, Config &c, int count_pos, int count_line);

int set_extern(string label, SimbolTable &simbol_table);

int set_public(string label, Config &c);

int set_definitions(Config &c);

int check_operandos(Config &c, vector<string> &tokens, int line_has_label);

int validate_copy(string str, Config &c);

int get_operando(string str, Operand &op);

int validate_token(string s, int option);

int monta_arquivo(fstream &fonte, string filename){
	Config c;

	inicializa_tabela_instrucao(c.instruction_table);
	inicializa_tabela_tamanhos_instrucao(c.inst_size_table);

	cout << BLU <<"==================================     MONTAGEM INICIADA    ===========================================" << endl << RESET;
	primeira_passagem(fonte, c);
	// so continua caso a primeira passagem esta livre de errors
	if(!check_error(c)) {
		cout << RED << "Arquivo nao montado por contem erros, favor corrigir os erros e tentar novamente!!" RESET<<endl;
		return 0;
	}
	/* segunda_passagem(fonte, c); */
}
/**************************************************************************
 * funcao principal da primeira passagem do algoritmo de montagem
 * ************************************************************************/
int primeira_passagem(fstream &fonte, Config &c){
	vector<string> tokens; // armazena os elementos separados da linha
	c.count_pos = 0;
	c.count_line = 1;
	string line;
	string operacao;
	string token;

	const char *delimiter = " ";

	if(fonte.is_open()){
		fonte.clear();
		fonte.seekg(0, ios::beg);
		while(!fonte.eof()){
			getline(fonte, line);
			to_uppercase(line); // passa para maiusculo

			if(line.empty() // linha vazia nao faz nada
					|| check_section_text(line, c.section_text_count) // comeco da secaon texto só pula
					|| check_section_data(line, c.section_data_count)){ // comecao da secao dados
				c.count_line++;
				continue;
			}

			//DEBUGG
			cout << "line cout: " << c.count_line << " - pos_count: "<<c.count_pos << endl;
			cout << YEL << line << endl << RESET;

			split(line, delimiter, tokens); // separa os elementos da linha em tokens
			token = tokens[0].substr(0, tokens[0].length() -1); // primeiro token da linha
			operacao = tokens[0];

			if(!check_validade_tokens(tokens)){
				c.err_type = ERRO_LEXICO;
				c.err_subtype = WRONG_TOKEN_FORMAT;
				log_error(c); // se tokens invalidas throw lexical error
				c.count_line++;
				continue;
			}
			if(eh_label(tokens[0])){
				operacao = tokens[1];
				if(existe_label(c.simbol_table, token)){
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = TOKEN_ALREADY_EXISTS;
					log_error(c); // se o label ja existe na TS throw semantic error
					c.count_line++;
					continue;
				}
				else {
					adiciona_label(c.simbol_table, token, c.count_pos); // caso nao exista label, adiciona na TS com a posicao atual
				}
			}

			int i;
			if((i = get_instruction(c.instruction_table, operacao)) != 0){
				//////////////////////////////      DEBUG            //////////////////////////////////////
				/* cout << "linha " << c.count_line 1<<" instrucao encontrada: " << c.instruction_table[operacao] << endl; */
				c.memory[c.count_pos] = 0;
				c.count_pos += c.inst_size_table[i];
			}
			else if(eh_diretiva(operacao)) {
				int dir_count = exec_diretiva(operacao, tokens, c, c.count_pos, c.count_line);
				if(dir_count < 0){

				}
				else
					c.count_pos+= dir_count;
			}
			else {
				c.err_type = ERRO_SINTATICO;
				c.err_subtype = INSTRUCTION_NOT_FOUND;
				log_error(c); // instrucao / diretiva nao encontrada
			}
			check_sections_order(c);
			c.count_line++;
		}

		set_definitions(c);

		//////////////////////////////      DEBUG            //////////////////////////////////////
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
	return 1;
}


/**************************************************************************
 * funcao principal da segunda passagem do algoritmo de montagem
 * ************************************************************************/
int segunda_passagem(fstream &fonte, Config &c){
	fonte.clear();
	fonte.seekg(0, ios::beg);

	vector<string> tokens; // armazena os elementos separados da linha
	string operacao;
	string token;
	string line;
	const char *delimiter = " ";

	c.count_pos = 0;
	c.count_line = 1;

	int line_has_label = 0;

	cout << BLU << "SEGUNDA PASSAGEM INICIADA" << RESET <<endl;

	while(!fonte.eof()){
		getline(fonte, line);
		to_uppercase(line); // passa para maiusculo
		c.count_line++;
		if(line.empty()|| line != SECTION_TEXT){
			continue;
		}
		break;
	}

	while(!fonte.eof()){
		line_has_label = 0;
		getline(fonte, line);
		to_uppercase(line); // passa para maiusculo

		if(!line.empty() || line == SECTION_DATA){
			c.count_line++;
			continue;
		}
		c.line = line;

		split(line, delimiter, tokens); // separa os elementos da linha em tokens
		token = tokens[0].substr(0, tokens[0].length() -1); // primeiro token da linha
		operacao = tokens[0];

		if(eh_label(tokens[0])){
			line_has_label = 1;
			operacao = tokens[1];
		}
		check_operandos(c, tokens, line_has_label);

	}
	return 1;
}

/**************************************************************************************
 * verifica se a primeira passagem contem erros, permitindo fazer a segunda passagem
 * **********************************************************************************/
int check_error(Config &c){
	int result = 1;
	if(c.eh_modulo > 0){
		if(c.eh_modulo > 1){
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = EXCEEDED_BEGIN_NUM;
			log_error(c);
			result&=0;
		}
		if(c.num_ends == 0){
			c.err_subtype = MISSING_END;
			c.err_type = ERRO_SEMANTICO;
			log_error(c);
			result&=0;
		}
		if(c.num_ends > 1){
			c.err_subtype = EXCEEDED_END_NUM;
			c.err_type = ERRO_SEMANTICO;
			log_error(c);
			result&=0;
		}
	}
	if(c.section_text_count == 0){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = MISSING_SECTION_TEXT;
		log_error(c);
		result&=0;
	}
	if(c.section_text_count > 1){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = EXCEEDED_SECTION_TEXT;
		log_error(c);
		result&=0;
	}
	if(c.section_data_count > 1){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = EXCEEDED_SECTION_DATA;
		log_error(c);
		result&=0;
	}
	if(c.num_errors>0){
		result&=0;
	}
	return result;
}

/**********************************************************************************
 * verifica se a sessao de texto veio antes da sessao de dados
 * *****************************************************************************/
int check_sections_order(Config &c){
	static int count = 0;
	if(c.section_data_count > 0 && c.section_text_count == 0 && count == 0){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = SECTIONS_IN_WRONG_ORDER;
		count++;
		log_error(c);
	}
}

/********************************************************************************
 * verifica validade das tokens da linha
 * ******************************************************************************/
int check_validade_tokens(vector<string> &tokens){
	int result = 1;
	int count = 0;
	int tem_label = 0;
	int primeiro = 1;
	string operacao = tokens[0];
	for(auto s:tokens){
		if(primeiro) {
			primeiro = 0;
			if(eh_label(s)){
				s = s.substr(0, s.length()-1);
				tem_label = 1;
				operacao = tokens[1];
			}
		}
		int se_diretiva = eh_diretiva(operacao);
		if(tem_label){
			if(count == 2 && operacao == "COPY"){
				result&= validate_token(s, TOKEN_TYPE_3);
			}
			else if(count >= 2 && se_diretiva){
				result &=validate_token(s, TOKEN_TYPE_1);
			}else if(count >=2){
				result &=validate_token(s, TOKEN_TYPE_2);
			}
			else{
				result &= validate_token(s, TOKEN_TYPE_4);
			}
		} else{
			if(count == 1 and operacao == "COPY"){
				result&= validate_token(s, TOKEN_TYPE_3);
			}
			else if(count >= 1 && se_diretiva){
				result &=validate_token(s, TOKEN_TYPE_1);
			}else if(count >=1){
				result &=validate_token(s, TOKEN_TYPE_2);
			}
			else{
				result &= validate_token(s, TOKEN_TYPE_4);
			}
		}
		count ++;
	}
	return result;
}

/**********************************************************************************
 * verifica se a token passada eh valida
 * ****************************************************************************/
int validate_token(string s, int option){
	int plus_num = 2;
	int comma_num = 2;
	vector<string> v;
	const char *d = ",";
	if(option == TOKEN_TYPE_1){
		if(is_hex_string(s) || is_number(s)) return 1;
		if(s.length() > 50 || isdigit(s[0]) || (!isalpha(s[0]) && s[0] != '_')) return 0;
		for(auto c:s){
			if(!isdigit(c) && (!isalpha(c) && c != '_')) return 0;
		}
		return 1;
	}
	else if(option == TOKEN_TYPE_2){
		if(s.length() > 50 || isdigit(s[0]) || (!isalpha(s[0]) && s[0] != '_')) return 0;
		for(auto c:s){
			if(!isdigit(c) && (!isalpha(c) && c != '_' && c != '+')) return 0;
			if(c == '+') plus_num--;
		}
		return 1 && plus_num;
	}
	else if(option == TOKEN_TYPE_4){
		if(s.length() > 50 || isdigit(s[0]) || (!isalpha(s[0]) && s[0] != '_')) return 0;
		for(auto c:s){
			if(!isdigit(c) && (!isalpha(c) && c != '_')) return 0;
		}
		return 1;
	}
	else if(option == TOKEN_TYPE_3){
		split(s, d, v);
		if(v.size() != 2) return 0;
		return validate_token(v[0], TOKEN_TYPE_2) && validate_token(v[1], TOKEN_TYPE_2);
	}
}

/********************************************************************************
 * verifica se operando estao definidos na tabela de simbolos
 * ****************************************************************************/
int check_operandos(Config &c, vector<string> &tokens, int line_has_label){
	string str;
	int instr = 0;
	if(line_has_label){
		str = tokens[1];
		instr = get_instruction(c.instruction_table, str);
		if(instr != 0){
			if(instr == COPY){
				switch(validate_copy(tokens[2], c)){
					case WRONG_ARG_NUM:
						c.err_type = ERRO_SINTATICO;
						c.err_subtype = WRONG_ARG_NUM;
						log_error(c);
						break;
					case 0:
						c.err_type = ERRO_SEMANTICO;
						c.err_subtype = MISSING_SIMBOL;
						log_error(c);
						break;
				}
			}
			else if(instr = STOP){
				return 1;
			}
			else {
				Operand arg_1;
				get_operando(tokens[2], arg_1);
				if(!existe_label(c.simbol_table, arg_1.label)){
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = MISSING_SIMBOL;
					log_error(c);
				}
			}
		}
		else if(str == PUBLIC) {
			Operand arg_1;
			get_operando(tokens[2], arg_1);
			if(!existe_label(c.simbol_table, arg_1.label)){
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = MISSING_SIMBOL;
				log_error(c);
			}
			return 1;
		}
	}
	else{
		str = tokens[0];
		instr = get_instruction(c.instruction_table, str);
		if(instr != 0){
			if(instr == COPY){
				switch(validate_copy(tokens[2], c)){
					case WRONG_ARG_NUM:
						c.err_type = ERRO_SINTATICO;
						c.err_subtype = WRONG_ARG_NUM;
						log_error(c);
						break;
					case 0:
						c.err_type = ERRO_SEMANTICO;
						c.err_subtype = MISSING_SIMBOL;
						log_error(c);
						break;
				}
			}
			else if(instr = STOP){
				return 1;
			}
			else {
				Operand arg_1;
				get_operando(tokens[2], arg_1);
				if(!existe_label(c.simbol_table, arg_1.label)){
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = MISSING_SIMBOL;
					log_error(c);
				}
			}
		}
		else if(str == PUBLIC) {
			Operand arg_1;
			get_operando(tokens[2], arg_1);
			if(!existe_label(c.simbol_table, arg_1.label)){
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = MISSING_SIMBOL;
				log_error(c);
			}
			return 1;
		}
	}
	return 0;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  validate_copy
 *  Description:  verifica se o comando copy eh valido
 * =====================================================================================
 */
int validate_copy(string str, Config &c){
	vector<string> v;
	const char *del = ",";

	Operand arg_1, arg_2;
	split(str, del, v);
	if(v.size() != 2) return WRONG_ARG_NUM;

	get_operando(v[0], arg_1);
	get_operando(v[1], arg_2);
	return (existe_label(c.simbol_table, arg_1.label) && existe_label(c.simbol_table, arg_2.label));

}

int get_operando(string str, Operand &op){
	vector<string> v;
	const char *del = "+";

	split(str, del, v);
	if(v.size() == 1){
		op.label = v[0];
		return 1;
	}
	else if(v.size() == 2){
		op.label = v[0];
		op.offset = stoi(v[1]);
		return 1;
	}
	return 0;
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
	int size = SECTION_TEXT.length();
	if(s.find(SECTION_TEXT)!= string::npos && s.length() == size){
		counter++;
		return 1;
	}
	else return 0;
}

/********************************************************************************
 * verifica se a linha eh o comeco da sesao de dados
 * ******************************************************************************/
int check_section_data(string &s, int &counter){
	int size = SECTION_DATA.length();

	if(s.find(SECTION_DATA)!= string::npos && s.length() == size){
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
	t["MUL"] = MUL;
	t["DIV"] = DIV;
	t["JMP"] = JMP;
	t["JMPN"] = JMPN;
	t["JMPP"] = JMPP;
	t["JMPZ"] = JMPZ;
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
	t[MUL] = 2;
	t[DIV] = 2;
	t[JMP] = 2;
	t[JMPN] = 2;
	t[JMPP] = 2;
	t[JMPZ] = 2;
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
	vector<string> diretivas { SPACE, CONST, EXTERN, PUBLIC, BEGIN, END};
	return in_array(s, diretivas);
}

/********************************************************************************
 * verifica a corretuda das diretivas e retorna o numero de possicoes que a diretiva
 * ocupa
 * ******************************************************************************/
int exec_diretiva(string &diretiva, vector<string> &argumentos, Config &c, int count_pos, int count_line){
	int result;
	int arg_size = argumentos.size();
	int aux;
	Operand arg_1;
	if(diretiva == SPACE){
		if(arg_size > 3){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // SPACE ou possui 1 ou nenum argumentos
		}
		else if(arg_size == 3){
			// DEBUG
			/* cout << CYN << "TRATANDO SPACE, argumento: " << argumentos[2] << RESET << endl; */
			if(is_number(argumentos[2])){
				aux = abs(stoi(argumentos[2]));
				for(int j = 0; j < aux; j++){
					c.memory[count_pos + j] = 0;
				}
				return aux;
			}
			else {
				c.err_type = ERRO_SINTATICO;
				c.err_subtype = WRONG_ARG_TYPE;
				log_error(c);
				return WRONG_ARG_TYPE; // SPACE ou possui 1 ou nenum argumentos
			}
			return -100;
		}
		else{
			// DEBUG
			/* cout << CYN << "TRATANDO SPACE, sem argumentos." << RESET << endl; */
			c.memory[count_pos] = 0;
			return 1; // space sem argumentis ocupa 1 espaco em memoria
		}
	}
	else if(diretiva == CONST){
		if(arg_size != 3){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // CONST sempre possui 1 argumento
		} else if(is_hex_string(argumentos[2])){
			c.memory[count_pos] = strtol(argumentos[2].c_str(), NULL, 16);
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else if(is_number(argumentos[2])){
			c.memory[count_pos] = stoi(argumentos[2]);
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else {
			c.err_type = ERRO_LEXICO;
			c.err_subtype = WRONG_TOKEN_FORMAT;
			log_error(c);
			return WRONG_TOKEN_FORMAT;
		}
	}
	else if(diretiva == BEGIN){
		if(arg_size !=2){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM;
		}
		c.eh_modulo++;
		return 0;
	}
	else if(diretiva == EXTERN){
		if(arg_size != 2){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // EXTERN POSSUI UM LABEL E A DIRTETIVA
		}
		set_extern(argumentos[0], c.simbol_table);
		return 0;
	}
	else if(diretiva == PUBLIC) {
		if(arg_size != 2){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // PUBLIC POSSUI 1 ARGUMENTO
		}
		set_public(argumentos[1], c);
		return 0;
	}
	else if(diretiva == END) {
		c.num_ends++;
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
	if(it != simbol_table.end()){
		it->second->outside = 1;
		it->second->val = 0;
	}
}
/********************************************************************************
 * se o label for public, insere na tabela de definicaoes
 * ******************************************************************************/
int set_public(string label, Config &c){
	c.definition_table[label] = 0;
}

/********************************************************************************
 * loga os error encotrados no codigo
 * ******************************************************************************/
void log_error(Config &c){

	switch(c.err_type){
		case ERRO_SINTATICO:
			cout << RED << "<< Erro Sintatico >> ";
			switch(c.err_subtype){
				case WRONG_ARG_NUM:
					cout << "Numero incorreto de argumentos, linha: " << c.count_line << RESET <<endl;
					break;
				case INSTRUCTION_NOT_FOUND:
					cout << "Instrucao ou diretiva não encontrada, linha: " << c.count_line << RESET <<endl;
					break;
				case WRONG_ARG_TYPE:
					cout << "Tipo do argumento invalido, linha: " << c.count_line << RESET <<endl;
					break;
				default:
					break;
			}
			break;
		case ERRO_SEMANTICO:
			cout << RED << "<< Erro Semantico >> ";
			switch(c.err_subtype){
				case TOKEN_ALREADY_EXISTS:
					cout << "Simbolo já existe, linha: " << c.count_line << RESET <<endl;
					break;
				case SECTIONS_IN_WRONG_ORDER:
					cout << "SECTION TEXT e SECTION DATA estao na ordem incorreta, linha: " << c.count_line << RESET <<endl;
					break;
				case EXCEEDED_BEGIN_NUM:
					cout << "Modulo somente pode conter uma diretiva BEGIN " << RESET <<endl;
					break;
				case EXCEEDED_END_NUM:
					cout << "Modulo somente pode conter uma diretiva END " << RESET <<endl;
					break;
				case MISSING_SECTION_TEXT:
					cout << "Arquivo nao possui SECTION TEXT " << RESET <<endl;
					break;
				case EXCEEDED_SECTION_TEXT:
					cout << "Arquivo possui mais de uma SECTION TEXT " << RESET <<endl;
					break;
				case EXCEEDED_SECTION_DATA:
					cout << "Arquivo possui mais de uma SECTION DATA " << RESET <<endl;
					break;
				case MISSING_SIMBOL:
					cout << "Simbolo indefinido, linha: " << c.count_line << RESET <<endl;
					break;
			}
			break;
		case ERRO_LEXICO:
			cout << RED << "<< Erro Lexico >> ";
			switch(c.err_subtype){
				case WRONG_TOKEN_FORMAT:
					cout << "Token invalida, linha: " << c.count_line << RESET <<endl;
					break;
			}
			break;
		default:
			break;
	}
	c.num_errors++;
	return;
}

#endif /* ifndef MONTADOR_CPP*/

