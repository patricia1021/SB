#include <string>
using namespace std;
#ifndef CONSTANTES_HPP
#define CONSTANTES_HPP

#define ADD 1
#define SUB 2
#define MULT 3
#define DIV 4
#define JUMP 5
#define JUMPN 6
#define JUMPP 7
#define JUMPZ 8
#define COPY 9
#define LOAD 10
#define STORE 11
#define INPUT 12
#define OUTPUT 13
#define STOP 14

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
const string SYNTAX_ERROR = "Syntax error: ";
const string SEMANTIC_ERROR = "Semantic error: ";
const string SECTION_TEXT = "SECTION TEXT";
const string SECTION_DATA = "SECTION DATA";

#endif