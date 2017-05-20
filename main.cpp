#include <iostream>
#include <fstream>
#include "constantes.hpp"
#include "helper.cpp"
#include "preprocessador.cpp"
#include "montador.cpp"

using namespace std;

void exec(int argc, char *argv[]);

int verfica_argumentos(char *argv[]);

int inicializa_arquivo(char *argv[], fstream &fonte);

int preprocessa_arquivo(fstream &fonte, fstream &saida_pre, char *argv[]);

int verifica_extensao(string &s);

string get_estencao(string s);

int montagem(fstream &fonte, fstream &saida_pre, char *argv[]);

int main(int argc, char *argv[])
{
	exec(argc, argv);
	return 0;
}


/*********************************************************************************
 * Funcao que verifica a quantidade e a validade dos arguetos passados ao programa
 ********************************************************************************/
int verifica_argumentos(int argc, char *argv[]){
	vector<string> valid_args {"-o", "-p"};

	if(argc != 4){
		cout << RED << "Numero incorreto de argumentos!!!!\n\nexecute o programa novamente com o numero correto de argumentos\n" << RESET;
		return 0;
	}
	else if(!in_array(argv[1], valid_args)) {
		cout << "opcoes de compilacao invalidas" << endl;
		return 0;
	}
	return 1;
}

/*******************************************************************
 * retorna extencao do arquivo relativa ao arquivo passado como 
 * parametro
 * ****************************************************************/
string get_estencao(string s){
	return s.substr(s.length()-4, s.length());
}

/*****************************************
 * retorna inteiro referenre a flag
 * *************************************/
int get_flag(char *flag){
	string s = string(flag);
	if(s == FLAG_O){
		return MONTA;
	} else if(s == FLAG_P){
		return PREPROCESSA;
	}
	else return 0;
}

/**********************************************************************************
 * verifica se a entecao do arquivo passado para o programa = .asm ou .pre
 * ********************************************************************************/
int verifica_extensao(string &s){
	return (s.substr(s.length()-4, s.length()).find(".asm") != string::npos ||
			s.substr(s.length()-4, s.length()).find(".pre") != string::npos );
}

/**********************************************************************************
 * funcao que inicializa o arquivo fonte, vereficando a sua existencia e se possui
 * a extencao adequada
 **********************************************************************************/
int inicializa_arquivo(char *argv[], fstream &fonte){
	string s(argv[2]);
	string line;
	if (verifica_extensao(s)){ // verifica se a extencao do arquivo esta correta
		fonte.open(s);
		if(fonte.is_open()){
			cout << GRN << "Arquivo aberto com sucesso\n" << RESET;
			return 1;
		}
		else {
			cout << RED << "Nao foi possivel abrir arquivo \n" << RESET;
		}
	} else {
		cout << RED << "arquivo passado nao possui extensao .asm ou .pre \n" << RESET;
	}
	return 0;
}

int montagem(fstream &fonte, fstream &saida_pre, char *argv[]){
	string ext = get_estencao(argv[2]);
	if(ext == ".asm"){
		preprocessa_arquivo(fonte, saida_pre, argv);
		monta_arquivo(saida_pre, string(argv[3]));
	}
	else if(ext == ".pre"){
		monta_arquivo(fonte, string(argv[3]));
	}
	return 1;
}

/**********************************************************************************
 * roda o montador/ligador
 * *******************************************************************************/
void exec(int argc, char *argv[]){
	fstream fonte;
	fstream pre_processado;
	string line;
	int flag;

	if(verifica_argumentos(argc, argv) && inicializa_arquivo(argv, fonte)){
		flag = get_flag(argv[1]);
		switch(flag){
			case PREPROCESSA:
				preprocessa_arquivo(fonte, pre_processado, argv);
				break;
			case MONTA:
				montagem(fonte, pre_processado, argv);
				break;
			default:
				cout << "Flag nao reconhecida" <<endl;

		}
	}
	fonte.close();
	pre_processado.close();
}
