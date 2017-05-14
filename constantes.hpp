#include <string>
using namespace std;
#ifndef CONSTANTES_HPP
#define CONSTANTES_HPP

#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define JMP 5
#define JMPN 6
#define JMPP 7
#define JMPZ 8
#define COPY 9
#define LOAD 10
#define STORE 11
#define INPUT 12
#define OUTPUT 13
#define STOP 14

// identificador de error

// sintaticos
#define ERRO_SINTATICO 1
#define WRONG_ARG_NUM -1
#define INSTRUCTION_NOT_FOUND -2
#define WRONG_ARG_TYPE -3

// semanticos
#define ERRO_SEMANTICO 2
#define TOKEN_ALREADY_EXISTS -15
#define SECTIONS_IN_WRONG_ORDER -16

// lexicos
#define ERRO_LEXICO 3
#define WRONG_TOKEN_FORMAT -30


// validacao tokens
#define TOKEN_TYPE_1 1
#define TOKEN_TYPE_2 2
#define TOKEN_TYPE_3 3
#define TOKEN_TYPE_4 4

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define PREPROCESSA 1
#define MONTA 2
const string FLAG_O = "-o";
const string FLAG_P = "-p";
const string EQU = "EQU";
const string IF = "IF";
const string SPACE = "SPACE";
const string CONST = "CONST";
const string SYNTAX_ERROR = "Syntax error: ";
const string SEMANTIC_ERROR = "Semantic error: ";
const string SECTION_TEXT = "SECTION TEXT";
const string SECTION_DATA = "SECTION DATA";
const string BEGIN = "BEGIN";
const string EXTERN = "EXTERN";
const string PUBLIC = "PUBLIC";
const string END = "END";

#endif
