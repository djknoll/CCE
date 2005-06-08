#include "Parser.c"

int main(int argc, char* argv[]) {
	
	char* ifn = argv[1];
	char* ofn;
	
	if(ifn == NULL) {
		printf("No input file given.\nusage: Compiler <source file> <dest file>");
		return -1;
	}
	openInputFile(ifn);
	if(inFile == NULL) {
		printf("Error opening the file: %s\n", ifn);
		return -1;
	}

	ofn = argv[2];
	if(ofn == NULL) {
		printf("No output file given.\nusage: Compiler <source file> <dest file>");
		return -1;
	}
	openOutputFile(ofn);
	if(outFile == NULL) {
		printf("Error opening the file: %s\n", ofn);
		return -1;
	}
	parse();

	closeInputFile();
	closeOutputFile();

	return 0;
}
