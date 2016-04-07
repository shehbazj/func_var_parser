/*
Data Structure Generator. 

1. read dictionary file and create lookup hash table
	This program takes dictionary file as in input in the form <function:variable:type>
	and <struct_name::element:type> and maintains hash in the form
	<function:variable:type>		---> function:variable => type
	<struct_name::element:type>	    	---> struct_name:element => type

2. read taint trace file which contains symbols in the form
	DSTRUCT:Addr=A:Size|S
	<function:variable1>
	<function:variable2>

3. output
	It gives an output in the form 
	type:A:S where type is obtained from either variable1 or variable2
*/

#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include<algorithm>

using namespace std;

std::map <string , string> variable_type_map;
std::map <string , string> struct_element_map;

// variable value will contain 1 : separator
bool isVariable(string s){
	if(std::count(s.begin(), s.end(), ':') == 2)
		return true;
}

// structure value will contain two :: separators
bool isStruct(string s){
	if(std::count(s.begin(), s.end(), ':') == 3)
		return true;
}

void print_map(map <string , string> M)
{
	for (auto m = M.begin(); m != M.end() ; m++){
		std::cout << m->first  << " => " << m->second << std::endl;
	}
}

// if key has an array remove [ ] subscript
string generateKey(string s){	
	size_t found;
	if((found = s.find("[")) == string::npos){
		return s;
	}
	//std::cout << "ARRAY DETECTED HERE " << s << std::endl;
	return s.substr(0,found);
}

// generate hash map by reading dictionary file
void generateLookUpTable(ifstream &dict_file)
{
	string s;
	size_t found;
	// create lookup hash table
	while(std::getline(dict_file,s)){
		found = s.find_last_of(":");
		string key = generateKey(s.substr(0,found));
		string value = s.substr(found+1);
		if(key.compare(s.substr(0,found)))	// array
			value.append("*");		
		if(isVariable(s)){
			variable_type_map[key] = value;							
		}else if(isStruct(s)){
			struct_element_map[key] = value;							
		}	
	}
	//print_map(variable_type_map);
	//print_map(struct_element_map);
}

bool isDSTRUCT(string s){
	return (s.find("DSTRUCT") != string::npos);
}

//	DSTRUCT:Addr=A:Size|S
string getAddress(string s){
	size_t found = s.find_first_of("=");
	size_t end = s.find_last_of(":");
	return s.substr(found+1,end - found - 1); 
}

string getSize(string s){
	size_t found = s.find_first_of("|");
	return s.substr(found+1);
}

/*
string getKey(string s){
	string var;
	size_t found;
	if(s.find(":") == string::npos)
		return var;
	if(s.find(".") == string::npos && s.find("->") == string::npos)
		return s;
	else{
		var = s.find(":");	
	}
}
*/

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Usage: ./dataStructureGenerator <dictionary_file> <trace_file>\n");
		exit(0);
	}
	
	ifstream dict_file(argv[1]);
	if (!dict_file) {
		cout << "unable to open file";
		exit(1);
	}	

	generateLookUpTable(dict_file);

	//2. read taint trace file which contains symbols in the form
	//	DSTRUCT:Addr=A:Size|S
	//	<function:variable1>
	//	<function:variable2>

	ifstream trace_file(argv[2]);
	
	string s;
	string Address;	
	string Size;
	string key1, key2;
	string diskBlockType;	

	while(std::getline(trace_file,s)){
		if(isDSTRUCT(s)){
			Address = getAddress(s);
			Size = getSize(s);		
/*			key1 = getKey(std::getline(trace_file,s));
			key2 = getKey(std::getline(trace_file,s));
			diskBlockType = getValue(key1, key2);
			if(!diskBlockType.empty())
				std::cout << Address << ":" << Size 
					<< ":" << diskBlockType << std::endl;	
*/
		std::cout << Address <<  ":" << Size << std::endl;
		}
	}
}
