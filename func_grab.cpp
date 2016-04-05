/*
	parser for set of functions and local variables of a C program.
	input - *.c file
	output - function_name : variable_name
	variable_name is both the variables declared as arguments as well as
	local arguments.
*/

#include<iostream>
#include<string>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>


using namespace std;

vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;
		char d = '(';
		char e = '{';
		char f = ')';

    do
    {
        const char *begin = str;

        while(*str != c && *str && *str != d && *str != e && *str != f)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

int main(int argc, char **argv){
	if (argc != 2){
		std::cout << "Enter filename " << std::endl;
		return 1;
	}

	ifstream file(argv[1]);
	if (!file) {
    cout << "unable to open file";
    exit(1);
	}	

	std::string s;
	string fun_name;
	string fun_name_full;

	// in case of function definition expanding multiple lines, this check fails
	// multi line comments iron out fails
	// # def fails. 
	// all this can be resolved if we check for ( after {
		
	while(std::getline(file,s)){
		// if statement is function
		if(s.find("(") != string::npos && s.find(")") != string::npos
				&& s.find(";") == string::npos 
				&& s.find("if ") == string::npos && s.find("for ") == string::npos
				&& s.find("//") == string::npos
				&& s.find("==") == string::npos
				&& s.find("!=") == string::npos
				&& s.find("<") == string::npos
				&& s.find(">") == string::npos
				&& s.find("#define") == string::npos
				&& s.find("#include") == string::npos
				&& s.find("while") == string::npos
				&& s.find("switch") == string::npos
				&& (std::count (s.begin(), s.end(), '(') == std::count (s.begin(), s.end(), ')'))  
			){
//				std::cout << "fname: " << s.data() << std::endl;	
				fun_name.clear();
				fun_name_full.clear();
				vector<string> V = split(s.data() , ',');
				bool fname = true;
				for (auto it : V){
					//std::cout << "token: " << it;
					if (fname){
						fun_name_full.append(it);
						vector <string> V2 = split(fun_name_full.data());
						fun_name.append(V2.back());
						fname = false;
					//std::cout << std::endl;
					}else{
						std::cout << fun_name << ":" << it << std::endl;
					//std::cout << "\t";
					}
				}
				std:: cout << endl;
		// if statement is declaration
		}else if( s.find(";") != string::npos 
				&& (s.find("int ") != string::npos 
						|| (s.find ("float ") !=string::npos)
						|| (s.find ("uint32_t ") !=string::npos)
						|| (s.find ("uint64_t ") !=string::npos)
						|| (s.find ("struct ") !=string::npos)
						|| (s.find ("long ") !=string::npos)
						|| (s.find ("WORD_TYPE ") !=string::npos)
						|| (s.find ("u_int32_t ") !=string::npos)
						|| (s.find ("size_t ") !=string::npos))
				&& (s.find("for") == string::npos)) 
			{
				std::size_t found = s.find_first_of(";=");						
				std::cout << fun_name << ":" << s.substr(0,found) << std::endl;
			}
	}
}
