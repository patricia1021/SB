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

class Module{
	public:
		Module(){}
		~Module(){file.close();}
		fstream file;
		int correction_factor = 0;
		map<string, int> definition_table;
		map<string, vector<int>> use_table;
		vector<int> realocation_table;
		map<int,int> mod_memory;

		string line;

		int se_tem_tu = 0;
		int se_tem_td = 0;

};
typedef shared_ptr<Module> SModule;

class ConfLig {
	public:
		vector<SModule> modules;
		map<string, int> global_df;
		map<int,int> memory;
		int memcount = 0;

};

int verifica_argumentos_ligador (int argc, char *argv[]);

int verifica_extensao_ponto_o(string &s);

int verifica_extensao_ponto_e(string &s);

int check_and_open_files(ConfLig &c, int argc, char *argv[]);

int gera_tgs(ConfLig &c);

int put_in_ut(SModule &m);

int put_in_td(SModule &m);

int put_in_tr(SModule &m);

int put_in_mem(ConfLig &c, SModule &m);

int correct_by_factor(SModule &module);

int push_gdt(ConfLig &c, SModule &m);

int fix_simbol_values(ConfLig &c);

int patch_memory(ConfLig &c);

void build_output_file(ConfLig &c, string filename);

int liga_programa(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	// chama o ligador
	liga_programa(argc, argv);
	return 0;
}

/*********************************************************************************
 * Funcao com nome intuitivo
 ********************************************************************************/
int liga_programa (int argc, char*argv[])
{
	ConfLig c;
	if(verifica_argumentos_ligador(argc, argv) && check_and_open_files(c, argc, argv)){
		gera_tgs(c);
		if(fix_simbol_values(c)){
			patch_memory(c);
			build_output_file(c, string(argv[argc -1]));
		}
		else{
			cout << RED << "Nai foi possivel gerar o arquivo executavel, por haver erros de ligacao" << RESET <<endl;
		}
	}else{
		cout << RED << "arquivos falharam" << endl;
	}
	return 0;
}

/**************************************************************************
 * funcao que gera a tabela de definicoes, de uso e de realocao para cada
 * modulo
 * ***********************************************************************/
int gera_tgs(ConfLig &c){
	string line;
	vector<string>head2{"TABLE DEFINITION", "TABLE REALOCATION", "CODE"};

	for(auto &module:c.modules){
		module->correction_factor = c.memcount; // fator de correcao do modulo
		int action = GET_NEXT_ACTION;
		while(!module->file.eof()){
			int pos = module->file.tellg();
			getline(module->file, line);
			to_uppercase(line);
			line = reduce(line);
			module->line = line;

			switch(action){ // switch que percorre o modulo procurando as tabelas
				case GET_NEXT_ACTION:
					if(line.empty()) break;
					if(line == "TABLE USE"){
						action = GET_USE_TABLE_LINE;
						break;
					}
					else if(line == "TABLE DEFINITION"){
						action = GET_TABLE_DEFINITION_LINE;
						break;
					}
					else if(line == "TABLE REALOCATION"){
						action = GET_TABLE_REALOC_LINE;
						break;
					}
					else if(line == "CODE"){
						action = GET_COD_LINE;
						break;
					}
					break;
				case GET_USE_TABLE_LINE:
					if(line.empty()) break;
					else if(in_array(line, head2)){
						action = GET_NEXT_ACTION;
						module->file.seekg(pos, ios::beg);
						break;
					}else{
						// parei aqui
						put_in_ut(module);
						action = GET_USE_TABLE_LINE;
						break;
					}
					break;
				case GET_TABLE_DEFINITION_LINE:
					if(line.empty()) break;
					else if(in_array(line, head2)){
						action = GET_NEXT_ACTION;
						module->file.seekg(pos, ios::beg);
						break;
					}else{
						put_in_td(module);
						action =  GET_TABLE_DEFINITION_LINE;
						break;
					}
					break;
				case GET_TABLE_REALOC_LINE:
					if(line.empty()) break;
					else if(in_array(line, head2)){
						action = GET_NEXT_ACTION;
						module->file.seekg(pos, ios::beg);
					}else{
						put_in_tr(module);
						action = GET_NEXT_ACTION;
						break;
					}
					break;
				case GET_COD_LINE:
					if(line.empty()) break;
					else put_in_mem(c, module);
					break;
				default:
					break;
			}
		}
		correct_by_factor(module);
		push_gdt(c, module);
	}
	return 1;
}

/**************************************************************************
 * coloca simbolo atual na table de uso do modulo m
 * ***********************************************************************/
int put_in_ut(SModule &m){
	vector<string> v;
	const char *del = " ";
	split(m->line, del, v);
	if(v.size() != 2) return 0;
	if(!is_number(v[1])) return 0;
	if(m->use_table.find(v[0])!= m->use_table.end()){
		m->use_table[v[0]].push_back(stoi(v[1]));
	}
	else{
		m->use_table[v[0]] = vector<int>{stoi(v[1])};
	}
	return 1;
}

/**************************************************************************
 * coloca simbolo atual na tabela de simbolos do modulo m
 * ***********************************************************************/
int put_in_td(SModule &m){
	vector<string> v;
	const char *del = " ";
	split(m->line, del, v);
	if(v.size() != 2) return 0;
	if(!is_number(v[1])) return 0;
	auto it = m->definition_table.find(v[0]);
	if(it == m->definition_table.end()){
		m->definition_table[v[0]] = stoi(v[1]);
		return 1;
	}
	return 0;
}

/**************************************************************************
 * preemche a tabela de realocacao do modulo m
 * ***********************************************************************/
int put_in_tr(SModule &m){
	for(auto c:m->line){
		if(isdigit(c)){
			int aux = (c == '1' ? 1:0);
			m->realocation_table.push_back(aux);
		}
	}
	return 1;
}

/**************************************************************************
 * preenche a memoria do modulo ja levando em consideracao o fator de 
 * correcao
 * ***********************************************************************/
int put_in_mem(ConfLig &c, SModule &m){
	vector<string> v;
	const char *del = " ";
	int counter = 0;
	split(m->line, del, v);
	for(auto el:v){
		if(is_number(el)){
			c.memcount++;
			counter++;
		}
	}
	// tamanho da tabela de realocao precisa ser igual ao tamanho do modulo
	if(m->realocation_table.size() == unsigned(counter)){
		counter = 0;
		for(auto el:v){
			if(is_number(el)){
				if(m->realocation_table[counter] == 0){
					m->mod_memory[counter++] = stoi(el);
				}
				else{
					m->mod_memory[counter++] = stoi(el) + m->correction_factor;
				}
			}
		}
	}
	return 1;
}

/**************************************************************************
 * adiciona tablea de definicoes do modul m para a tabela global de
 * definicoes
 * ***********************************************************************/
int push_gdt(ConfLig &c, SModule &m){
	int result = 1;
	for(auto it: m->definition_table){
		auto el = c.global_df.find(it.first);
		// se o simbolo ja existe na tabela, gera erro
		if(el != c.global_df.end()){
			cout << RED << "<< Erro de ligacao>> SIMBOLO REDEFINIDO: "<<it.first << endl << RESET;
			result &=0;
		}
		else{
			c.global_df[it.first] = it.second;
		}
	}
	return result;
}

/**************************************************************************
 * corrige a tabela de uso e definicao pelo fator de correcao
 * ***********************************************************************/
int correct_by_factor(SModule &module){
	int c_factor = module->correction_factor;
	for(auto &it:module->definition_table){
		int curr_val = module->definition_table[it.first];
		module->definition_table[it.first] = curr_val + c_factor;
	}
	for(auto it:module->use_table){
		int v_count = 0;
		for(auto &el:it.second){
			module->use_table[it.first][v_count++] = el+c_factor;
		}
	}
	return 1;
}

/**************************************************************************
 * Para todos os simbolos das tabelas de uso, corrige os valores nos
 * enderecoes em em que eles aparecem
 * ***********************************************************************/
int fix_simbol_values(ConfLig &c){
	int result = 1;
	for(auto m:c.modules){
		int cf = m->correction_factor;
		for(auto el:m->use_table){
			if(c.global_df.find(el.first)!= c.global_df.end()){
				int el_value = c.global_df[el.first];
				for(auto line: el.second){
					m->mod_memory[line - cf] = (m->mod_memory[line - cf] - cf) + el_value;
				}
			}
			else{
				cout << RED << "<<Erro de ligacao>> Simbolo Usado mas nao definido: "<< el.first << endl;
				result&=0;
			}
		}
	}
	return result;
}

/*********************************************************************************
 * junta a memoria dos modulos em uma moria unica
 ********************************************************************************/
int patch_memory(ConfLig &c){
	int counter = 0;
	for(auto m:c.modules){
		for(auto it:m->mod_memory){
			c.memory[counter++] = it.second;
		}
	}
	return 1;
}

/*********************************************************************************
 * abre os arquivos de entrada para leitura
 ********************************************************************************/
int check_and_open_files(ConfLig &c, int argc, char *argv[]){
	int result = 1;
	for(int i = 1; i < (argc -1); i++){
		Module *m = new Module();
		string s = string(argv[i]);
		m->file.open(s);
		if(!m->file.is_open()){
			cout << "nao foi possivel abrir o arquivo: " << s <<endl;
			result&=0;
		}else{
			c.modules.push_back(SModule(m));
		}
	}
	return result;
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
	return 1;
}

/*********************************************************************************
 * Cria o arquivo de saida e preenche com os opecodes
 ********************************************************************************/
void build_output_file(ConfLig &c, string filename){
	cout << "FILENAME: "<< filename << endl;
	fstream saida(filename, fstream::out| fstream::trunc);
	if(saida.is_open()){
		cout << YEL << "MEMORY" << endl << RESET;
		for(auto el: c.memory){
			saida << el.second << " ";
		}
	}
	saida.close();
}

/*********************************************************************************
 * Funcao com nome intuitivo
 ********************************************************************************/
int verifica_extensao_ponto_o(string &s){
	return (s.substr(s.length()-2, s.length()).find(".o") != string::npos);
}

/*********************************************************************************
 * Funcao com nome intuitivo
 ********************************************************************************/
int verifica_extensao_ponto_e(string &s){
	return (s.substr(s.length()-2, s.length()).find(".e") != string::npos);
}

#endif
