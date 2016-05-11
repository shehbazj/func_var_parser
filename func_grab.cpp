/*
   parser for set of functions and local variables of a C program.
   also parses *.h files for struct information.
   input - *.c file
   output - function_name : variable_name
   variable_name is both the variables declared as arguments as well as
   local arguments.

	Function declaration cannot expand multiple lines
	multi line comments, single line comments not checked
	global struct variable members also listed as declarations
	if same variable is declared multiple times with different
	types, the tool does not differentiate between them
		
	
 */

#include<iostream>
#include<string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <regex>

using namespace std;

bool checkWarnings(string s);

template<typename FwdIter>
// replace whitespace by one space
FwdIter normalize_space(FwdIter begin, FwdIter end)
{
	FwdIter dst = begin;
IGNORE_LEADING_WHITESPACE:
	if (begin == end) return dst;
	switch (*begin)
	{
		case ' ':
		case '\t':
			++begin;
			goto IGNORE_LEADING_WHITESPACE;
	}
COPY_NON_WHITESPACE:
	if (begin == end) return dst;
	switch (*begin)
	{
		default:
			*dst++ = *begin++;
			goto COPY_NON_WHITESPACE;
		case ' ':
		case '\t':
			++begin;
			// INTENTIONAL FALLTHROUGH
	}
LOOK_FOR_NEXT_NON_WHITESPACE:
	if (begin == end) return dst;
	switch (*begin)
	{
		case ' ':
		case '\t':
			++begin;
			goto LOOK_FOR_NEXT_NON_WHITESPACE;
		default:
			*dst++ = ' ';
			*dst++ = *begin++;
			goto COPY_NON_WHITESPACE;
	}
}

vector<string> split(const char *str, char c = ' ')
{
	vector<string> result;
	char d = '(';
	char e = '{';
	char f = ')';
	do
	{
		const char *begin = str;
		while(*str != c
			&& *str
			&& *str != d
			&& *str != e
			&& *str != f)
			str++;
			result.push_back(string(begin, str));
	} while (0 != *str++);
	return result;
}

void printDecl(string _s, string fun_name){
	_s.erase(_s.find_last_not_of(" \n\r\t")+1);
	size_t c = 0;
	if(!_s.empty()){	
		vector <string> V3 = split(_s.data());
		if (V3.size() > 1){
			std::cout << fun_name << ":";
			if(!V3.back().empty()){
				if(V3.back().find("*") != string::npos){
					c = count(V3.back().begin(),V3.back().end(),'*');
					V3.back().erase(0,V3.back().find_last_of("*") +1);
				}
				std::cout << V3.back() << ":";
				V3.pop_back();
			}
			for(auto it : V3){
				std::cout << it << " ";
				if(it.find(",") != string::npos){
					std::cout << "\t\tXXX COMMA DETECTED" << std::endl;
				}else if(it.find("[") != string::npos){
					std::cout << "\t\tXXX ARRAY DETECTED" << std::endl;
				}
			}
			while(c--){
				std::cout << "*";
			}
			std::cout << std::endl; 
		}
	}
}

bool isStructStatement(string s){
	if(s.find("struct ") !=string::npos
		&& std::count (s.begin(), s.end(), '{'))
		return true;
}

bool isFunctionStatement(string s){
	if((s.find("(") != string::npos 
		&& s.find(")") != string::npos)
		&& s.find(";") == string::npos 
		&& s.find("if ") == string::npos 
		&& s.find("for ") == string::npos
		&& s.find("//") == string::npos
		&& s.find("==") == string::npos
		&& s.find("!=") == string::npos
		&& s.find("<") == string::npos
		&& s.find(">") == string::npos
		&& s.find("#define") == string::npos
		&& s.find("#include") == string::npos
		&& s.find("while") == string::npos
		&& s.find("switch") == string::npos)
	 { 
		if(std::count (s.begin(), s.end(), '(')
			== std::count (s.begin(), s.end(), ')'))
			return true;
		else
			checkWarnings(s);
	 }
	return false;
}

bool isDeclarationStatement(string s){
	if( (s.find(";") != string::npos)// semi-colon exists
		// none of the following expressions exist
		&& (s.find("+=") == string::npos) 
		&& (s.find("-=") == string::npos) 
		&& (s.find("for") == string::npos)
		&& (s.find("memcpy") == string::npos)
		&& (s.find("bzero") == string::npos)
		&& (	// one of following expressions exist
		(s.find ("float ") !=string::npos)
		|| (s.find ("uint32_t ") !=string::npos)
		|| (s.find ("uint64_t ") !=string::npos)
		|| (s.find ("inode_type ") !=string::npos)	// XXX hardcoded
		|| (s.find ("struct ") !=string::npos)
		|| (s.find ("int ") !=string::npos)
		|| (s.find ("char ") !=string::npos)
		|| (s.find ("long ") !=string::npos)
		|| (s.find ("WORD_TYPE ") !=string::npos)
		|| (s.find ("u_int32_t ") !=string::npos)
		|| (s.find ("size_t ") !=string::npos)))
		return true;
	return false;
}
	
bool checkWarnings(string s){
	bool warn = false;
	if(s.find("/*") !=string::npos
		&& s.find("*/") != string::npos
		&& s.find(";") != string::npos )
		warn = true;
	else if(std::count (s.begin(), s.end(), '(')
		!= std::count (s.begin(), s.end(), ')'))
		warn = true;
	if(warn)
		std::cout << s << "WARNING" << std::endl;
}

void formatAndPrint(string s, string fun_name){
	auto new_end = normalize_space(s.begin(), s.end());
	s.erase(new_end, s.end());
	string _s;
	if(!s.empty()){
		s.erase(s.find_first_of(";="), s.length());
		printDecl(s,fun_name);
	}
}

int main(int argc, char **argv){
	if (argc < 2){
		std::cout << "Enter filename " << std::endl;
		return 1;
	}

	int fileNo=1;
	while(fileNo!= argc){
		ifstream file(argv[fileNo]);
		if (!file) {
			cout << "unable to open file";
			exit(1);
		}	
	
		std::string s;
		string fun_name;
		string fun_name_full;
		while(std::getline(file,s)){
			// if statement is function
			if(isFunctionStatement(s)){
				fun_name.clear();
				fun_name_full.clear();
				vector<string> V = split(s.data() , ',');
				bool fname = true;
				for (auto it : V){
					if (fname){
						fun_name_full.append(it);
						vector <string> V2 = split(fun_name_full.data());
						fun_name.append(V2.back());
						fun_name.erase(std::remove(fun_name.begin(), fun_name.end(), '*'), fun_name.end());
						fname = false;
					}else{
						string _s(it);
						while	(!_s.empty() && _s.at(0) == ' '){
							_s.erase(_s.begin(), _s.begin() +1 );
						}
						
						if(!_s.empty()){
							printDecl(_s, fun_name);
						}
					}
				}
				std:: cout << endl;
			}
			// if statement is structure declaration
			else if(isStructStatement(s)){
				auto new_end = normalize_space(s.begin(), s.end());
				s.erase(new_end, s.end());
				vector<string> V3 = split(s.data());
				string struct_name = V3[1];
				struct_name.append(":");
				string decl;
				std::getline(file,decl);
				while(isDeclarationStatement(decl)){
					formatAndPrint(decl,struct_name);
					std::getline(file,decl);
				}
			}
			// if statement is declaration
			else if(isDeclarationStatement(s)){
				formatAndPrint(s, fun_name);
			}
		}
		fileNo++;
	}
}
