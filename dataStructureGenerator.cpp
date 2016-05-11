/*
Data Structure Generator. 

1. read dictionary file and create lookup hash table
	This program takes dictionary file as in input in the form <function:variable:type>
	and <struct_name::element:type> and maintains hash in the form
	<function:variable:type>		---> function:variable => type
	<struct_name::element:type>	    	---> struct_name:element => type

2. read taint trace file which contains symbols in the form
	#DSTRUCT:Addr=A:Size|S
	#<function:variable1>
	#<function:variable2>

3. output
	It gives an output in the form 
	type:A:S where type is obtained from either variable1 or variable2
*/

#include<iostream>
#include<string>
#include<fstream>
#include<map>
#include<algorithm>
#include<cstring>
#include<cstdio>
#include<sstream>

#define BLOCK_SIZE 64

using namespace std;

std::map <string , string> variable_type_map;
std::map <string , string> struct_element_map;

string traceFile;

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
	// remove # from beginning
	return s.substr(0,found);
}

// generate hash map by reading dictionary file
void generateLookUpTable(ifstream &dict_file)
{
	string s;
	size_t found;
	// create lookup hash table
	while(std::getline(dict_file,s)){
	//	std::cout << s << std::endl;
		if(s.find(":") == string::npos)
			continue;
		found = s.find_last_of(":");
		// 1 since # needs to be considered
		string key = generateKey(s.substr(0,found));
		string value = s.substr(found+1);
		if(key.compare(s.substr(0,found)))	// array
			value.append("*");		
		if(isVariable(s)){
			//std::cout << "key = " << key << " value = " << value << std::endl;
			variable_type_map[key] = value;							
		}else if(isStruct(s)){
			//std::cout << "key = " << key << " value = " << value << std::endl;
			struct_element_map[key] = value;							
		}	
	}
	//print_map(variable_type_map);
	//print_map(struct_element_map);
}

bool isDSTRUCT(string s){
	return (s.find("DSTRUCT") != string::npos);
}

//	DSTRUCT:t827,8:Size|2
string getTaintNo(string s){
	size_t found = s.find_first_of(":");
	size_t end = s.find_last_of(",");
	string taintNo =  s.substr(found+1,end - found - 1);
	return taintNo;	
}

//	DSTRUCT:t827,8:Size|2
string getOffset(string s){
	size_t found = s.find_first_of(",");
	size_t end = s.find_last_of(":");
	string offset = s.substr(found+1,end-found-1);
	return offset;
}

int getBlockNo(string taintNo){
	taintNo+= "=";	// prepare taintNo for searching through taint string
	ifstream trace_file(traceFile);
	string line;
//	std::cout << "trying to find " << taintNo << " in trace file " << traceFile << std::endl;
	while(std::getline(trace_file,line)){
		if(line.find(taintNo) != std::string::npos){
	//		std::cout << "FOUND " << taintNo << " in " << line << std::endl;
			break;
		}
	}
	// got line like this:t9=B(64,0,t4,t8, 9)
	if(line.find("B") == std::string::npos)
		return -1;
	size_t found = line.find_first_of(",");
	string rest = line.substr(found+1,line.size());
	found = rest.find_first_of(",");
	string blockNum = rest.substr(0,found);
	if(blockNum.size() == 0)		
		return -1;						
	
//	std::cout << "Block Num = " << blockNum << std::endl;
	istringstream buffer(blockNum);
	int bnum;
	buffer >> bnum;
	return bnum;
}

int getAddress(string taintNo, string offset){
	int blockNo = getBlockNo(taintNo);	
	int off;
	stringstream buffer(offset);
	buffer >> off;
	if (blockNo < 0){
//		std::cout << __func__ << "():could not find block " << std::endl;
		return -1;
	}
//	std::cout << "blockNo = " << blockNo << std::endl;
	return (blockNo * BLOCK_SIZE) + off;
}

//	DSTRUCT:Addr=A:Size|S
string getAddress(string s){
	string taintNo = getTaintNo(s);
	string offset = getOffset(s);	
	int addr = getAddress(taintNo, offset);						
	if (addr < 0)
		return string("");
//	std::cout << "Address received" << addr << std::endl;
	return std::to_string(addr);
}

string getSize(string s){
	size_t found = s.find_first_of("|");
	return s.substr(found+1);
}

string normalize(string s){
	char chars[] = " &*.";
   for (unsigned int i = 0; i < strlen(chars); ++i)
   {
      // you need include <algorithm> to use general algorithms like std::remove()
      s.erase (std::remove(s.begin(), s.end(), chars[i]), s.end());
   }
	return s;
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

string getStructName(string s){
	size_t found = s.find("struct ");
	if(found != string::npos){
		vector <string > V = split(s.data());
		return V[1];
	}	
	return std::string("");
}

//bitmap_create:b->v
//testfs_get_inode:&in->in
//testfs_init_super_block:&sb->sb

string getStructElementKey(string s){
	string subElement = s.substr(s.find_last_of(".->") + 1, s.length() - s.find_last_of(".->") - 1);
	string structDef  = normalize(s.substr(0,s.find_first_of(".->")));
//	std::cout << "structDef " << structDef << std::endl;
	string structType = variable_type_map[structDef];
//	std::cout << "structType " << structType << std::endl;
	string structName = getStructName(structType); 
//	std::cout << "structName " << structName << std::endl;
	structName.append("::");
	string structKey = structName.append(subElement);
//	std::cout << "String = " << s  << " struct Key = " << structKey << std::endl;
	return structKey;
}

// get string in the form #testfs_read_data:buf + buf_offset, return testfs_read_data
string getFunName(string s){
	return s.substr(1, s.find_first_of(":") -1);
}

//#testfs_read_data:buf + buf_offset
//#testfs_read_data:block + b_offset
//#testfs_get_inode:block + block_offset
//#testfs_init_super_block:block

string getKey(string s){
	string empty("");
	size_t found;
	if(s.find("+") != string::npos){
			string funName = getFunName(s);
			funName.append(":");
		string t = s.substr(s.find_last_of(":") + 1,s.length());
		string varName = t.substr(0, t.find_first_of(" +"));
		return funName.append(varName.c_str());
	}
	if(s.find(".") != string::npos || s.find("->") != string::npos)
		// remove # from begining before sending
		return getStructElementKey(s.substr(1,s.length())); 
	if(s.find(":") == string::npos)
		return empty;
	else{
		string funName = getFunName(s);
		funName.append(":");
		string varName = s.substr(s.find_last_of(":")+1,s.length());
		return funName.append(varName.c_str());
	}
}

bool isValid(string A){
	return !(A.empty());
}

string getValue(string key1, string key2){
	string value1 = variable_type_map[key1];
	if(value1.empty()){
//		std::cout << "Cant find key" << key1 << " in variable type map" << std::endl;
		value1 = struct_element_map[key1];
	}
	if (value1.empty()){
//		std::cout << "Cant find key" << key1 << " in struct_element_map" << std::endl;
		return value1;	// return NULL
	}

	string value2 = variable_type_map[key2];
	if(value2.empty())
		value2 = struct_element_map[key2];
	if (value2.empty())
		return value1;	// return value 1

	if (value1.find("struct") != string::npos)
		return value1;
	else 
		return value2;
}

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Usage: ./dataStructureGenerator <dictionary_file> <trace_file>\n");
		exit(0);
	}
	
	ifstream dict_file(argv[1]);
	if (!dict_file) {
//		cout << "unable to open file";
		exit(1);
	}	

	generateLookUpTable(dict_file);

	//2. read taint trace file which contains symbols in the form
	//	DSTRUCT:Addr=A:Size|S
	//	<function:variable1>
	//	<function:variable2>

	ifstream trace_file(argv[2]);
	traceFile.append(argv[2]);
	
	string s;
	string Address;	
	string Size;
	string key1, key2;
	string diskBlockType;	

	while(std::getline(trace_file,s)){
		if(isDSTRUCT(s)){
	//		std::cout << "Identified DSTRUCT in s " << s << std::endl;
			Address = getAddress(s);
			Size = getSize(s);
	//		std::cout << "Extracted Addr " << Address << " Extract Size " << Size << std::endl;
			std::getline(trace_file,s);		
	//		std::cout << "Next Line " << s << std::endl;
			key1 = getKey(s);
	//		std::cout << "key1 " << key1 << std::endl;		
	
			if(!key1.empty()){
		//		std::cout << "In string " << s << " Obtained Key " << key1 << std::endl;
				std::getline(trace_file,s);		
				key2 = getKey(s);
		//		std::cout << "In string " << s << " Obtained Key " << key2 << std::endl;
				diskBlockType = getValue(key1, key2);
		//		std::cout << "diskBlockType " << diskBlockType << std::endl; 
				if(!diskBlockType.empty() && isValid(Address))
					std::cout << Address << ":" << Size 
						<< ":" << diskBlockType << std::endl;	
			}
		}
	}
}
