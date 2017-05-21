/******************************************************************
 * Esse modulo contem funcoes de apoio para manipulacao de strings
 ******************************************************************/
#include <string>
#include <sstream>
#include <iterator>
#include <string.h>
#include <vector>
#include <algorithm>
#include <locale>
#include <functional>

#ifndef HELPER_CPP
#define HELPER_CPP
using namespace std;

/***********************************************************************
 * verifica se determinada string esta contida em um array de strings
 * ********************************************************************/
bool in_array(const std::string &value, const std::vector<string> &array)
{
	return std::find(array.begin(), array.end(), value) != array.end();
}

std::string trim(const std::string& str,
		const std::string& whitespace = " \t")
{
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return ""; // no content

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

std::string reduce(const std::string& str,
		const std::string& fill = " ",
		const std::string& whitespace = " \t")
{
	// trim first
	auto result = trim(str, whitespace);

	// replace sub ranges
	auto beginSpace = result.find_first_of(whitespace);
	while (beginSpace != std::string::npos)
	{
		const auto endSpace = result.find_first_not_of(whitespace, beginSpace);
		const auto range = endSpace - beginSpace;

		result.replace(beginSpace, range, fill);

		const auto newStart = beginSpace + fill.length();
		beginSpace = result.find_first_of(whitespace, newStart);
	}
	return result;
}
string join(const vector<string>& vec, const char* delim)
{
    stringstream res;
    copy(vec.begin(), vec.end(), ostream_iterator<string>(res, delim));
    return res.str();
}

/*******************************************************************************
 * separa a string em um array de strings com separacao definida pelo
 * delimitador passado como argumento
 * *****************************************************************************/
void split(const string &s, const char* delim, vector<string> & v){
	// to avoid modifying original string
	// first duplicate the original string and return a char pointer then free the memory
	v.clear();
	char * dup = strdup(s.c_str());
	char * token = strtok(dup, delim);
	while(token != NULL){
		v.push_back(string(token));
		// the call is treated as a subsequent calls to strtok:
		// the function continues from where it left in previous invocation
		token = strtok(NULL, delim);
	}
	free(dup);
}

/**************************************************************************
 * passa string para sua uppercase version
 * *************************************************************************/
void to_uppercase(string &str){
	std::transform(str.begin(), str.end(),str.begin(), ::toupper);
}

/*******************************************************************************
 * verifica se string eh um numero exadecimal
 * *****************************************************************************/
bool is_hex_string(std::string& s) {
	string str = s;
	to_uppercase(str);
	if(s[0] == '-')
		str = s.substr(1,s.length());
	vector<string> v;
	const char *c = "X";
	split(str, c, v);
	if(v.size() > 2 || v[0] != "0") return 0;

	for(auto &x:v[1]) {
		if(!isxdigit(x)) return 0;
	}

	return 1;
}

/*******************************************************************************
 * verifica se string eh um numero Inteiro
 * *****************************************************************************/
bool is_number(const std::string& s)
{
	string str;
	str = s;
	if(s[0] == '-')
		str = s.substr(1, s.length()+1);
	return !str.empty() && std::find_if(str.begin(),
			str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}
/*********************************************************************************
 *  verifica se uma string eh um label
 *  *****************************************************************************/
int eh_label(string &str){
	return (str.back() == ':' && !isdigit(str[0]));
}
#endif
