#include "Scanner.c"
#include "codegen.c"


// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- P A R S E R
// ----------------------------------------------------------------------------





void reportError(string msg, bool final) {
	printf("\n>> ERROR: %s! \nin line(%i), row(%i)\n", msg, currentLine, currentRow);
	occurredErrors = occurredErrors + 1;
	if(final == true) {
		exit(0-1);
	}
	if(PARS_DEBUG >= 1) {
		exit(0-1);
	}
}

int checkParameter(struct ObjectDescription objDesc,int alreadyDec,int indexOfLastFoundParameter){
	int found = (0-1);
	int i = indexOfLastFoundParameter;
	while((i<(MAX_SYMBOLICNAMES-1)) & (found==(0-1) ) ){
		
		i=i+1;
		
		if(alreadyDec    == symbolTable[i].funcID   &
		   objDesc.type  == symbolTable[i].type 	&
		   objDesc.array == symbolTable[i].array	&
		   compare(objDesc.name,symbolTable[i].name) == 1){
		   	
		   	found = i;
		   }
		   
	}
	return found;
}



int validType() {
	int returnValue = 0;
	int sym;
	if((currentSymbol == SYM_INT) | (currentSymbol == SYM_CHAR) | (currentSymbol == SYM_FILE) | (currentSymbol == SYM_STRING)) {
		returnValue = 1;
	} else if (currentSymbol == SYM_STRUCT) {
		nextSymbol();
		if(currentSymbol == SYM_IDENTIFIER) {
			sym = lookupType(stringVal);
			if((sym < 6) & (sym > 0) ) {
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

struct ObjectDescription signature(int pClass) {
	struct ObjectDescription objDesc;
	struct item nested_it;
	nested_it.address =0;
	if(PARS_DEBUG >= 2) {
		printf(">signature\n");
	}
	objDesc.type   = lookupType(stringVal);
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
		if(currentSymbol == SYM_LEFTBRACKET) {
			nextSymbol();
			if(currentSymbol != SYM_RIGHTBRACKET) {
				if(pClass == CLASS_PAR){
					reportError("Array Index in Parameterlist not allowed", false);
				}
				nested_it = arithmeticExpression();
				if( (nested_it.mode != MODE_CONST) | (nested_it.address < 1)) {
					reportError("wrong array index", false);
				}
			}
			if(currentSymbol == SYM_RIGHTBRACKET) {
				objDesc.array=1;
				objDesc.length=nested_it.address;
				nextSymbol();
			} else {
				reportError("] expected", false);
			}
		}
	} else {
		reportError("identifier expected", false);
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
	if (currentSymbol == SYM_STRING_LITERAL)  {   
	    
	    copy(it.strValue,stringVal);
	    it.type = lookupType("string");
	    it.mode = MODE_CONST;
	    nextSymbol();
	    
	}else if(currentSymbol == SYM_CHAR_LITERAL){
		
		copy(it.strValue,stringVal);
		it.type = lookupType("char");
		it.mode = MODE_CONST;
	    nextSymbol();
	
	} else {
		it = arithmeticExpression();
		
	  /*if (constDec!=1){
			it = loadIntoRegister(it);
		}*/
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
	objDesc.class = CLASS_CONST;
	
	nextSymbol();
	
	objDesc.type = lookupType(stringVal);
	
	if (objDesc.type < 0){
		reportError("unknown type in  const declaration", false);
	}
	
	nextSymbol();
	
	
	if(currentSymbol == SYM_IDENTIFIER) {
		
		help=lookupSymbolicName(stringVal,0-1);
		
		//printf("\nfound: %s. at %i",objDesc.name,help);
		
		if(help<0){
		   copy(objDesc.name, stringVal);
		   }
		else{
		   	reportError("Constant allready declarated", false);
		   }
	} else{
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

	
	if(PARS_DEBUG >= 2) {
		printf("<constdeclaration\n");
	}
	
}

void structdeclaration() {
	
	struct ObjectDescription objDesc;
	int size  = 0;
	int foundOK = 0-1;
	int akk_size=0;

   
     if(PARS_DEBUG >= 2) {
		printf(">structdeclaration\n");
	}
    

    	
	nextSymbol();
	
	if (currentSymbol == SYM_IDENTIFIER) {
		
		foundOK = 0-1;
		foundOK = lookupType(stringVal);
		
		if (foundOK < 0 ){
	   		saveType(stringVal, TYPE_STRUCT, 4); 
		}else{
			reportError("struct-definition error:", false);
		}
		
		nextSymbol();
		
		if (currentSymbol == SYM_LEFTCURLYBRACES) {
			
			nextSymbol();
			
			while( currentSymbol != SYM_RIGHTCURLYBRACES ) {
			    
			    if (foundStructFields==MAX_STRUCTFIELDS){
			    	reportError("maximum of structfields reached", false);
			    }
			    
				objDesc = signature(CLASS_VAR);
                
                if ( typeTable[objDesc.type].form == TYPE_STRUCT ){
                	reportError("structs in structdefinition not allowed", false);	
                }
                
			    foundOK = 0-1;
			    foundOK = lookupStructName(objDesc.name , foundTypeDeclarations - 1);
			    
			    if(foundOK<0){
			    	
			    	size=typeTable[objDesc.type].size;
			    	
			    	if(objDesc.array==1){
			    		size = size * objDesc.length;
			    	}
			    	
			    	if(size%4 != 0){
			    		size = size+(4-size%4);
			    	}
			    	
			    	structTable[foundStructFields].offset = akk_size;
			    				    	
			    	akk_size = akk_size + size;
			    		    
		   			copy(structTable[foundStructFields].name, objDesc.name);
		    		structTable[foundStructFields].fieldID = objDesc.type;
		    		structTable[foundStructFields].array = objDesc.array;
		    		structTable[foundStructFields].length = objDesc.length;
					structTable[foundStructFields].structID = foundTypeDeclarations-1;
			
					foundStructFields = foundStructFields + 1;
					semicolon();
					}
				else{
					reportError("StructName allready used", false);	
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
	int indexOfLastFoundParameter=(0-1); //parameters must have the same order on symbolTable
	struct ObjectDescription objDesc;
	
	if(PARS_DEBUG >= 2) {
		printf(">parameterlist\n");
	}
    
	objDesc = signature(CLASS_PAR);
	objDesc.class = CLASS_PAR;
	objDesc.funcID = foundFunctions-1;	
	
	if( (objDesc.type > 4) | (objDesc.array==1)){ // string,struct,array	    
		if( objDesc.array ==1 ){
	   		tmpSize = tmpSize*objDesc.length;
	 	}			    
	   	objDesc.address=getAddress(4,2); 
	   	tmpAddress = getAddress(tmpSize,1);
	   	writeToAbsPlaceRelAdr(objDesc.address,1,tmpAddress,1);			    				   											 				 								                    
	}else{
		objDesc.address = getAddress(tmpSize,2);
	}		
	
	
    if(alreadyDec<0){
		saveObjectDescription(objDesc);
    }else{
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
		
			if(alreadyDec<0){
				saveObjectDescription(objDesc);
  			}else{
    			indexOfLastFoundParameter=checkParameter(objDesc,alreadyDec,indexOfLastFoundParameter);
    			if(indexOfLastFoundParameter<0){
    				reportError("function already known with other parameters !", true);
    			}
 			}
				
		} else {
			reportError("type expected", false);
		}
		
		if(alreadyDec<0){
    		indexOfLastFoundParameter=checkParameter(objDesc,alreadyDec,indexOfLastFoundParameter);
 			if(indexOfLastFoundParameter>(0-1)){ //ForwardDeclaration has more parameters
    			reportError("function already declarated with other parameters!", true);
    		}
 		}

	
	}
	if(PARS_DEBUG >= 2) {
		printf("<parameterlist\n");
	}
}

void argumentlist() {
	if(PARS_DEBUG >= 2) {
		printf(">argumentlist\n");
	}
	argument();
	while (currentSymbol == SYM_COMMA) {
		nextSymbol();
		argument(0);
	}
	if(PARS_DEBUG >= 2) {
		printf("<argumentlist\n");
	}
}

void functioncall() {
	if(PARS_DEBUG >= 2) {
		printf(">functioncall\n");
	}
	nextSymbol();
	if (currentSymbol == SYM_RIGHTPARENTHESES) {
		nextSymbol();
	} else {
		argumentlist();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
		} else {
			reportError(") expected, factor, end functioncall", false);
		}
	}
	if(PARS_DEBUG >= 2) {
		printf("<functioncall\n");
	}
}

struct item factor() {
	struct item it;
	struct item nested_it;
	struct ObjectDescription objDesc;
	char id[100];
	if(PARS_DEBUG >= 2) {
		printf(">factor\n");
	}
	it.mode = MODE_REG;
	it.reg = -1;
	if (currentSymbol == SYM_INT_LITERAL) {
		if (numberVal > 0x7FFFFFFF) {
			reportError("integer literal too large", true);
		}
		it.mode = MODE_CONST;
		it.type = lookupType("int");
		it.address = numberVal;
		nextSymbol();
	} else if (currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol();
		it = arithmeticExpression();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
		} else {
			reportError(") expected, factor, end arithmeticexpression", false);
		}
	} else if (currentSymbol == SYM_IDENTIFIER) {
		copy(id, stringVal);
//		objDesc = lookupSymbolicName(id,funcID);

//      wenn struct dann nextSymbol();
//		wenn currentSymbol != SYM_DOT
//			nextSymbol()
//		    wenn identifier
//		       lookup in structtabelle und speicherplatz berechnen               
//			sonst fehler
//		....


		it.mode = MODE_VAR;
		it.type = objDesc.type;
		it.address = objDesc.address;
		nextSymbol();
		if (currentSymbol == SYM_LEFTPARENTHESES) {
			functioncall();
		} else if (currentSymbol == SYM_LEFTBRACKET) {
			nextSymbol();
			nested_it = arithmeticExpression();
	//		it.type = typeTable[it.type].type;
			if(nested_it.mode == MODE_CONST) {
				it.address = it.address + typeTable[it.type].size * nested_it.address;
			} else {
				if(nested_it.mode == MODE_VAR) {
					nested_it = loadIntoRegister(nested_it);
				}
				it.mode = MODE_REG;
				it.reg = getFreeRegister();
				//writeInstruction(INSTR_LDW, it.reg, DATA_BASE_REGISTER, it.address, "load array base address");
				writeInstruction(INSTR_MULI, nested_it.reg, nested_it.reg, typeTable[it.type].size, "multiply array index with element size");;
				writeInstruction(INSTR_ADD, it.reg, it.reg, nested_it.reg, "add base and offset");
			//	writeInstruction(INSTR_LDW, it.reg, DATA_BASE_REGISTER, it.reg, "load element value from memory");
				it.address = -1;
			}
			releaseRegister(nested_it.reg);
			if (currentSymbol == SYM_RIGHTBRACKET) {
				nextSymbol();
			} else {
				reportError("] expected, factor", false);
			}
		} else if (currentSymbol == SYM_DOT) {
			nextSymbol();
			if (currentSymbol == SYM_IDENTIFIER) {
				nextSymbol();
			} else {
				reportError("identifier expected", false);
			}
		}
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
	while ((currentSymbol == SYM_TIMES) | (currentSymbol == SYM_DIV)) {
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
		} else { // DIVISION
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
		}
	}
	if(PARS_DEBUG >= 2) {
		printf("<term\n");
	}
	return it1;
}

struct item arithmeticExpression() {
	struct item it1;
	struct item it2;
	int operation;
	if(PARS_DEBUG >= 2) {
		printf(">arithmeticExpression\n");
	}
	it1 = term();
	while ((currentSymbol == SYM_PLUS) | (currentSymbol == SYM_MINUS)) {
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
		} else { // SUBTRACTION
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
		}
	}
	if(PARS_DEBUG >= 2) {
		printf("<arithmeticExpression\n");
	}
	return it1;
}

void booleanFactor() {
	if(PARS_DEBUG >= 2) {
		printf(">booleanFactor\n");
	}
	if (currentSymbol == SYM_CHAR_LITERAL) {
		nextSymbol();
		// code gen
	} else {
		arithmeticExpression();
	}
	if(PARS_DEBUG >= 2) {
		printf("<booleanFactor\n");
	}
}

void booleanTerm() {
	if(PARS_DEBUG >= 2) {
		printf(">booleanTerm\n");
	}
	if (currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol();
		booleanExpression();
		if (currentSymbol == SYM_RIGHTPARENTHESES) {
			nextSymbol();
		} else {
			reportError(") expected, boolean term", false);
		}
	} else {
		booleanFactor();
		if ((currentSymbol == SYM_EQUAL) | (currentSymbol == SYM_NOTEQUAL) | (currentSymbol == SYM_LESS) | (currentSymbol == SYM_LESSEQUAL) | (currentSymbol == SYM_GREATER) | (currentSymbol == SYM_GREATEREQUAL)) {
			nextSymbol();
			// code gen
		} else {
			reportError("relation symbol expected", false);
		}
		booleanFactor();
	}
	if(PARS_DEBUG >= 2) {
		printf("<booleanTerm\n");
	}
}

void booleanExpression() {
	if(PARS_DEBUG >= 2) {
		printf(">booleanExpression\n");
	}
	booleanTerm();
	while ((currentSymbol == SYM_AND) | (currentSymbol == SYM_OR)) {
		nextSymbol();
		// code gen
		booleanTerm();
	}
	if(PARS_DEBUG >= 2) {
		printf("<booleanExpression\n");
	}
}

void whilestatement() {
	if(PARS_DEBUG >= 2) {
		printf(">whilestatement\n");
	}
	nextSymbol();
	if(currentSymbol == SYM_LEFTPARENTHESES) {
		nextSymbol();
		booleanExpression();
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
		booleanExpression();
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
		writeInstruction(INSTR_RET, 0, 0, 0, "return to address zero");
	}
	semicolon();
	if(PARS_DEBUG >= 2) {
		printf("<returnstatement\n");
	}
}

void assignment(string varName) {
	struct item left_it;
	struct item right_it;
	struct item nested_it;
	struct ObjectDescription objDesc;
	if(PARS_DEBUG >= 2) {
		printf(">assignment\n");
	}
	//objDesc = lookupSymbolicName(varName,funcID);
	left_it.mode = MODE_VAR;
	left_it.address = objDesc.address;
	left_it.type = objDesc.type;
	if (currentSymbol == SYM_LEFTBRACKET) {
		//left_it.type = typeTable[left_it.type].type;
		nextSymbol();
		nested_it = arithmeticExpression();
		if(nested_it.mode == MODE_CONST) {
			left_it.address = left_it.address + typeTable[left_it.type].size * nested_it.address;
		} else {
			nested_it = loadIntoRegister(nested_it);
			left_it.mode = MODE_REG;
			left_it.reg = getFreeRegister();
	//		writeInstruction(INSTR_LDW, left_it.reg, DATA_BASE_REGISTER, left_it.address, "load array base address");
			writeInstruction(INSTR_MULI, nested_it.reg, nested_it.reg, typeTable[left_it.type].size, "multiply array index with element size");;
			writeInstruction(INSTR_ADD, left_it.reg, left_it.reg, nested_it.reg, "add base and offset");
			left_it.address = -1;
		}
		if (currentSymbol == SYM_RIGHTBRACKET) {
			nextSymbol();
		} else {
			reportError("] expected, assignment", false);
		}
	} else if (currentSymbol == SYM_DOT) {
		nextSymbol();
		if (currentSymbol == SYM_IDENTIFIER) {
			nextSymbol();
		} else {
			reportError("identifier expected", false);
		}
	}
	if (currentSymbol == SYM_BECOMES) {
		nextSymbol();
		right_it = argument();
		if(left_it.mode = MODE_CONST) {
		//	writeInstruction(INSTR_STW, right_it.reg, DATA_BASE_REGISTER, left_it.address, "store value into memory");
		} else {
	//		writeInstruction(INSTR_ADD, left_it.reg, DATA_BASE_REGISTER, left_it.reg, "add base and offset");
			writeInstruction(INSTR_STW, right_it.reg, left_it.reg, 0, "store value into memory");
		}
		releaseRegister(right_it.reg);
	} else {
		reportError("= expected", false);
	}
	semicolon();
	if(PARS_DEBUG >= 2) {
		printf("<assignment\n");
	}
}

void statement(){
	char id[100];
	if(PARS_DEBUG >= 2) {
		printf("|>statement\n");
	}
	if (currentSymbol == SYM_IDENTIFIER) {
		copy(id, stringVal);
		nextSymbol();
		if ((currentSymbol == SYM_LEFTBRACKET) | (currentSymbol == SYM_BECOMES) | (currentSymbol == SYM_DOT)) {
			printf("----decideASS!\n");
			assignment(id);
		} else if (currentSymbol == SYM_LEFTPARENTHESES) {
			functioncall();
			semicolon();
		} else {
			reportError("assignment or function call expected", false);
		}
	} else if (currentSymbol == SYM_IF) {
		ifstatement();
	} else if (currentSymbol == SYM_WHILE) {
		whilestatement();
	} else if (currentSymbol == SYM_RETURN) {
		returnstatement();
	} else {
		reportError("statement expected", false);
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



void functiondeclaration(int alreadyDec) {
	struct ObjectDescription objDesc;
	if(PARS_DEBUG >= 2) {
		printf(">functiondeclaration\n");
	}
	nextSymbol();
	if(validType() == 1) {
		parameterlist(alreadyDec);
	}
	if(currentSymbol == SYM_RIGHTPARENTHESES) {
		nextSymbol();
	} else {
		reportError(") expected", false);
	}
	if (currentSymbol == SYM_SEMICOLON) { // == forward declaration
		nextSymbol();
	} else if (currentSymbol == SYM_LEFTCURLYBRACES) {
		if(alreadyDec<0){
			actualFunction=foundFunctions-1;
		}else{
			actualFunction=alreadyDec;
		}									
		
		funcTable[actualFunction].address=currentInstruction;
		funcTable[actualFunction].implemented=1;				
			
		nextSymbol();
		while(validType() == 1) {
			objDesc = signature(CLASS_VAR);
			saveObjectDescription(objDesc);
			if(currentSymbol == SYM_BECOMES) {
				nextSymbol();
				argument();
			}
			semicolon();
		}
		while(currentSymbol != SYM_RIGHTCURLYBRACES) {
			statement();
		}
		resetLocalAddress();
		actualFunction=0-1;
		nextSymbol();
	} else {
		reportError("left curly braces expected", false);
	}
	if(PARS_DEBUG >= 2) {
		printf("<functiondeclaration\n");
	}
}



void parse() {
	
	struct ObjectDescription objDesc;

	setupKeyWordTable();
	
	nextChar();
	nextSymbol();
	
	int tmpAddress=0;
	int tmpSize=0;
	int tmpRegA=0;
	int tmpRegB=0;
	int tmpRegC=0;
	
	int help = 0;
	
	struct item it;
	int alreadyDec = 0;
	
	if(PARS_DEBUG >= 1) {
		printf("--reading includes\n");
	}
	while(currentSymbol == SYM_HASH) {
		directive();
	}
	if(PARS_DEBUG >= 1) {
		printf("--reading constant declarations\n");
	}
    while(currentSymbol == SYM_CONST) {
		
		constdeclaration();
	}
		
	if(PARS_DEBUG >= 1) {
		printf("--struct declarations\n");
	}
	

	while(currentSymbol == SYM_STRUCT) {
		structdeclaration();
	}
	
 
	if(PARS_DEBUG >= 1) {
		printf("--reading variable and function declarations\n");
	}
	
	
	while((currentSymbol == SYM_VOID) | (validType() == 1)) {
		
		if(currentSymbol == SYM_VOID) {
			nextSymbol();
			if(currentSymbol == SYM_IDENTIFIER) {
				
				help=lookupType("void");																
				nextSymbol();
				if(currentSymbol == SYM_LEFTPARENTHESES) {
					alreadyDec=lookupFunction(stringVal);
					if(alreadyDec == (0-1)){
						saveFunction(objDesc.type,stringVal);
						functiondeclaration(alreadyDec);
					}else{
			 			if(funcTable[alreadyDec].implemented==0){
							functiondeclaration(alreadyDec);
			  			}else{
			  				reportError("function already implemented!", true);		
			  			}
					}	  				  				
				} else {
					reportError("( expected", false);
				}
			} else {
				reportError("identifier expected", false);
			}			
		} else {
		
			objDesc = signature(CLASS_VAR);
			if(currentSymbol == SYM_SEMICOLON | currentSymbol == SYM_BECOMES) {				
				
				objDesc.class = CLASS_VAR;
	 			objDesc.funcID=(0-1);
	 				
				tmpSize = typeTable[objDesc.type].size;
			
				if( (objDesc.type > 4) | (objDesc.array==1)){ // string,struct,array	    
			   		if( objDesc.array ==1 ){
			    		tmpSize = tmpSize*objDesc.length;
			   	 	}			    
			    	objDesc.address=getAddress(4,0); 
			    	tmpAddress = getAddress(tmpSize,0);
			    	writeToAbsPlaceRelAdr(objDesc.address,0,tmpAddress,0);			    				   											 				 								                    
				}else{
					objDesc.address = getAddress(tmpSize,0);
				}		
						
				if (currentSymbol == SYM_BECOMES) {
					nextSymbol();				
					it = argument();
				
					if ( (it.mode==MODE_CONST) & (objDesc.array==0) ){				
						if(it.type==lookupType("string")){
							if(objDesc.type==it.type){
								writeStringToMem(it.strValue,objDesc.address,0);
							}else{
								reportError("assignment not possible", true);
							}
						}else if(it.type==lookupType("char")){
				    		if(objDesc.type==it.type){
				    	     	writeCharToMem(it.strValue[0],objDesc.address,0);
				    		}else{
				    			reportError("assignment not possible", true);
				    		}	
				    	}else { //interger
					    	if(it.type==lookupType("int")){
					    		writeIntegerToMem(it.address,objDesc.address,0);
					    	}else{
					   			reportError("assignment not possible", true);
					    	}					    	
				   		}				   			
					}else{
						reportError("assignment not possible", true);
					}
				semicolon();
				}else {
					nextSymbol(); //semicolon
				}
		
				saveObjectDescription(objDesc);	
						
			}else if(currentSymbol == SYM_LEFTPARENTHESES) {
				alreadyDec=lookupFunction(objDesc.name);			
				if(alreadyDec == (0-1)){
					saveFunction(objDesc.type,objDesc.name);
					functiondeclaration(alreadyDec);
				}else{
			 		if(funcTable[alreadyDec].implemented==0){
						functiondeclaration(alreadyDec);
				  	}else{
			  			reportError("function already implemented!", true);		
			  		}
				}	  				  	
			} else {
				reportError("; or = or ( expected", false);
			}		
		}
	
	}
	writeInstruction(INSTR_RET,0,0,31, "jump to function main");  //ENDE ( nur zum testen)

	
	showTypeTable();
	
	showStructTable();
	
	showFuncTable();

	showSymbolTable();
	
	
	
	if (currentSymbol != SYM_EOF) {
		reportError("Unknown type encountered!", false);
	}

	printf("\n\n\tParsing completed!");
	printf("\n\tSymbolic names found: %i", foundSymbolicNames);
	printf("\n\tTypeDeclarations found: %i", foundTypeDeclarations);
	printf("\n\tFunctions found: %i", foundFunctions);
	printf("\n\tStructFields found: %i", foundStructFields);
	printf("\n\tUnrecognized symbols: %i", unrecognizedSymbols);
	printf("\n\tErrors occured: %i\n", occurredErrors);
	return;
}
