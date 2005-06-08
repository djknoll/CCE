#include "bootstrap.h"
#include "sym.h"

struct keyWord {
	char name[8];
};

struct keyWord keywords[23];

file inFile;
int currentLine;
int currentRow;

char stringVal[40];
int numberVal = 0;
char currentChar = 0;
int currentSymbol = 0;
int unrecognizedSymbols = 0;
int occurredErrors = 0;

void reportError(char msg[], bool final);
void block();
struct item arithmeticExpression();
void booleanExpression();


// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- S C A N N E R
// ----------------------------------------------------------------------------

void openInputFile(char filename[]) {
	inFile = fopen(filename, "r");
	currentLine = 1;
	currentRow = 1;
}

void closeInputFile() {
	fclose(inFile);
}

void nextChar() {
	if(feof(inFile) == 0) {
		currentChar = fgetc(inFile);
		if(currentChar < 0) {
			currentChar = SYM_EOF;
		}
		
		if(SCAN_DEBUG>=2) {
			if(currentChar<' ') {
				printf("?%i", currentChar);
			} else {
				printf("%c", currentChar);
			}
		}
		if(currentChar == 10) {
			currentLine = currentLine + 1;
			currentRow = 1;
		} else {
			currentRow = currentRow + 1;
		}
	} else {
		currentChar = SYM_EOF;
	}
}

int number() {
	int exit = 0;
	numberVal = 0;
	while((currentChar!=SYM_EOF) & (exit == 0)) {
		if((currentChar >= '0') & (currentChar <= '9')) {
			if (numberVal > (0x7FFFFFFF-currentChar-48)/10) {
				reportError("integer literal too large", true);
			}
			numberVal = 10 * numberVal + currentChar - 48;
			nextChar();
		} else {
			exit = 1;
		}
	}
	return 0;
}

int identifier() {
	int exit = 0;
	int count = 0;
	while((currentChar!=SYM_EOF) & (exit == 0) & (count < 40)) {
		if(((currentChar >= 'A') & (currentChar <= 'Z')) | ((currentChar >= 'a') & (currentChar <= 'z')) | ((currentChar >= '0') & (currentChar <='9')) | (currentChar == '_')) {
			stringVal[count] = currentChar;
			count = count + 1;
			nextChar();
		} else {
			stringVal[count] = 0; // ==> '\0'
			exit = 1;
		}
	}
	return 0;
}

int stringLiteral() {
	int exit = 0;
	int count = 0;
	nextChar();
	while((currentChar != SYM_EOF) & (exit == 0)) {
		if(currentChar == 34) { // ==> '"'
			stringVal[count] = 0; // ==> '\0'
			exit = 1;
		} else if ( count < 100 ) {
			stringVal[count] = currentChar;
			count = count + 1;
		} else {
			return -1;
		}
		nextChar();
	}
	return 0;
}

int character() {
	int exit = 0;
	int count = 0;

	nextChar();
	while((currentChar != SYM_EOF) & (exit == 0)) {
		if(currentChar == 39) { // ==> '\''
			stringVal[count] = 0; // ==> '\0'
			exit = 1;
		} else if ( count < 100 ) {
			stringVal[count] = currentChar;
			count = count + 1;
		} else {
			return -1;
		}
		nextChar();
	}
	return 0;
}

int getStringLength(char dat[]) {
	int result = 0;
	while(dat[result] != 0) { // ==> '\0'
		//printf("%c ",dat[result]);
		result = result + 1;
	}
	return result;
}

int compare(char dat1[], char dat2[]) {
	int returnValue = 0;
	int pos = getStringLength(dat1);
	int minLength = getStringLength(dat2);
	if(pos == minLength) {
		pos = 0;
		returnValue = 1;
		while((pos < minLength) & (returnValue > 0)) {
			if(dat1[pos] != dat2[pos]) {
				returnValue = 0;
			}
			pos = pos +1;
		}
	}
	return returnValue;
}

void copy(char target[], char source[]) {
	int pos = 0;
	int length = getStringLength(source);
	while(pos < length) {
		target[pos] = source[pos];
		pos = pos + 1;
	}
	target[pos] = 0;
}

// This routine now implements a linear search of the given string
// in the array of keywords
int getMatchingKeywordID() {
	int currentKeyPos = -1;
	int returnValue = -1;
	while((currentKeyPos < MAX_KEYWORDS) & (returnValue == -1)) {
		currentKeyPos = currentKeyPos + 1;
		if(compare(keywords[currentKeyPos].name, stringVal) == 1) {
			returnValue = currentKeyPos + KEYWORD_OFFSET;
		}
	}
	return returnValue;
}


void nextSymbol() {
	int exit=0;
	int ret=0;
	currentSymbol = SYM_NULL;
	if(SCAN_DEBUG >= 1) {
		printf(">nextSymbol\n");
	}

	while((currentChar <= ' ') & (currentChar != SYM_EOF)) {
		//printf(".");
		nextChar();
	}
	if(currentChar == SYM_EOF){
		currentSymbol= SYM_EOF;
	} else if(currentChar == '#') {
		nextChar();
		currentSymbol = SYM_HASH;
	} else if(currentChar == ';') {
		nextChar();
		currentSymbol = SYM_SEMICOLON;
	} else if(currentChar == ',') {
		nextChar();
		currentSymbol = SYM_COMMA;
	} else if(currentChar == '.') {
		nextChar();
		currentSymbol = SYM_DOT;
	} else if(currentChar == '(') {
		nextChar();
		currentSymbol = SYM_LEFTPARENTHESES;
	} else if(currentChar == ')') {
		nextChar();
		currentSymbol = SYM_RIGHTPARENTHESES;
	} else if(currentChar == '[') {
		nextChar();
		currentSymbol = SYM_LEFTBRACKET;
	} else if(currentChar == ']') {
		nextChar();
		currentSymbol = SYM_RIGHTBRACKET;
	} else if(currentChar == '{') {
		nextChar();
		currentSymbol = SYM_LEFTCURLYBRACES;
	} else if(currentChar == '}') {
		nextChar();
		currentSymbol = SYM_RIGHTCURLYBRACES;
	} else 	if(currentChar == '=') {
		nextChar();
		currentSymbol = SYM_BECOMES;
		if(currentChar == '=') {
			nextChar();
			currentSymbol = SYM_EQUAL;
		}
	} else 	if(currentChar == '<') {
		nextChar();
		currentSymbol = SYM_LESS;
		if(currentChar == '=') {
			nextChar();
			currentSymbol = SYM_LESSEQUAL;
		}
	} else 	if(currentChar == '>') {
		nextChar();
		currentSymbol = SYM_GREATER;
		if(currentChar == '=') {
			nextChar();
			currentSymbol = SYM_GREATEREQUAL;
		}
	} else if(currentChar == '+') {
		nextChar();
		currentSymbol = SYM_PLUS;
	} else if(currentChar == '-') {
		nextChar();
		currentSymbol = SYM_MINUS;
	} else if(currentChar == '*') {
		nextChar();
		currentSymbol = SYM_TIMES;
	} else if(currentChar == '/') {
		nextChar();
		if(currentChar == '/') { // comment starting with "//"
			nextChar();
			while(currentChar != 10) { nextChar(); } // ignore until carrige return
			nextChar();
			nextSymbol(); //SYM_COMMENT;
		} else if (currentChar == '*') { // comment starting with / *
			nextChar();
			while(exit == 0) {
				while((currentChar != '*') & (currentChar != SYM_EOF)) { nextChar(); }
				nextChar();
				if(currentChar == '/') {
					exit = 1;
				} else if(currentChar == SYM_EOF) {
					exit = 1;
					reportError("unfinished comment at end of file", true);
				}
			}
			nextChar();
			nextSymbol(); //SYM_COMMENT;
		} else {
			currentSymbol = SYM_DIV;
		}
	} else if(currentChar == '%') {
		nextChar();
		currentSymbol = SYM_MOD;
	} else if(currentChar == '|') {
		nextChar();
		currentSymbol = SYM_OR;
	} else if(currentChar == '&') {
		nextChar();
		currentSymbol = SYM_AND;
	} else if(currentChar == '!') {
		nextChar();
		currentSymbol = SYM_NOT;
		if(currentChar == '=') {
			nextChar();
			currentSymbol = SYM_NOTEQUAL;
		}
	} else if((currentChar >= '0') & (currentChar <= '9')) {
		number();
		currentSymbol = SYM_INT_LITERAL;
	} else if(((currentChar >='A') & (currentChar <= 'Z')) |
		((currentChar >='a') & (currentChar <= 'z'))) { // -- -- -- -- IDENTIFIER -- -- -- -- --
		identifier();
		currentSymbol = getMatchingKeywordID(); // -- -- KEYWORDS -- --
		if(currentSymbol == -1) {
			 currentSymbol = SYM_IDENTIFIER;
		}
	} else if(currentChar == 34) { // -- -- -- -- -- STRING LITERAL -- -- -- -- --
		ret = stringLiteral();
		if(ret < 0) {
			reportError("string literal exceeded maximal size!!!", true);
		}
		currentSymbol = SYM_STRING_LITERAL;
	} else if (currentChar == 39) { // -- -- -- -- -- CHAR LITERAL -- -- -- -- --
		ret = character();
		if(ret < 0) {
			reportError("character literal exceeded maximal size!!!", true);
		}
		currentSymbol = SYM_CHAR_LITERAL;
	} else {
		currentSymbol = SYM_NULL;
		printf("\n>> CHARACTER not recognized in line %i! (%i)\n", currentLine, currentChar);
		nextChar();
	}

	if(SCAN_DEBUG >= 3) {
		printf("_->%i :: %i", currentSymbol, currentChar);
	}
	if(SCAN_DEBUG >= 1) {
		if(currentSymbol >= KEYWORD_OFFSET) {
			printf("\nkeyword:%i_%s_\n", currentSymbol, stringVal);
		} else if(currentSymbol == SYM_IDENTIFIER) {
			printf("\nidentifier:%i_%s_\n", currentSymbol, stringVal);
		} else if(currentSymbol == SYM_CHAR_LITERAL) {
			printf("character literal:'%s'\n", stringVal);
		} else if(currentSymbol == SYM_STRING_LITERAL) {
			printf("string literal:'%s'\n", stringVal);
		} else if(currentSymbol == SYM_INT_LITERAL) {
			printf("integer literal:%i\n", numberVal);
		} else if(currentSymbol == SYM_NULL) {
			unrecognizedSymbols = unrecognizedSymbols + 1;
			printf("\n>> SYMBOL not recognized in line %i! (%i)\n", currentLine, currentChar);
			//getchar();
		} else {
			printf("sym:%i, ", currentSymbol);
		}
	}
	if(SCAN_DEBUG >= 1) {
		printf("<nextSymbol\n");
	}
}

void setupKeyWordTable(){
	  copy(keywords[0].name , "break");
	  copy(keywords[1].name , "case");
	  copy(keywords[2].name , "char");
	  copy(keywords[3].name , "const");
	  copy(keywords[4].name , "continue");
	  copy(keywords[5].name , "default");
	  copy(keywords[6].name , "do");
	  copy(keywords[7].name , "else");
	  copy(keywords[8].name , "if");
	  copy(keywords[9].name , "include");
	  copy(keywords[10].name , "int");
	  copy(keywords[11].name , "return");
	  copy(keywords[12].name , "struct");
	  copy(keywords[13].name , "switch");
	  copy(keywords[14].name , "typedef");
	  copy(keywords[15].name , "void");
	  copy(keywords[16].name , "while");
	  copy(keywords[17].name , "bool");
	  copy(keywords[18].name , "true");
	  copy(keywords[19].name , "false");
	  copy(keywords[20].name , "func");
	  copy(keywords[21].name , "file");
	  copy(keywords[22].name , "string");
	  copy(keywords[23].name , "print");
	  copy(keywords[24].name , "alloc");
	  copy(keywords[25].name , "dealloc");
 }

