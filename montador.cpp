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
		int space_size = 0;
		int se_const = 0;
};

typedef map<string, shared_ptr<Ts>> SimbolTable;

// represatacao do eperando junto com o offset
class Operand {
	public:
		string label;
		int offset = 0;
};

// celula de memoria, que armazena se e absoluto ou relativo
class MemCell {
	public:
		MemCell(int val, int relativo = 0){
			this->val = val;
			this->relativo = relativo;
		}
		int val;
		int relativo = 0;
};
typedef shared_ptr<MemCell> CellMem;

// class com variaveis de configuracao da montagem (evita de passar mil argumentos a toda chamada de funcao)
class Config {
	public:
		// estruturas de dados
		SimbolTable simbol_table;
		map<int, CellMem> memory;
		map<string, int> instruction_table;
		map<string, int> definition_table;
		map<string, vector<int>> use_table;
		map<int, int> inst_size_table;

		vector<int> lines_with_errors;

		// flags
		int eh_modulo = 0;
		int num_ends = 0;
		int num_errors = 0;

		int se_tem_stop = 0;

		int section_data_count = 0;
		int section_text_count = 0;

		// curent line being read from the code
		string line;
		string operacao;
		int se_tem_label = 0;
		string last_label;

		// contados de linha e de endereco
		int count_pos = 0;
		int count_line = 1;


		int err_type;
		int err_subtype;
};


int monta_arquivo(fstream &fonte, string filename);

int primeira_passagem(fstream &fonte, Config &c);

int segunda_passagem(fstream &fonte, Config &c);

int gera_arquivo_executavel(Config &c, string filename);

void inicializa_tabela_instrucao(map<string, int> &t);

void inicializa_tabela_tamanhos_instrucao(map<int,int> &t);

int check_validade_tokens(vector<string> &tokens);

int check_section_text(string &s, int &counter);

int check_section_data(string &s, int &counter);

int check_sections_order(Config &c);

void log_error(Config &c);

int check_error_primeira_passagem(Config &c);

int check_error_segunda_passagem(Config &c);

int check_valid_line(Config &c);

int get_instruction(map<string, int> &t, string s);

int existe_label(SimbolTable &simbol_table, string token);

void adiciona_label(SimbolTable &simbol_table, string simbol, int posicao);

int eh_diretiva(string &s);

int exec_diretiva(string &diretiva, vector<string> &argumentos, Config &c, int count_pos, int count_line);

int executa_instrucao(Config &c);

int get_address(Config &c, Operand &op);

int run_diretiva(Config &c);

int run_diretiva(Config &c);

int set_extern(string label, SimbolTable &simbol_table);

int set_public(string label, Config &c);

int set_definitions(Config &c);

int check_operandos(Config &c, vector<string> &tokens, int line_has_label);

int check_space(Config &c, Operand &a1);

int check_can_jump(Config &c, Operand arg_1);

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
	check_error_primeira_passagem(c);
	segunda_passagem(fonte, c);
	// so cintunua caso segunda passagem nao tenha erros
	if(!check_error_segunda_passagem(c)){
		cout << RED << "Arquivo nao montado por conter erros, favor corrigir os erros e tentar novamente!!" RESET<<endl;
		return 0;
	}
	gera_arquivo_executavel(c, filename);
	return 1;
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
			c.se_tem_label = 0;

			if(line.empty() // linha vazia nao faz nada
					|| check_section_text(line, c.section_text_count) // comeco da secaon texto só pula
					|| check_section_data(line, c.section_data_count)){ // comecao da secao dados
				c.count_line++;
				continue;
			}


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
				if(existe_label(c.simbol_table, token)){
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = TOKEN_ALREADY_EXISTS;
					log_error(c); // se o label ja existe na TS throw semantic error
					c.count_line++;
					continue;
				}
				adiciona_label(c.simbol_table, token, c.count_pos); // caso nao exista label, adiciona na TS com a posicao atual
				c.last_label = token;
				if(tokens.size() < 2){
					c.count_line++;
					continue;
				}
				operacao = tokens[1];
				c.se_tem_label = 1;
			}

			int i;
			if((i = get_instruction(c.instruction_table, operacao)) != 0){
				int copy_offset = (i == COPY? 1:0);
				if(tokens.size() != unsigned(c.se_tem_label + c.inst_size_table[i] - copy_offset)){
					c.err_type = ERRO_SINTATICO;
					c.err_subtype = WRONG_ARG_NUM;
					log_error(c);
					c.count_line ++;
					continue;
				}
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
	}
	return 1;
}


/**************************************************************************
 * funcao principal da segunda passagem do algoritmo de montagem
 * ************************************************************************/
int segunda_passagem(fstream &fonte, Config &c){
	fonte.clear();
	fonte.seekg(0, ios::beg);
	c.section_data_count = 0;

	vector<string> tokens; // armazena os elementos separados da linha
	string operacao;
	string token;
	string line;
	const char *delimiter = " ";

	c.count_pos = 0;
	c.count_line = 1;

	int line_has_label = 0;

	while(!fonte.eof()){
		getline(fonte, line);
		to_uppercase(line); // passa para maiusculo
		c.count_line++;
		if(line.empty()|| line != SECTION_TEXT || !check_valid_line(c)){
			continue;
		}
		break;
	}

	while(!fonte.eof()){
		line_has_label = 0;
		c.se_tem_label = 0;
		getline(fonte, line);
		to_uppercase(line); // passa para maiusculo

		if(line.empty() || check_section_data(line, c.section_data_count)|| !check_valid_line(c)){
			c.count_line++;
			continue;
		}
		c.line = line;

		split(line, delimiter, tokens); // separa os elementos da linha em tokens
		token = tokens[0].substr(0, tokens[0].length() -1); // primeiro token da linha
		operacao = tokens[0];

		if(eh_label(tokens[0])){
			line_has_label = 1;
			c.se_tem_label = 1;
			operacao = tokens[1];
		}
		c.operacao = operacao;
		if(!check_operandos(c, tokens, line_has_label)){
			c.count_line++;
			continue;
		}else{
			if(get_instruction(c.instruction_table, operacao)){
				// fazer o algoritmo dessa funcao
				executa_instrucao(c);
			}

			else if(eh_diretiva(operacao)){
				run_diretiva(c);
			}
			else {
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = INSTRUCTION_NOT_FOUND;
				log_error(c);
			}
		}
		if(operacao == END) break;
		c.count_line++;
	}
	return 1;
}

/**************************************************************************************
 * nome da funcao eh intuitivo
 * **********************************************************************************/
int gera_arquivo_executavel(Config &c, string filename){
	fstream saida;
	saida.open(filename+".o", fstream::out | fstream::trunc);
	if(!c.eh_modulo){
		for(auto &cell:c.memory){
			saida << cell.second->val << " ";
		}
	}else{
		saida << "TABLE USE"<<endl;
		for(auto &t:c.use_table){
			for(auto p:t.second){
				saida << t.first << " "<< p<<endl;
			}
		}
		saida << "TABLE DEFINITION"<<endl;
		for(auto &t: c.definition_table){
			saida << t.first << " "<<t.second << endl;
		}
		saida << "TABLE REALOCATION"<<endl;
		for(auto &m:c.memory){
			saida << m.second->relativo;
		}
		saida << endl;
		saida << "CODE"<<endl;
		for(auto &cell:c.memory){
			saida << cell.second->val << " ";
		}
		saida << endl;
	}
	saida.close();
	return 1;
}
/**************************************************************************************
 * verifica se a primeira passagem contem erros, permitindo fazer a segunda passagem
 * **********************************************************************************/
int check_error_primeira_passagem(Config &c){
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
	for(auto &it:c.definition_table){
		auto value = c.simbol_table.find(it.first);
		if(value == c.simbol_table.end()){
			cout << RED <<"<<Erro Semantico: >> Simbolo declarado como publico não existe [["<< it.first <<"]]" << RESET <<endl;
			result&=0;
		}else{
			it.second = value->second->val;
		}

	}
	return result;
}
/***************************************************************************
 * checa se segunda passagem possui erros
 * **************************************************************************/
int check_error_segunda_passagem(Config &c){
	if(c.num_errors) return 0;

	if(!c.se_tem_stop && !c.eh_modulo){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = MISSING_STOP;
		log_error(c);
		return 0;
	}
	return 1;
}

/**************************************************************************
 * se a linha atual coteve erros na primeira passagem, ela nao eh avaliada
 * na segunda passagem
 * ***********************************************************************/
int check_valid_line(Config &c){
	return !(find(c.lines_with_errors.begin(), c.lines_with_errors.end(), c.count_line) != c.lines_with_errors.end());
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
		return 0;
	}
	return 1;
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
				// caso a linha contenha somente o label, entao a operacao nao existe.(obviamente)
				if(tokens.size()>1)
					operacao = tokens[1];
				else
					operacao="";
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
	return 1;
}

/********************************************************************************
 * verifica se operando estao definidos na tabela de simbolos
 * ****************************************************************************/
int check_operandos(Config &c, vector<string> &tokens, int line_has_label){
	string str;
	int instr = 0;
	int offset = 0;
	if(line_has_label) offset = 1;

	str = c.operacao;
	instr = get_instruction(c.instruction_table, str);
	if(instr != 0){
		if(instr == COPY){
			switch(validate_copy(tokens[1+offset], c)){ // valida a instrucao copy
				case WRONG_ARG_NUM:
					c.err_type = ERRO_SINTATICO;
					c.err_subtype = WRONG_ARG_NUM;
					log_error(c);
					return 0;
					break;
				case 0:
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = MISSING_SIMBOL;
					log_error(c);
					return 0;
					break;
				default:
					return 1;
					break;
			}
		}
		else if(instr == STOP){ // instrucao STOP nao precisa de validacao
			return 1;
		}
		else { // se nao eh copy e nem STOP, a validacao eh a mesma para o restante das intrucoes
			Operand arg_1;
			get_operando(tokens[1+offset], arg_1);
			if(!existe_label(c.simbol_table, arg_1.label)){
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = MISSING_SIMBOL;
				log_error(c);
				return 0;
			}
		}
		return 1;
	}
	else if(str == PUBLIC) { // para ser declarado como publico, o simbolo precisa existir na tabela de sombolos
		Operand arg_1;
		get_operando(tokens[1+offset], arg_1);
		if(!existe_label(c.simbol_table, arg_1.label)){
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = MISSING_SIMBOL;
			log_error(c);
			return 0;
		}
		return 1;
	}
	return 1;
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
	if(v.size() != 2){
		return WRONG_ARG_NUM;
	}

	get_operando(v[0], arg_1);
	get_operando(v[1], arg_2);
	return (existe_label(c.simbol_table, arg_1.label) && existe_label(c.simbol_table, arg_2.label));
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  get_operando
 *  Description:  gera um operando com endereco mais um offset caso ele exista
 * =====================================================================================
 */
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
	int result = 1;
	for(auto &it: c.definition_table){
		auto aux = c.simbol_table.find(it.first);
		if(aux != c.simbol_table.end()){
			c.definition_table[it.first] = aux->second->val;
		}else{
			result&=0;
		}
	}
	return result;
}

/********************************************************************************
 * Verifica se o operando nao ultrapassa os limites do space
 * ******************************************************************************/
int check_space(Config &c, Operand &a1){
	auto aux = c.simbol_table[a1.label];
	return aux->space_size >= a1.offset;
}

/********************************************************************************
 * Verifica se o operando e uma constante
 * ******************************************************************************/
int check_const(Config &c, Operand &a1){
	auto aux = c.simbol_table[a1.label];
	return aux->se_const;
}

/********************************************************************************
 * Verifica se o endereco e passivel de jump
 * ******************************************************************************/
int check_can_jump(Config &c, Operand arg_1){
	auto aux = c.simbol_table[arg_1.label];
	return (!check_const(c, arg_1) && aux->space_size == 0);
}

/********************************************************************************
 * verifca se linha a é o comeco da sessao de texto
 * ******************************************************************************/
int check_section_text(string &s, int &counter){
	unsigned int size = SECTION_TEXT.length();
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
	unsigned int size = SECTION_DATA.length();

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
	t["MULT"] = MULT;
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
	t[MULT] = 2;
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
				aux = stoi(argumentos[2]);
				if(aux < 1){
					c.err_type = ERRO_SEMANTICO;
					c.err_subtype = SPACE_ARGUMENT_NOT_POSITIVE;
					log_error(c);
					return WRONG_ARG_NUM; // SPACE ou possui 1 ou nenum argumentos
				}
				c.simbol_table[c.last_label]->space_size = aux;
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
			c.simbol_table[c.last_label]->space_size = 1;
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
			c.simbol_table[c.last_label]->se_const = 1;
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else if(is_number(argumentos[2])){
			c.simbol_table[c.last_label]->se_const = 1;
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else {
			c.err_type = ERRO_LEXICO;
			c.err_subtype = WRONG_TOKEN_FORMAT;
			log_error(c);
			return WRONG_TOKEN_FORMAT;
		}
	}
	else if(diretiva == BEGIN){ // diretiva begin nao gera codigo
		if(arg_size !=2){ // nao possui argumentos
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
		set_extern(argumentos[0], c.simbol_table); // adiciona para tabela de uso
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
		if(arg_size != 1){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // PUBLIC POSSUI 1 ARGUMENTO
		}
		c.num_ends++;
		return 0;
	}
	return 0;
}

/************************************************************************
 * Retorna o endereco do aperando
 * *********************************************************************/
int get_address(Config &c, Operand &op){
	auto it = c.simbol_table.find(op.label);
	if(it != c.simbol_table.end()){
		if(it->second->outside){
			if(c.use_table.find(op.label)!= c.use_table.end()){
				c.use_table[op.label].push_back(c.count_pos);
			}else{
				c.use_table[op.label] = vector<int>{c.count_pos};
			}
			return op.offset;
		}
		return it->second->val + op.offset;
	}
	c.err_type = ERRO_SEMANTICO;
	c.err_subtype = MISSING_SIMBOL;
	log_error(c);
	return 0;
}

/************************************************************************
 * joga pra memoria o codigo da instrucao
 * *********************************************************************/
int executa_instrucao(Config &c){
	vector<string> args;
	const char *speice = " ";
	split(c.line, speice, args);
	if(c.section_data_count > 0){
		c.err_type = ERRO_SEMANTICO;
		c.err_subtype = COMAND_ON_WRONG_SECTION;
		log_error(c);
	}
	Operand arg_1, arg_2;
	vector<string> copy_args;
	const char *del = ",";
	int offset = 0;
	if(c.se_tem_label){
		offset = 1;
	}

	if(c.operacao == "COPY"){
		split(args[1+offset], del, copy_args);
		get_operando(copy_args[0], arg_1);
		get_operando(copy_args[1], arg_2);
		if(!check_space(c, arg_1) || !check_space(c, arg_2)) { // verifica se o space tem espaco suficiente
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = INVALID_ADDRESS;
			log_error(c);
			return 0;
		}
		else if(check_const(c, arg_2)){ // nao se pode alterar um endereco que seja constante
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = CANT_CHANGE_CONST;
			log_error(c);
			return 0;
		}else{
			c.memory[c.count_pos++] = CellMem(new MemCell(COPY));
			c.memory[c.count_pos++] = CellMem(new MemCell(get_address(c, arg_1), 1));
			c.memory[c.count_pos++] = CellMem(new MemCell(get_address(c, arg_2), 1));
			return 1;
		}

	}
	else if(c.operacao == "STOP"){
		c.memory[c.count_pos++] = CellMem(new MemCell(STOP));
		c.se_tem_stop = 1;
		return 1;
	}
	else{
		get_operando(args[1+offset], arg_1);
		if(!check_space(c, arg_1)){
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = INVALID_ADDRESS;
			log_error(c);
			return 0;
		}
		int step = get_instruction(c.instruction_table, c.operacao);
		if(step == INPUT || step == STORE){
			if(check_const(c, arg_1)){
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = CANT_CHANGE_CONST;
				log_error(c);
				return 0;
			}
		}
		if(step == JMP || step == JMPN || step == JMPP || step == JMPZ){
			if(!check_can_jump(c, arg_1)){
				c.err_type = ERRO_SEMANTICO;
				c.err_subtype = CANT_JUMP_HERE;
				log_error(c);
				return 0;
			}
		}
		c.memory[c.count_pos++] = CellMem(new MemCell(step));
		c.memory[c.count_pos++] = CellMem(new MemCell(get_address(c, arg_1), 1));
		return 1;
	}
	return 0;
}

int run_diretiva(Config &c){
	vector<string> args;
	const char *speice = " ";
	split(c.line, speice, args);

	/* int offset = 0; */
	/* if(c.se_tem_label){ */
	/* 	offset = 1; */
	/* } */

	if(in_array(c.operacao, vector<string>{BEGIN, EXTERN, PUBLIC, END})){
		return 1;
	}
	else if(c.operacao == SPACE){
		if(c.section_data_count == 0){
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = DATA_ON_WRONG_SECTION;
			log_error(c);
		}
		if(args.size() == 2){
			c.memory[c.count_pos++] = CellMem(new MemCell(0));
		}
		else if(is_number(args[2])){
			int aux = abs(stoi(args[2]));
			for(int j = 0; j < aux; j++){
				c.memory[c.count_pos + j] = CellMem(new MemCell(0));
			}
			c.count_pos+=aux;
		}
		else {
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_TYPE;
			log_error(c);
			return 0; // SPACE ou possui 1 ou nenum argumentos
		}

	}
	else if(c.operacao == CONST){
		if(c.section_data_count == 0){
			c.err_type = ERRO_SEMANTICO;
			c.err_subtype = DATA_ON_WRONG_SECTION;
			log_error(c);
		}
		if(args.size() != 3){
			c.err_type = ERRO_SINTATICO;
			c.err_subtype = WRONG_ARG_NUM;
			log_error(c);
			return WRONG_ARG_NUM; // CONST sempre possui 1 argumento
		} else if(is_hex_string(args[2])){
			c.memory[c.count_pos++] = CellMem(new MemCell(strtol(args[2].c_str(), NULL, 16)));
			return 1;
		} else if(is_number(args[2])){
			c.memory[c.count_pos++] = CellMem(new MemCell(stoi(args[2])));
			return 1; // CONST SEMPRA OCUPA 1 espaco em memoria
		} else {
			c.err_type = ERRO_LEXICO;
			c.err_subtype = WRONG_TOKEN_FORMAT;
			log_error(c);
			return 0;
		}
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
		return 1;
	}
	return 0;
}
/********************************************************************************
 * se o label for public, insere na tabela de definicaoes
 * ******************************************************************************/
int set_public(string label, Config &c){
	c.definition_table[label] = 0;
	return 1;
}

/********************************************************************************
 * loga os error encotrados no codigo
 * ******************************************************************************/
void log_error(Config &c){
	c.lines_with_errors.push_back(c.count_line);
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
				case DATA_ON_WRONG_SECTION:
					cout << "Dado declarado na sessao errada, linha: " << c.count_line << RESET <<endl;
					break;
				case COMAND_ON_WRONG_SECTION:
					cout << "Comando declarado na sessao errada, linha: " << c.count_line << RESET <<endl;
					break;
				case SPACE_ARGUMENT_NOT_POSITIVE:
					cout << "ARGUNENTO DO SPACE DEVE SER POSITIVO, linha: " << c.count_line << RESET <<endl;
					break;
				case INVALID_ADDRESS:
					cout << "Nao e possivel acessar endereco especificado, linha: " << c.count_line << RESET <<endl;
					break;
				case CANT_CHANGE_CONST:
					cout << "Nao e possivel alterar uma constante, linha: " << c.count_line << RESET <<endl;
					break;
				case CANT_JUMP_HERE:
					cout << "Nao e possivel pular para este endereco, linha: " << c.count_line << RESET <<endl;
					break;
				case MISSING_STOP:
					cout << "Programas que nao sao modulo precisam de pelo menos uma instrucao STOP" << RESET <<endl;
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

