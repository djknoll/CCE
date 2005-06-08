#include "bootstrap.h"
#include "sym.h"

const int MAX_SYMBOLICNAMES = 500 ;
const int MAX_TYPES = 100 ;
const int MAX_FUNCTIONS = 100 ;
const int MAX_STRUCTFIELDS = 500;

const int MAIN_TYPE=5;   //index of the last baseType
						 //string is no baseType (called by reference)

const int R_FP = 30; //register framepointer
const int R_HP = 29; //register baseaddress for global variables

struct ObjectDescription {
	char name[40];
	int class;      // CONST, VAR , Parameter, returnValue
	int type;       // index in typeTable
	int intValue;   // value of constant integer
	int address;    // actual memory address of symbol / object
    bool array;     // 0:no array, 1:array
    int length;     // number of array elements
    int funcID;     // index in functionTable (-1: global)
};

struct TypeDescription {
	char name[40];
	int form;       // int, bool, char, string, file, struct, void
	int size;       // number of bytes to allocate (at compile-time)
};

struct StructDescription {
	char name[40];
	int structID;    // index in typeTable
	int type;        // index in typeTable
	bool array;      // 0:no array, 1:array
	int length;      // number of array elements (0:no array)
	int offset;
};

struct FunctionDescription {
	char name[40];
	int type;               //number in typeTable
	int address;            //number of currentInstruction
	int parameterMemSpace;  //required space for parameter
	bool implemented;       //0:only forwardDec, 1:function implemented
};

struct item {
	int type;
	int mode;   		//MODE_REG,MODE_CONST,MODE_VAR
	int address;
	int reg;
	int adrType;		//0 global var, 1 local var, 2 absolute
	char strValue[40];	//StringLiteral or Char (if constant)
	int array;
};

struct ObjectDescription symbolTable[500];
struct TypeDescription typeTable[100]; 
struct StructDescription structTable[500];
struct FunctionDescription funcTable[100];

int foundSymbolicNames = 0;
int foundTypeDeclarations = 0;
int foundFunctions=0;
int foundStructFields = 0;
int foundParameters = 0;
int foundImplementedFunction=0;

int currentDataAddress = 0;
int currentGlobalAddress = 0;
int currentParameterAddress = 8;
int actualFunction = -1;
int currentInstruction = 0;
int callParameterAddress = 0;

file outFile;
bool registers[32];

int strSize;
int stringID;
int charID;
int intID;	
int boolID;	
int fileID;
int voidID;

int lookupSymbolicName(char symName[],int funcID,int pClass,int startIndex);

void saveType(char name[],int form, int size) {
	if(CODE_DEBUG >= 2) {
		printf(">saveSimpleType: %s\n", name);
	}
	
	copy(typeTable[foundTypeDeclarations].name , name);
	
	typeTable[foundTypeDeclarations].form = form;
	typeTable[foundTypeDeclarations].size = size;
	foundTypeDeclarations = foundTypeDeclarations + 1;
	if(CODE_DEBUG >= 2) {
		printf("<saveSimpleType and Structs\n");
	}
}


void saveFunction(int pType, char pStringVal[]) {
	if(CODE_DEBUG >= 2) {
		printf(">saveFunction: %s\n", pStringVal);
	}
	
	copy(funcTable[foundFunctions].name,pStringVal);
	
	funcTable[foundFunctions].type = pType;
	funcTable[foundFunctions].implemented = 0;

	foundFunctions = foundFunctions + 1;
	if(CODE_DEBUG >= 2) {
		printf("<saveFunction\n");
	}
}



int lookupType(char name[]) {

	struct TypeDescription curr;

	if(CODE_DEBUG >= 2) {
		printf(">lookupType: %s\n", name);
	}
	int found = 0;
	int pos = 0;

	while((pos < foundTypeDeclarations) & (pos < MAX_TYPES) & (found == 0)) {
		curr = typeTable[pos];
		//printf("## _%s_ ?=? _%s_\n",symName,curr.name);
		if(compare(name, curr.name) == 1) {
			found = 1;
		}
		pos = pos +1;
	}
	
	pos = pos - 1;
	
	if(found == 0) {
		pos = -1;
	}

	if(CODE_DEBUG >= 2) {
		if(found == 1) {
			printf("<lookupTypeName found -- %s\n", curr.name);
		} else {
			printf("<lookupTypeName not found\n");
		}
	}
	return pos;
}

int lookupFunction(char name[]) {

    int found = 0;
	int pos = 0;
	struct FunctionDescription curr;

	if(CODE_DEBUG >= 2) {
		printf(">lookupType: %s\n", name);
	}

	while((pos < foundFunctions) & (pos < MAX_FUNCTIONS) & (found == 0)) {
		curr = funcTable[pos];
		//printf("## _%s_ ?=? _%s_\n",symName,curr.name);
		if(compare(name, curr.name) == 1) {
			found = 1;
		}
		pos = pos +1;
	}
	
	pos = pos - 1;
	
	if(found == 0) {
		pos = -1;
	}
	
	if(CODE_DEBUG >= 2) {
		if(found == 1) {
			printf("<lookupFunctionName found -- %s\n", curr.name);
		} else {
			printf("<lookupFunctionName not found\n");
		}
	}
		
	return pos;
}


int lookupStructFieldName(char symName[],int structID) {
	int found = 0;
	int pos = 0;
	struct StructDescription curr;
	
	if(CODE_DEBUG >= 2) {
		printf(">lookupStructName: %s\n", symName);
	}
	while((pos < foundStructFields) & (pos < MAX_STRUCTFIELDS) & (found == 0)) {
		curr = structTable[pos];
		//printf("## _%s_ ?=? _%s_\n",symName,curr.name);
		if((structID==curr.structID) && (compare(symName, curr.name) == 1)) {
			found = 1;
		}
		pos = pos +1;
	}
	
	pos = pos - 1;
	
	if(found == 0) {
		pos = 0-1;
	}
	
	if(CODE_DEBUG >= 2) {
		if(found == 1) {
			printf("<lookupStructName found -- %s\n", curr.name);
		} else {
			printf("<lookupStructName not found\n");
		}
	}
	return pos;
}

int lookupSymbolicName(char symName[],int funcID,int pClass,int startIndex) {
	int found = 0;
	int pos = startIndex;
	struct ObjectDescription curr;
	curr.type = 0-1;
	
	if(CODE_DEBUG >= 2) {
		printf(">lookupSymbolicName: %s\n", symName);
	}
	while((pos < foundSymbolicNames) & (pos < MAX_SYMBOLICNAMES) & (found == 0)) {
		curr = symbolTable[pos];
		//printf("## _%s_ ?=? _%s_\n",symName,curr.name);
		if(pClass == 0){	//ignore class	   
		   if(symName[0]=='0'){ //ignore name		   
		   		if(funcID==curr.funcID) {
					found = 1;
		   		}
			} else {   //check name
		   		if((funcID==curr.funcID) & (compare(symName, curr.name) == 1)) {
					found = 1;
				}		   
		   }		
		} else {   //check also class
			if(symName[0]=='0'){ //ignore name	
				if((funcID==curr.funcID) & (pClass == curr.class)) {
					found = 1;
				}
			}else{  //check name
				if((funcID==curr.funcID) & (pClass == curr.class) & (compare(symName, curr.name) == 1)) {
					found = 1;
				}
			}
		}
		pos = pos +1;
	}

	pos = pos - 1;
	
	if(found == 0) {
		pos = 0-1;
	}
	
	if(CODE_DEBUG >= 2) {
		if(found == 1) {
			printf("<lookupSymbolicName found -- %s\n", curr.name);
		} else {
			printf("<lookupSymbolicName not found\n");
		}
	}
	return pos;
}


void saveObjectDescription(struct ObjectDescription objDesc) {
	int tmpID=0;
	
	if(CODE_DEBUG >= 2) {
		printf(">saveObjectDescription: %s\n", objDesc.name);
	}
	if(foundSymbolicNames >= MAX_SYMBOLICNAMES) {
		reportError("number of possible symbols exceeded!", true);
	}
	

    if (lookupSymbolicName(objDesc.name,actualFunction,0,0) < 0){      		
		symbolTable[foundSymbolicNames] = objDesc; //save objDescription
		foundSymbolicNames = foundSymbolicNames + 1;
    } else {
    	reportError("Symbol already defined!", true);
    }

	if(CODE_DEBUG >= 2) {
		printf("<saveObjectDescription, addr: %d\n", symbolTable[foundSymbolicNames-1].address);
	}
}


void resetLocalAddress(){
	currentDataAddress = 0;
	currentParameterAddress = 8;
}
    
int getReturnValue(){
	return getAddress(12,3);
}

void openOutputFile(char filename[]) {
	int pos = 0;
	outFile = fopen(filename, "wb");
	currentInstruction = 0;
	
	while(pos < 32) {
		registers[pos] = false;
		pos = pos + 1;
	}
	registers[0] = true;
	registers[29] = true;
	registers[30] = true;
	registers[31] = true;
}

void closeOutputFile() {
	fclose(outFile);
}

int getFreeRegister() {
	int pos = 0;
	int returnValue = 0-1;
	while((pos < 32) & (returnValue < 0)) {
		if(registers[pos] == false) {
			returnValue = pos;
			registers[pos] = true;
		}
		pos = pos + 1;
	}
	if (returnValue < 0) {
		reportError("No free register found!!!!!", true);
	}
	return returnValue;
}

void releaseRegister(int regNr) {
	registers[regNr] = false;
}

void writeInstruction(int opcode, int a, int b, int c, char msg[]) {
	int byte1;
	int byte2;
	int byte3;
	int byte4;
	int byte5;
	int byte6;
	int byte7;
	int help1;

	if(CODE_DEBUG >= 1) {
		printf(">writeInstruction:\n %s\n", msg);
	}
	if(CODE_DEBUG >= 2) {
		printf("instruction: %d, %d, %d, %d\n", opcode, a, b, c);
	}
	if(CODE_DEBUG >= 3) {
		printf("opcode: %d ", opcode);
	}
	
	if(c < 0) {
		help1 = c;
		c = c + 0x80000000;
	}
		
	byte7 = c % 0x100;
	byte6 = (c / 0x100) % 0x100;
	byte5 = (c / 0x10000) % 0x100;
	byte4 = (c / 0x1000000) % 0x100;
	byte3 =  0x0;
	byte2 = b % 0x100;
	byte1 = a % 0x100;
    
    if(help1 < 0 ) {	
		byte4 = byte4 + 128;
	}

	fputc(opcode, outFile);
	fputc(byte1, outFile);
	fputc(byte2, outFile);
	fputc(byte3, outFile);
	fputc(byte4, outFile);
	fputc(byte5, outFile);
	fputc(byte6, outFile);
	fputc(byte7, outFile);
		
	currentInstruction = currentInstruction + 1;
	if(CODE_DEBUG >= 3) {
		printf("byte1: %d ", (unsigned char)byte1);
		printf("byte2: %d ", (unsigned char)byte2);
		printf("byte3: %d\n", (unsigned char)byte3);
	}
	if(CODE_DEBUG >= 1) {
		printf("<writeInstruction, nr: %d\n", currentInstruction-1);
	}
	return;
}

struct item loadIntoRegister(struct item it) {
	if(it.mode == MODE_CONST) {
		it.reg = getFreeRegister();
		writeInstruction(INSTR_ADDI, it.reg, 0, it.address, "load constant argument into register");
	} else if(it.mode == MODE_VAR) {
		it.reg = getFreeRegister();
		if(it.adrType == ADR_GLOBAL) { //global
       		writeInstruction(INSTR_LDW, it.reg, R_HP, it.address, "load global variable from memory");
		} else if( it.adrType == ADR_LOCAL) {
			writeInstruction(INSTR_LDW, it.reg, R_FP, it.address, "load local variable from memory");
		} else {
			writeInstruction(INSTR_LDW, it.reg, 0, it.address, "load absolute variable from memory");
		}
	}
	it.address = -1;
	it.mode = MODE_REG;
	return it;
}

void showSymbolTable(){
	int i=0;
	printf("\nSymbolTable:\n");
	while(i<MAX_SYMBOLICNAMES & symbolTable[i].name[0]!=0 ) {
		printf("name:%s, class:%i, type:%i, intValue:%i, address:%i, array:%i, length:%i, funcID:%i\n",
	         symbolTable[i].name, symbolTable[i].class, symbolTable[i].type, symbolTable[i].intValue,
	         symbolTable[i].address, symbolTable[i].array, symbolTable[i].length, symbolTable[i].funcID);
    	i=i+1;
	}
}
	
void showTypeTable(){
	int i=0;
	printf("\nTypeTable:\n");
	while(i<MAX_TYPES & typeTable[i].name[0]!=0 ){ 
		printf("name:%s, form:%i, size:%i\n", typeTable[i].name, typeTable[i].form, typeTable[i].size);
    	i=i+1;
	}
}

void showFuncTable(){
	int i=0;
	int j=0;
	printf("\nFuncTable:\n");
	while(j<MAX_FUNCTIONS & funcTable[j].name[0]!=0 ){
		printf("\nID:%i, name:%s, type:%i, address:%i, implemented:%i, MemParameter:%i\n",
			j, funcTable[j].name, funcTable[j].type, funcTable[j].address,
			funcTable[j].implemented,funcTable[j].parameterMemSpace);
        i=0;
		while(i<MAX_SYMBOLICNAMES & symbolTable[i].name[0]!=0 ){
	 		if( (symbolTable[i].funcID==j) & symbolTable[i].class==CLASS_PAR){
				printf("name:%s, class:%i, type:%i, intValue:%i, address:%i, array:%i, length:%i, funcID:%i\n",
					symbolTable[i].name, symbolTable[i].class, symbolTable[i].type, symbolTable[i].intValue,
					symbolTable[i].address, symbolTable[i].array, symbolTable[i].length, symbolTable[i].funcID);
			}
	 		i=i+1;
		}
    	j=j+1;
	}
}


void showStructTable(){
	int i=0;
	printf("\nStructTable:\n");
	while( (i<MAX_STRUCTFIELDS) & (structTable[i].name[0]!=0) ){  
		printf("name:%s, structID:%i, fieldID:%i, length:%i, array:%i, offset:%i\n",
	         structTable[i].name,
	         structTable[i].structID, structTable[i].type, 
	         structTable[i].length,structTable[i].array,
	         structTable[i].offset);
    	i=i+1;
	}
}

int getAddress(int p_size,int adrType){ 
	int calcAdr;

	//0 = global,1=local,2 = Parameter (functiondeclaration), 3 = Parameter (functioncall)
	// invariant: currrent...addresses are always aligned
	// p_size >= 0
    
	// align p_size
	if(( p_size % 4) != 0){
		p_size = p_size + (4-(p_size%4) );
	}

	if(adrType == 0){	
		currentGlobalAddress = currentGlobalAddress - p_size;
		calcAdr = currentGlobalAddress;
	}
	else if (adrType == 1) {

		currentDataAddress = currentDataAddress - p_size;
		calcAdr = currentDataAddress;
	
	}else if(adrType == 2){
		
		calcAdr = currentParameterAddress;
		currentParameterAddress = currentParameterAddress + p_size;
		
	}else if (adrType == 3){
		
		calcAdr = callParameterAddress ;
		callParameterAddress = callParameterAddress+p_size;					    
		
	}else{
		reportError("Unknown fatal error occurred", true);
	}    
    return calcAdr;
}

void writeToAbsPlaceRelAdr(int relPlace,int relPlaceAdrType,int relAddress,int relAdrType){  //adrType: 0 glob,1 local var
	int tmpRegA=getFreeRegister();		
	writeInstruction(INSTR_ADDI, tmpRegA , 0, relAddress, "");
	
	if(relAdrType==0){
		writeInstruction(INSTR_ADD,  tmpRegA,tmpRegA,R_HP, "change to abs address");
	}else{
        writeInstruction(INSTR_ADD,  tmpRegA,tmpRegA,R_FP, "change to abs address");
	}
	
	if(relPlaceAdrType==0){
		writeInstruction(INSTR_STW,  tmpRegA ,R_HP, relPlace, "write position of data to global area");
	}else{
        writeInstruction(INSTR_STW,  tmpRegA , R_FP, relPlace, "write position of data to local area");
	}						
   releaseRegister(tmpRegA);                
}

void writeStringToMem(char pValue[],int pAdr,int relAdrType){//adrType: 0 glob,1 local var
	int i=0;
	int reg;
	int tmpRegA=getFreeRegister();
	
	if(relAdrType==0){
	  reg=R_HP;
	}else{
	  reg=R_FP;
	}
	
	while(i<40){

		writeInstruction(INSTR_ADDI,  tmpRegA , 0, pValue[i], "write value to register");		
		writeInstruction(INSTR_STB,  tmpRegA , reg, pAdr, "store byte to Mem");
		i=i+1;
		pAdr=pAdr+1;
	}
		releaseRegister(tmpRegA);
}
	
void writeCharToMem(char pValue,int pAdr,int relAdrType){//adrType: 0 glob,1 local var
	
	int i=0;
	int reg;
	int tmpRegA=getFreeRegister();
	
	if(relAdrType==0){
	  reg=R_HP;
	}else{
	  reg=R_FP;
	}
	
	writeInstruction(INSTR_ADDI,  tmpRegA , 0, pValue, "write value to register");		
	writeInstruction(INSTR_STB,  tmpRegA , reg, pAdr, "store byte to Mem");

	releaseRegister(tmpRegA);
}

void writeIntegerToMem(int pValue,int pAdr,int local){//adrType: 0 glob,1 local var
	
	int tmpRegA=getFreeRegister();
	
	writeInstruction(INSTR_ADDI,  tmpRegA , 0, pValue, "write address to register");
	
 	if(local == 0){
  		writeInstruction(INSTR_STW,  tmpRegA , R_HP, pAdr, "write position of data to global area");
 	}else{
 		writeInstruction(INSTR_STW,  tmpRegA , R_FP, pAdr, "write position of data to local area");
 	}
 
  releaseRegister(tmpRegA);
  
}


int getParMemSpace(int funcID){
	int i=0;
	int size=0;
	int akkSize=0;
	struct ObjectDescription objDesc;
	
	while(i<MAX_SYMBOLICNAMES){
		
		if ( (symbolTable[i].funcID==funcID) && (symbolTable[i].class == CLASS_PAR) ){
			
			size = 0;
			
			objDesc=symbolTable[i];
			
			if ( typeTable[objDesc.type].form == TYPE_STRUCT | objDesc.array==1 ){
               	  	size=4;	
			}else{
			   		size=typeTable[objDesc.type].size;
			}			   	
			   	
			 if(size%4 != 0){
			 	size = size+(4-size%4);
			 }
			 akkSize = akkSize+size;	
		}	
		i=i+1;
	}
	return akkSize;
}

void checkType(struct item it1, struct item it2){
	if( (it1.type == it2.type) & (it2.array == it2.array)) {
		return;
	} else {		
		reportError("type mismatch occured", false);
	}
}	

void copyParToParSpace(struct item it){
	int tmpAddress;
	int tmpSize;
	
	if(it.mode==MODE_CONST){
		if(it.type==stringID){
			tmpSize=typeTable[stringID].size;
			tmpAddress = getAddress(tmpSize,3);
			writeStringToMem(it.strValue,tmpAddress,1);
		}else if(it.type==charID){
			tmpSize=typeTable[it.type].size;
			tmpAddress = getAddress(tmpSize,3);
			writeCharToMem(it.strValue[0],tmpAddress,1);
		}else if( it.type == intID ){
			tmpAddress = getAddress(4,3);
			writeIntegerToMem(it.address,tmpAddress,1);
		}else{//should not happen
			reportError("unknown error in copyParToParSpace", true);	
		}	
	}else if(it.mode == MODE_VAR){
		;//TODO
	}else if(it.mode == MODE_REG){
		;//TODO
	}
}
							

void fixMain(){
 	int found=(0-1);
 	int mainAdr=(0-1);
 	found = lookupFunction("main");
 	if(found>=0){
 		mainAdr=funcTable[found].address;
 	}
 	fseek(outFile,0,SEEK_SET);
 	writeInstruction(INSTR_ADDI,1,0, mainAdr, "fix position of main");
}
 	
struct item makeConstItem(int type, int value) {
	struct item it;
	it.mode = MODE_CONST;
	it.type = type;
	it.address = value;
	it.reg = 0;
	it.array = 0;
	return it;
}

struct item makeItem(struct ObjectDescription objDesc) {
	struct item it;
	it.type = objDesc.type;
	it.array = objDesc.array;
	if(objDesc.class == CLASS_CONST) {
		it.mode = MODE_CONST;
		it.address = objDesc.intValue;
	} else if((objDesc.class == CLASS_VAR) | (objDesc.class == CLASS_PAR)) {
		it.mode = MODE_VAR;
		it.address = objDesc.address;
	}
	if(objDesc.funcID == -1) {
		it.reg = R_HP;
		it.adrType = ADR_GLOBAL;
	} else {
		it.reg = R_FP;
		it.adrType = ADR_LOCAL;
	}
	if(objDesc.type > MAIN_TYPE) {
		//TODO: indirect addressing for compound datatypes
	}
	return it;
}

struct item field(struct item it, struct StructDescription strDesc){
	int reg;
	// this works only when structs are not references
	// it.address = it.address + objDesc.address;
	
	// turn reference into register containing address of selected field
	if(it.adrType != ADR_ABSOLUTE) {
		reg = getFreeRegister();
		writeInstruction(INSTR_LDW, reg, it.reg, it.address, "derefernce newly");
		it.adrType = ADR_ABSOLUTE; // changed to absolute addressing
		it.reg = reg;
		it.address = 0;
	} else {
		writeInstruction(INSTR_LDW, it.reg, it.reg, 0, "dereference again");
	}
	writeInstruction(INSTR_ADDI, it.reg, it.reg, strDesc.offset, "add field offset");
	it.type = strDesc.type;
	it.array = strDesc.array;
	return it;
}

struct item arrayIndex(struct item array_it, struct item index_it) {
	int reg;
	int typeSize;
	if(index_it.type != intID) {
		reportError("invalid array index", false);
		return array_it;
	}
	// this works only when array are not references
	// array_it.address = array_it.address + index_it.address * typeTable[array_it.type].size;
	if(array_it.adrType != ADR_ABSOLUTE) {
		reg = getFreeRegister();
		writeInstruction(INSTR_LDW, reg, array_it.reg, array_it.address, "dereference newly");
		array_it.adrType = ADR_ABSOLUTE; // changed to absolute addressing
		array_it.reg = reg;
		array_it.address = 0;
	} else {
		writeInstruction(INSTR_LDW, array_it.reg, array_it.reg, 0, "dereference again");
	}
	if(array_it.type > MAIN_TYPE) {
		typeSize = 4;
	} else {
		typeSize = typeTable[array_it.type].size;
	}
	if(index_it.mode == MODE_CONST) {
		index_it.address = index_it.address * typeSize;
		writeInstruction(INSTR_ADDI, array_it.reg, array_it.reg, index_it.address, "add constant array offset");
	} else {
		writeInstruction(INSTR_MULI, index_it.reg, index_it.reg, typeSize, "multiply by typesize");
		writeInstruction(INSTR_ADD, array_it.reg, array_it.reg, index_it.reg, "add to base address");
	}
	array_it.array = false;
	return array_it;
}

void setupFunctionTable() {
	saveFunction(intID, "main");
}

void setupTypeTable() {
	saveType("void",TYPE_VOID, 0);
	saveType("int",TYPE_INT, 4);
	saveType("bool",TYPE_BOOL, 4);
	saveType("char",TYPE_CHAR, 1);
	saveType("file",TYPE_FILE, 4);
	saveType("string",TYPE_STRING, 40); //string has the fix length 40
	
	strSize=typeTable[lookupType("string")].size;
	
 	stringID=lookupType("string");
	charID=lookupType("char");
	intID=lookupType("int");	
	boolID=lookupType("bool");	
	fileID=lookupType("file");
	voidID=lookupType("void");
}
