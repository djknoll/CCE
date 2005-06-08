#include "Scanner.c"
#include "Codegen.c"

// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- P A R S E R
// ----------------------------------------------------------------------------
struct item simpleExpression();
struct item expression();
void printstatement();
void variabledeclaration(int scope);

void reportError(char msg[], bool final) {
	printf("\n>> ERROR: %s! \nin line(%i), row(%i)\n", msg, currentLine, currentRow);
	occurredErrors = occurredErrors + 1;
	if(final == true) {
		exit(-1);
	}
	if(PARS_DEBUG >= 1) {
		exit(-1);
	}
}

int checkParameter(struct ObjectDescription objDesc,int alreadyDec,int indexOfLastFoundParameter){
	int found = -1;
	int i = indexOfLastFoundParameter;
	int parOK = 0;
	
	while((i<(foundSymbolicNames-1)) & (found==-1 ) ){
		i=i+1;
		if(alreadyDec == symbolTable[i].funcID ) {
			found = i;
			
			if(objDesc.name[0]=='0'){
				parOK = 1;
			} else {
				if(objDesc.type == symbolTable[i].type 	&
			   	  objDesc.array == symbolTable[i].array	&
			   	  compare(objDesc.name,symbolTable[i].name) == 1){
					parOK = 1;
		   	  	}
			}		
		}			
	}
	
	if(parOK==0){
		found = -1;
	}
	return found;
}

int validType() {
	int returnValue = 0;
	int sym;
	if((currentSymbol == SYM_INT) | (currentSymbol == SYM_CHAR) | (currentSymbol == SYM_FILE) | (currentSymbol == SYM_STRING |(currentSymbol == SYM_BOOL))) {
		returnValue = 1;
	} else if (currentSymbol == SYM_STRUCT) {
		nextSymbol();
		if(currentSymbol == SYM_IDENTIFIER) {
			sym = lookupType(stringVal);
			if(sym <= MAIN_TYPE)  {
				reportError("type not defined", true);
			}
		}
		returnValue = 1;
	}
	return returnValue;
}

void semicolon() {
	if(PARS_DEBUG >= 2) {
		printf(">semicolon\n");
	}
	if (currentSymbol == SYM_SEMICOLON) {
		nextSymbol();
	} else {
		reportError("; expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<semicolon\n");
	}
}

void directive() {
	if(PARS_DEBUG >= 2) {
		printf(">directive\n");
	}
	nextSymbol();
	if(currentSymbol == SYM_INCLUDE) {
		nextSymbol();
		if(currentSymbol == SYM_STRING_LITERAL) {
			//?
			nextSymbol();
		} else {
			reportError("stringLiteral expected", false);
		}
	} else {
		reportError("include expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<directive\n");
	}
}

int structType() {
	int found;
	if(PARS_DEBUG >= 2) {
		printf(">structType\n");
	}
	nextSymbol(); // overread 'struct'
	if(currentSymbol == SYM_IDENTIFIER) {
		found = lookupType(stringVal);
		if(found <= MAIN_TYPE ){
			found = -1;
		}
		nextSymbol(); // overread struct-type-name
	} else {
		reportError("identifier expected", true);
	}
	if(PARS_DEBUG >= 2) {
		printf("<structType\n");
	}
	return found;
}

int type() {
	int found;
	if(PARS_DEBUG >= 2) {
		printf(">type\n");
	}
	found = lookupType(stringVal);
	if(PARS_DEBUG >= 2) {
		printf("<type\n");
	}
	return found;
}

struct ObjectDescription variableSignature(int pClass) {
	struct ObjectDescription objDesc;
	struct item nested_it;
	nested_it.address =0;
	
	if(PARS_DEBUG >= 2) {
		printf(">variableSignature\n");
	}
	
	if(currentSymbol == SYM_STRUCT){	
		objDesc.type = structType();
	} else {
   		objDesc.type = type();
	}
	
	if(objDesc.type == -1){
		reportError("Type not found", true);
	}
	
	objDesc.array  = 0;
	objDesc.length = 0;
	objDesc.address=0;
	objDesc.class=pClass;
	objDesc.funcID=0;
	objDesc.intValue=0;
	
	nextSymbol();
	if(currentSymbol == SYM_IDENTIFIER) {
		copy(objDesc.name, stringVal);
		nextSymbol();
	} else {
		reportError("identifier expected", false);
	}

	if(PARS_DEBUG >= 2) {
		printf("<variableSignature\n");
	}
	return objDesc;
}

struct ObjectDescription signature(int pClass) {
	struct ObjectDescription objDesc;
	struct item nested_it;
	objDesc.intValue=0;
	objDesc.array=0;
	objDesc.funcID=-1;
	objDesc.length=0;
	objDesc.address = 0;


	objDesc = variableSignature(pClass);
	if(currentSymbol == SYM_LEFTBRACKET) {
		nextSymbol();
		if(pClass==CLASS_PAR){
			if(currentSymbol == SYM_RIGHTBRACKET){
				objDesc.array=1;
				nextSymbol();
			}else{
				reportError("] expected", false);
			}
		}else{						
			nested_it = simpleExpression();
			if( (nested_it.mode != MODE_CONST) | (nested_it.address < 1)) {
				reportError("wrong array index", false);
			}
			if(currentSymbol == SYM_RIGHTBRACKET) {
				objDesc.array=1;
				objDesc.length=nested_it.address;
				nextSymbol();
			} else {
				reportError("] expected", false);
			}
		}
	}
	
	if(PARS_DEBUG >= 2) {
		printf("<signature\n");
	}
	return objDesc;
}

struct item argument() {
	struct item it;
	if(PARS_DEBUG >= 2) {
		printf(">argument\n");
	}
	
	if (currentSymbol == SYM_STRING_LITERAL) {   	    
	    copy(it.strValue,stringVal);
	    it.type = lookupType("string");
	    it.array = 0;
	    it.mode = MODE_CONST;
	    nextSymbol();
	} else if(currentSymbol == SYM_CHAR_LITERAL) {		
		copy(it.strValue, stringVal);
		it.type = charID;
        it.array = 0;
		it.mode = MODE_CONST;
		nextSymbol();
	} else {
		  it = expression();
	}
	
	if(PARS_DEBUG >= 2) {
		printf("<argument\n");
	}
	return it;
}

void constdeclaration() {
	struct ObjectDescription objDesc;
	struct item it;
	int help;
	if(PARS_DEBUG >= 2) {
		printf(">constdeclaration\n");
	}
	
	objDesc.intValue=0;
	objDesc.array=0;
	objDesc.funcID=-1;
	objDesc.length=0;
	objDesc.address=0;
	
	objDesc.class = CLASS_CONST;
	
	nextSymbol();
	
	if(currentSymbol == SYM_IDENTIFIER) {
		objDesc.type = lookupType(stringVal);
		if (objDesc.type < 0) {
			reportError("unknown type in const declaration", false);
		}
		nextSymbol();
		
		if(currentSymbol == SYM_IDENTIFIER) {
			help=lookupSymbolicName(stringVal,-1,0,0);
			//printf("\nfound: %s. at %i",objDesc.name,help);
			if (help < 0) {
			   copy(objDesc.name, stringVal);
			} else {
			   	reportError("Constant allready declarated", false);
			}
		} else {
			reportError("identifier expected", false);
	    }
		
		nextSymbol();
		
		if (currentSymbol == SYM_BECOMES) {
			nextSymbol();
			it = argument();
			
			objDesc.intValue=it.address;
			objDesc.array=0;
		    objDesc.funcID=(foundFunctions-1);
		    objDesc.length=0;
		    objDesc.address = 0;
		    saveObjectDescription(objDesc);
			semicolon();
			
		} else {
			reportError("= expected, const declaration", false);
		}
	} else {
		reportError("identifier expected", false);
	}
	
	if(PARS_DEBUG >= 2) {
		printf("<constdeclaration\n");
	}
	
}

void structdeclaration() {
	struct ObjectDescription objDesc;
	int size  = 0;
	int found = 0;
	int akk_size=0;
	int tmpAddress=0;
	int tmpSize=0;

    if(PARS_DEBUG >= 2) {
		printf(">structdeclaration\n");
	}
	found = structType();
	
	if (found > MAIN_TYPE) {//struct variable
		
		if(typeTable[found].form!=TYPE_STRUCT){
			reportError("varable-definition error:", false);
		}
		
		if (currentSymbol != SYM_IDENTIFIER) {
			reportError("Identifier expected:", true);
		}
		
		copy(objDesc.name,stringVal);
		objDesc.class=CLASS_VAR;
		objDesc.type=found;
		objDesc.array=0;
		objDesc.funcID= -1;
		objDesc.length=0;
		objDesc.intValue=0;
		objDesc.address=0;
		
		nextSymbol();
		
		if(currentSymbol == SYM_LEFTBRACKET){
			nextSymbol();
			if(currentSymbol == SYM_INT_LITERAL){
			 	if (numberVal > 0x7FFFFFFF) {
					reportError("integer literal too large", true);
				}
				objDesc.length=numberVal;
				objDesc.array=1;
				nextSymbol();
			} else {
				reportError("constant value expected", true);				 				   
			}
			if(currentSymbol == SYM_RIGHTBRACKET){
				nextSymbol();
			} else {
				reportError("rightbracket expected", true);
			}
		}
 		semicolon();
				
		tmpSize = typeTable[objDesc.type].size;
	
		if( (objDesc.type > MAIN_TYPE) | (objDesc.array==1)){ //struct,array	    
			if( objDesc.array ==1 ){
		   		tmpSize = tmpSize*objDesc.length;
		   	 	}			    
		   	objDesc.address=getAddress(4,0); 				   											 				 								                    
		}
		saveObjectDescription(objDesc);

	} else if (found < 0 ) {
		if (currentSymbol == SYM_LEFTCURLYBRACES) {
	   		saveType(stringVal, TYPE_STRUCT, 4); 
			nextSymbol();
			
			if (currentSymbol == SYM_RIGHTCURLYBRACES)  {
				reportError("empty struct not allowed", false);
			}
		
			while ( currentSymbol != SYM_RIGHTCURLYBRACES ) {
			    
			    if (foundStructFields==MAX_STRUCTFIELDS){
			    	reportError("maximum of structfields reached", false);
			    }
			    
				objDesc = signature(CLASS_VAR);
			    found = lookupStructFieldName(objDesc.name , foundTypeDeclarations - 1);
			    
			    if (found < 0) {
			    	
			    	if ( typeTable[objDesc.type].form == TYPE_STRUCT ) { //nested structs
	            	  	size = 4;	
			    	} else {
			    		size = typeTable[objDesc.type].size;
			    	}
			    	if (objDesc.array == 1) {
			    		size = size * objDesc.length;
			    	}
			    	if (size%4 != 0) {
			    		size = size + (4-size%4);
			    	}
			    	
			    	structTable[foundStructFields].offset = akk_size;
			    				    	
			    	akk_size = akk_size + size;
			    		    
		   			copy(structTable[foundStructFields].name, objDesc.name);
		    		structTable[foundStructFields].type = objDesc.type;
		    		structTable[foundStructFields].array = objDesc.array;
		    		structTable[foundStructFields].length = objDesc.length;
					structTable[foundStructFields].structID = foundTypeDeclarations-1;
			
					foundStructFields = foundStructFields + 1;
					semicolon();
				} else {
					reportError("StructName already used", false);	
				}
			}
		nextSymbol();
		semicolon();
	} else {
		reportError("{ expected", false);
	}
	
	typeTable[foundTypeDeclarations-1].size=akk_size;
	} else {
		reportError("idenifier expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<structdeclaration\n");
	}
}

void parameterlist(int alreadyDec) {
	int help = 0;
	int tmpSize=0;
	int tmpAddress = 0;
	int indexOfLastFoundParameter=-1; //parameters must have the same order on symbolTable
	struct ObjectDescription objDesc;
	
	
	objDesc.intValue=0;
	objDesc.array=0;
	objDesc.funcID=-1;
	objDesc.length=0;
	objDesc.address=0;
	
	
	if(PARS_DEBUG >= 2) {
		printf(">parameterlist\n");
	}
	
	objDesc = signature(CLASS_PAR);
	objDesc.class = CLASS_PAR;
	objDesc.funcID = foundFunctions-1;	
	tmpSize = typeTable[objDesc.type].size;
	
	if( (objDesc.type > MAIN_TYPE) | (objDesc.array==1)){ // struct,array	    		    
	   	objDesc.address=getAddress(4,2); 			    				   											 				 								                    
	} else {
		objDesc.address = getAddress(tmpSize,2);
	}		
	
	
    if(alreadyDec<0) {
		saveObjectDescription(objDesc);
    } else {
    	indexOfLastFoundParameter=checkParameter(objDesc,alreadyDec,indexOfLastFoundParameter);
    	if(indexOfLastFoundParameter<0){
    		reportError("function already known with other parameters !", true);
    	}
    }
	
	while (currentSymbol == SYM_COMMA) {
		nextSymbol();
		if (validType() == 1) {
			
			objDesc = signature(CLASS_PAR);
			objDesc.class = CLASS_PAR;
			objDesc.funcID = foundFunctions-1;
			objDesc.address=getAddress(typeTable[objDesc.type].size,2);
		
			if(alreadyDec<0) {
				saveObjectDescription(objDesc);
  			} else {
    			indexOfLastFoundParameter=checkParameter(objDesc,alreadyDec,indexOfLastFoundParameter);
    			if(indexOfLastFoundParameter<0){
    				reportError("function already known with other parameters !", true);
    			}
 			}
				
		} else {
			reportError("type expected", false);
		}	
	}
	
	if(alreadyDec > -1 ){
    	objDesc.name[0]='0';
    	indexOfLastFoundParameter=checkParameter(objDesc,alreadyDec,indexOfLastFoundParameter);
 		if(indexOfLastFoundParameter>-1){ //ForwardDeclaration has more parameters
    		reportError("function already declarated with more parameters!", false);
    	}
 	}
 			
	if(PARS_DEBUG >= 2) {
		printf("<parameterlist\n");
	}
}

void argumentlist(int calledFunction) {
	struct item it;
	int lastFoundIndex=-1;
	int typeOK=0;
	int tmpAddress;
	int tmpSize=0;
	char zero[1];
	zero[0]='0';
	
	if(PARS_DEBUG >= 2) {
		printf(">argumentlist\n");
	}

	it=argument();
	lastFoundIndex=lookupSymbolicName(zero,calledFunction,CLASS_PAR,lastFoundIndex+1);

	if(lastFoundIndex<0){
		reportError("too many parameters!", false);
	} else {
		checkType(it, makeItem(symbolTable[lastFoundIndex]));
		//copy value or reference into parameterSpace
		copyParToParSpace(it);	
	}
	
	while (currentSymbol == SYM_COMMA) {
		nextSymbol();
		it = argument();
		lastFoundIndex=lookupSymbolicName("0",calledFunction,CLASS_PAR,lastFoundIndex+1);
		
		if(lastFoundIndex<0){
			reportError("too many parameters", false);
		} else {
			checkType(it, makeItem(symbolTable[lastFoundIndex]));
			//copy value or reference into parameterSpace
			copyParToParSpace(it);
		}
	}
	
	lastFoundIndex=lookupSymbolicName("0",calledFunction,CLASS_PAR,lastFoundIndex+1);
	if(lastFoundIndex>=0){
		reportError("too few parameters in argumentlist", false);
	}

	if(PARS_DEBUG >= 2) {
		printf("<argumentlist\n");
	}
}

void functioncall(char name[]) {
	int calledFunction = 0;
	int tmpAddress;
	int found=-1;
	int parameterMemSpace;
	int funcAdr = 0;
	if(PARS_DEBUG >= 2) {
		printf(">functioncall\n");
	}

	calledFunction=lookupFunction(name);
	printf("\n\n\n\tindex of calledFunction in table: %d\n\n\n", calledFunction);
	if(calledFunction < 0){
		reportError("unknown function", false);
	}
	parameterMemSpace=funcTable[calledFunction].parameterMemSpace;
    funcAdr = funcTable[calledFunction].address;
	//initialize currentCallParameterAddress
	callParameterAddress=currentDataAddress-parameterMemSpace;
	
	nextSymbol(); // overread (
	if (currentSymbol == SYM_RIGHTPARENTHESES) {
		found = lookupSymbolicName("0",calledFunction,CLASS_PAR,0);
		if (found>=0){
			reportError("delclarated function has parameters", false);
		}
		nextSymbol();
	} else {
		argumentlist(calledFunction);
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
		} else {
			reportError(") expected, factor, end functioncall", false);
		}
	}
	
	tmpAddress = currentDataAddress-parameterMemSpace-4;//get address for FP
	writeInstruction(INSTR_STW,R_FP,R_FP,tmpAddress, "save FP to memory");
	tmpAddress = tmpAddress-4;
	writeInstruction(INSTR_STW,31,R_FP,tmpAddress, "save R31 to memory");
	writeInstruction(INSTR_ADDI,R_FP,R_FP,tmpAddress, "set FP to new address");
	
	//TODO if address not known -> fix later
	printf("\n\n\n\tadress of function: %d\n\n\n", funcAdr);
	writeInstruction(INSTR_JSR,0,0,funcAdr, "jump to called function");	
	
	writeInstruction(INSTR_LDW,31,R_FP,0, "load R31 from stack");
	writeInstruction(INSTR_LDW,R_FP,R_FP,4, "load FP from stack");
			
	if(PARS_DEBUG >= 2) {
		printf("<functioncall\n");
	}
}

struct item selector(struct item it) {
	struct item nested_it;
	struct StructDescription strDesc;
	int foundOK;
	while((currentSymbol == SYM_LEFTBRACKET) | (currentSymbol == SYM_DOT)) {
		if(currentSymbol == SYM_LEFTBRACKET) { // handling array accessor
			nextSymbol(); // overreading [
			nested_it = expression(); // parsing the indexing expression
			if(it.array == true) {
				it = arrayIndex(it, nested_it);
				releaseRegister(nested_it.reg);
			} else {
				reportError("not an array",true);
			}
			if(currentSymbol == SYM_RIGHTBRACKET) {
				nextSymbol(); // overreading ]
			} else {
				reportError("] expected", false);
			}
		} else { // handling field accessor
			nextSymbol(); // overreading .
			if(currentSymbol == SYM_IDENTIFIER) {
				if(typeTable[it.type].form == TYPE_STRUCT) {
					foundOK = lookupStructFieldName(stringVal, it.type);
					if(foundOK == -1) {
						reportError("undefined stuct field name occurred", false);
					}
					strDesc = structTable[foundOK];
					it = field(it, strDesc);
				} else {
					reportError("not a record", false);
				}
				nextSymbol(); // overreading field identifier
			} else {
				reportError("identifier expected", false);
			}
		}// end if [ or .
	} // end while
} // end selector

struct item factor() {
	struct item it;
	struct ObjectDescription objDesc;
	int foundOK=-1;

	if(PARS_DEBUG >= 2) {
		printf(">factor\n");
	}
	if (currentSymbol == SYM_INT_LITERAL) {
		it = makeConstItem(intID, numberVal);
		nextSymbol(); // overread integer literal
	} else if (currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol(); // overread (
		it = expression();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol(); // overread )
		} else {
			reportError(") expected, end factor", false);
		}
	} else if (currentSymbol == SYM_IDENTIFIER) {
		foundOK = lookupSymbolicName(stringVal,actualFunction,0,0);
		if (foundOK == -1){
			foundOK = lookupSymbolicName(stringVal,-1,0,0);//look for global var's
			if (foundOK == -1){
				reportError("Unknown identifier", true);
			}
		}
		nextSymbol(); // overread identifier
		objDesc = symbolTable[foundOK];
		it = makeItem(objDesc);
		it = selector(it);
	} else if(currentSymbol == SYM_NOT) {
		nextSymbol();
		factor();
	} else {
		reportError("number, variable, or function expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<factor\n");
	}
	
	return it;
	
}

struct item term() {
	struct item it1;
	struct item it2;
	int operation;
	if(PARS_DEBUG >= 2) {
		printf(">term\n");
	}
	it1 = factor();
	while ((currentSymbol == SYM_TIMES) | (currentSymbol == SYM_DIV) | (operation == SYM_MOD) | (operation == SYM_AND)) {
		operation = currentSymbol;
		nextSymbol();
		it2 = factor();
		if(operation == SYM_TIMES) { // MULTIPLICATION
			if((it1.mode == MODE_REG) & (it2.mode == MODE_REG)) {
				writeInstruction(INSTR_MUL, it1.reg, it1.reg, it2.reg, "multiply registers");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_CONST) & (it2.mode == MODE_REG)) {
				it1.reg = getFreeRegister();
				it1.mode = MODE_REG;
				writeInstruction(INSTR_MULI, it1.reg, it2.reg, it1.address, "multiply register with constant");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_REG) & (it2.mode == MODE_CONST)) {
				writeInstruction(INSTR_MULI, it1.reg, it1.reg, it2.address, "multiply register with constant");
			} else { // two constants
				it1.address = it1.address * it2.address;
			}
		} else if(operation == SYM_DIV) { // DIVISION
			if((it1.mode == MODE_REG) & (it2.mode == MODE_REG)) {
				writeInstruction(INSTR_DIV, it1.reg, it1.reg, it2.reg, "divide registers");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_CONST) & (it2.mode == MODE_REG)) {
				it1.reg = getFreeRegister();
				it1.mode = MODE_REG;
				writeInstruction(INSTR_DIVI, it1.reg, it2.reg, it1.address, "devide register by constant");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_REG) & (it2.mode == MODE_CONST)) {
				writeInstruction(INSTR_DIVI, it1.reg, it1.reg, it2.address, "devide register by constant");
			} else { // two constants
				it1.address = it1.address / it2.address;
			}
		} else if(operation == SYM_MOD) { // MODULO
			//CODEGEN
		} else if(operation == SYM_AND) { // 
			//CODEGEN
		}
	}
	if(PARS_DEBUG >= 2) {
		printf("<term\n");
	}
	return it1;
}

struct item simpleExpression() {
	struct item it1;
	struct item it2;
	int operation;
	if(PARS_DEBUG >= 2) {
		printf(">simpleExpression\n");
	}
	if(currentSymbol==SYM_MINUS) {
		nextSymbol();
		//CODEGEN
	}
	it1 = term();
	while ((currentSymbol == SYM_PLUS) | (currentSymbol == SYM_MINUS) | (currentSymbol == SYM_OR)) {
		operation = currentSymbol;
		nextSymbol();
		it2 = term();
		if(operation == SYM_PLUS) { // ADDITION
			if((it1.mode == MODE_REG) & (it2.mode == MODE_REG)) {
				writeInstruction(INSTR_ADD, it1.reg, it1.reg, it2.reg, "add registers");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_CONST) & (it2.mode == MODE_REG)) {
				it1.reg = getFreeRegister();
				it1.mode = MODE_REG;
				writeInstruction(INSTR_ADDI, it1.reg, it2.reg, it1.address, "add constant to register");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_REG) & (it2.mode == MODE_CONST)) {
				writeInstruction(INSTR_ADDI, it1.reg, it1.reg, it2.address, "add constant to register");
			} else { // two constants
				it1.address = it1.address + it2.address;
			}
		} else if(operation == SYM_MINUS) { // SUBTRACTION
			if((it1.mode == MODE_REG) & (it2.mode == MODE_REG)) {
				writeInstruction(INSTR_SUB, it1.reg, it1.reg, it2.reg, "substract registers");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_CONST) & (it2.mode == MODE_REG)) {
				it1.reg = getFreeRegister();
				it1.mode = MODE_REG;
				writeInstruction(INSTR_SUBI, it1.reg, it2.reg, it1.address, "substract constant from register");
				releaseRegister(it2.reg);
			} else if((it1.mode == MODE_REG) & (it2.mode == MODE_CONST)) {
				writeInstruction(INSTR_SUBI, it1.reg, it1.reg, it2.address, "substract constant from register");
			} else { // two constants
				it1.address = it1.address - it2.address;
			}
		} else if(operation == SYM_OR) {
			//CODEGEN
		}
	}
	if(PARS_DEBUG >= 2) {
		printf("<simpleExpression\n");
	}
	return it1;
}

struct item expression() {
	struct item it1;
	struct item it2;
	if(PARS_DEBUG >= 2) {
		printf(">expression\n");
	}
	it1 = simpleExpression();
	if((currentSymbol == SYM_EQUAL) | (currentSymbol == SYM_NOTEQUAL) | (currentSymbol == SYM_LESS)| (currentSymbol == SYM_LESSEQUAL) | (currentSymbol == SYM_GREATEREQUAL) | (currentSymbol == SYM_GREATER)) {
		nextSymbol();
		// code gen
		it2 = simpleExpression();
	}
	if(PARS_DEBUG >= 2) {
		printf("<expression\n");
	}
	return it1;
}

void whilestatement() {
	if(PARS_DEBUG >= 2) {
		printf(">whilestatement\n");
	}
	nextSymbol();
	if(currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol();
		expression();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
			block();
		} else {
			reportError(") expected", false);
		}
	} else {
		reportError("( expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<whilestatement\n");
	}
}
void ifstatement() {
	if(PARS_DEBUG >= 2) {
		printf(">ifstatement\n");
	}
	nextSymbol();
	if (currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol();
		expression();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
			block();
			if (currentSymbol == SYM_ELSE) {
				nextSymbol();
				if (currentSymbol == SYM_IF) {
					ifstatement();
				} else {
					block();
				}
			}
		} else {
			reportError(") expected, if statement", false);
		}
	} else {
		reportError("( expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<ifstatement\n");
	}
}

void returnstatement() {
	if(PARS_DEBUG >= 2) {
		printf(">returnstatement\n");
	}
	nextSymbol();
	if(currentSymbol != SYM_SEMICOLON) {
		argument();
		writeInstruction(INSTR_RET, 0, 0, 31, "return to address zero");
	}
	semicolon();
	if(PARS_DEBUG >= 2) {
		printf("<returnstatement\n");
	}
}

void assignment(char varName[]) {
	struct item left_it;
	struct item right_it;
	int foundOK;
	if(PARS_DEBUG >= 2) {
		printf(">assignment\n");
	}
	foundOK = lookupSymbolicName(varName, actualFunction, 0, 0);
	if(foundOK == -1) {
		reportError("unknown identifier", false);
		return;
	}
	left_it = makeItem(symbolTable[foundOK]);

	left_it = selector(left_it);

	if (currentSymbol == SYM_BECOMES) {
		nextSymbol();
		right_it = argument();
		checkType(left_it, right_it);
		right_it = loadIntoRegister(right_it);
		
		if(left_it.adrType == ADR_LOCAL) {
			writeInstruction(INSTR_STW, right_it.reg, R_FP, left_it.address, "store value into local addressed memory");
		} else if (left_it.adrType == ADR_GLOBAL) {
			writeInstruction(INSTR_STW, right_it.reg, left_it.reg, R_HP, "store value into global addressed memory");
		} else {
			writeInstruction(INSTR_STW, right_it.reg, left_it.reg, 0, "store value into absolute addressed memory");
		}
		releaseRegister(right_it.reg);
	} else {
		reportError("= expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<assignment\n");
	}
}

void statement(){
	int calledFunction=-1;
	char id[40];
	if(PARS_DEBUG >= 2) {
		printf("|>statement\n");
	}
	if (currentSymbol == SYM_IDENTIFIER) {
		copy(id, stringVal);
		nextSymbol();
		if ((currentSymbol == SYM_LEFTBRACKET) | (currentSymbol == SYM_DOT) | (currentSymbol == SYM_BECOMES)) {
			assignment(id);
		} else if (currentSymbol == SYM_LEFTPARENTHESES) {
			functioncall(id);
		} else {
			reportError("assignment or function call expected", false);
		}
		semicolon();
	} else if (currentSymbol == SYM_IF) {
		ifstatement();
	} else if (currentSymbol == SYM_WHILE) {
		whilestatement();
	} else if (currentSymbol == SYM_RETURN) {
		returnstatement();
	} else if (currentSymbol == SYM_PRINT){
		printstatement();
	} else {
		reportError("statement expected", true);
		nextSymbol();
	}
	if(PARS_DEBUG >= 2) {
		printf("|<statement\n");
	}
}


void block(funcID) {
	if(PARS_DEBUG >= 2) {
		printf(">statement\n");
	}
	nextSymbol();
	while(currentSymbol != SYM_RIGHTCURLYBRACES) {
		statement(funcID);
	}
	nextSymbol();
	if(PARS_DEBUG >= 2) {
		printf("<statement\n");
	}
}


void functiondeclaration() {
	struct ObjectDescription sigDesc;
	struct ObjectDescription objDesc;
	int tmpSize=0;
	int tmpAddress=0;
	int alreadyDec;
	int help=-1;
		
	if(PARS_DEBUG >= 2) {
		printf(">functiondeclaration\n");
	}
	
	nextSymbol(); // overread func
	sigDesc = signature(CLASS_VAR);

	alreadyDec = lookupFunction(sigDesc.name);			
	if(alreadyDec == -1) {
		saveFunction(sigDesc.type, sigDesc.name);
		actualFunction = foundFunctions-1;
	} else { 
		actualFunction = alreadyDec;
		if(funcTable[alreadyDec].implemented==1){
			reportError("function already implemented!", true);		
		}
	}	  		

	if(currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol(); //overread (
	} else {
		reportError("( expected", false);
	}
	if(validType() == 1) {
		parameterlist(alreadyDec);
	}
	if(currentSymbol == SYM_RIGHTPARENTHESES & alreadyDec > -1 ) { //check if forwarddec has also no parameters
		objDesc.name[0]='0';
		help=checkParameter(objDesc,alreadyDec,-1);
		if(help > -1){
			reportError("function already known with different parameters!", true);		
		}
	}
	
	funcTable[actualFunction].parameterMemSpace=getParMemSpace(actualFunction); //calculate mem for parameters
	
	if(currentSymbol == SYM_RIGHTPARENTHESES) {
		nextSymbol(); // overread )
	} else {
		reportError(") expected", false);
	}
		
	if (currentSymbol == SYM_SEMICOLON) { // == forward declaration
		actualFunction = -1;
		nextSymbol();
	} else if (currentSymbol == SYM_LEFTCURLYBRACES) {
		nextSymbol(); // overread {

		//first occurrence of an implemented function: -> programm continues at main
		if(foundImplementedFunction==0){
			foundImplementedFunction = 1;	
			writeInstruction(INSTR_RET,0,0,1, "jump to main");	
		    releaseRegister(1);		
		}

		funcTable[actualFunction].address=currentInstruction;
		funcTable[actualFunction].implemented=1;				

		while (validType() == true) {
			variabledeclaration(actualFunction);
		}
		while(currentSymbol != SYM_RIGHTCURLYBRACES ) {
			statement();
		}
		//TODO look for returnStatement
		//	    put returnValue on stack
		    
		resetLocalAddress();
		actualFunction = -1;
		
		writeInstruction(INSTR_RET,0,0,31, "return to r31");		
		
		nextSymbol(); // overread }
	} else {
		reportError("{ expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<functiondeclaration\n");
	}
}

void variabledeclaration(int scope) {
	struct ObjectDescription objDesc;
	struct item it;
	int tmpAddress=0;
	int tmpSize=0;
	if(PARS_DEBUG >= 2) {
		printf(">variabledeclaration\n");
	}	
	objDesc.intValue = 0;
	objDesc.array = 0;
	objDesc.funcID = scope;
	objDesc.length = 0;
	objDesc.address = 0;
		
	objDesc = signature(CLASS_VAR);
	if(currentSymbol == SYM_SEMICOLON | currentSymbol == SYM_BECOMES) {				
		objDesc.funcID = actualFunction;
		tmpSize = typeTable[objDesc.type].size;
	
		if( (objDesc.type > MAIN_TYPE) | (objDesc.array == 1)){ //struct,array	    		    
	    	objDesc.address = getAddress(4, 0); 			    				   											 				 								                    
		} else {
			objDesc.address = getAddress(tmpSize,0);
		}		
		if (currentSymbol == SYM_BECOMES) {
			nextSymbol();				
			it = argument();
		
			if ( (it.mode==MODE_CONST) & (objDesc.array == 0) ){				
				if(it.type==stringID) {
					if(objDesc.type == it.type) {
						writeStringToMem(it.strValue,objDesc.address,0);
					} else {
						reportError("type mismatch in assignment", true);
					}
				} else if(it.type==charID) {
		    		if(objDesc.type==it.type){
		    	     	writeCharToMem(it.strValue[0],objDesc.address,0);
					} else {
						reportError("type mismatch in assignment", true);
		    		}	
		    	} else { //integer
			    	if(it.type==intID) {
			    		writeIntegerToMem(it.address,objDesc.address,0);
					} else {
						reportError("type mismatch in assignment", true);
			    	}					    	
		   		}				   			
			} else {
				reportError("assignment not possible", true);
			}
			semicolon();
		} else {
			nextSymbol(); //semicolon
		}
		saveObjectDescription(objDesc);
	} else {
		reportError("; or = or ( expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<variabledeclaration\n");
	}	
}

void printstatement(){
	nextSymbol();
	struct item it;
	int tmpAdr;
	int tmpReg;

	if(PARS_DEBUG >= 2) {
		printf(">printstatement\n");
	}
	if (currentSymbol==SYM_LEFTPARENTHESES){
		nextSymbol();
		it=argument();
		if(it.mode == MODE_CONST){
			if( (it.type == stringID) | (it.type == charID) ){
				tmpAdr=getAddress(typeTable[stringID].size,1);
				writeStringToMem(it.strValue,tmpAdr,1);
				writeInstruction(INSTR_PSTR, R_FP, 0, tmpAdr, "print string");		
			} else {
				tmpReg=getFreeRegister();
				writeInstruction(INSTR_ADDI, tmpReg, 0, it.address, "");	
				writeInstruction(INSTR_PINT, tmpReg, 0, 0 , "print integer");
				releaseRegister(tmpReg);
			}
		} else if(it.mode == MODE_VAR) {
			it = loadIntoRegister(it);
		}
		if(it.mode == MODE_REG) {
			if( (it.type == stringID) | (it.type == charID) ){
				writeInstruction(INSTR_PSTR, it.reg, 0, 0, "print string");		
			} else {
				writeInstruction(INSTR_PINT, it.reg, 0, 0, "print integer");
			}
		    releaseRegister(it.reg);
		}
		
		if(currentSymbol!=SYM_RIGHTPARENTHESES){
	        nextSymbol();
	        reportError(") expected!", true);
        }
    
        nextSymbol();
	
		semicolon();
		
	}else{
		nextSymbol();
		reportError("( expected!", true);
	}
	
	if(PARS_DEBUG >= 2) {
		printf("<printstatement\n");
	}	

}

void program() {
	struct ObjectDescription objDesc;
	struct item it;
	
	writeInstruction(INSTR_ADDI, 1, 0, 0, "dummy instruction: PC of main");
    registers[1]=1; 
	
	
	if(PARS_DEBUG >= 1) {
		printf("--reading includes\n");
	}
	while(currentSymbol == SYM_HASH) {
		directive();
	}
		
	if(PARS_DEBUG >= 1) {
		printf("--struct declarations\n");
	}
	while(currentSymbol == SYM_STRUCT) {
		structdeclaration();
	}
	
	if(PARS_DEBUG >= 1) {
		printf("--reading constant declarations\n");
	}
    while(currentSymbol == SYM_CONST) {
		constdeclaration();
	}
 
	if(PARS_DEBUG >= 1) {
		printf("--reading variable declarations\n");
	}
	while(validType() == true) {
		variabledeclaration(-1);	
	}
	
	if(PARS_DEBUG >= 1) {
		printf("--reading variable and function declarations\n");
	}
	while(currentSymbol == SYM_FUNC) {
		functiondeclaration();
	}

	if (currentSymbol != SYM_EOF) {
		reportError("Unknown type encountered!", false);
	}

	fixMain();
}


	
void parse() {
	setupKeyWordTable();
	setupTypeTable();
	setupFunctionTable();
	
	nextChar();
	nextSymbol();
	
	program();

	showTypeTable();
	showStructTable();
	showFuncTable();
	showSymbolTable();

	printf("\n\n\tParsing completed!");
	printf("\n\tSymbolic names found: %i", foundSymbolicNames);
	printf("\n\tTypeDeclarations found: %i", foundTypeDeclarations);
	printf("\n\tFunctions found: %i", foundFunctions);
	printf("\n\tStructFields found: %i", foundStructFields);
	printf("\n\tUnrecognized symbols: %i", unrecognizedSymbols);
	printf("\n\tErrors occured: %i\n", occurredErrors);
	return;
}


