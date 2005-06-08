#ifndef _SYM_H_
#define _SYM_H_

const int SCAN_DEBUG = 0;
const int PARS_DEBUG = 2;
const int CODE_DEBUG = 2;

// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- S C A N N E R
// ----------------------------------------------------------------------------

// SPECIAL CHARS
const int SYM_NULL = 0;
const int SYM_EOF = -2;

// GENERAL SYMBOLS
const int SYM_COMMENT = 4;
const int SYM_IDENTIFIER = 5;
const int SYM_INT_LITERAL = 6;
const int SYM_CHAR_LITERAL = 7;
const int SYM_STRING_LITERAL = 8;

// OPERATORS
const int SYM_BECOMES = 10;
const int SYM_PLUS = 11;
const int SYM_MINUS = 12;
const int SYM_TIMES = 13;
const int SYM_DIV = 14;
const int SYM_MOD = 15;
const int SYM_AND = 16;
const int SYM_OR = 17;
const int SYM_EQUAL = 18;
const int SYM_NOTEQUAL = 19;
const int SYM_LESS = 20;
const int SYM_LESSEQUAL = 21;
const int SYM_GREATER = 22;
const int SYM_GREATEREQUAL = 23;
const int SYM_NOT = 24;
// SIGNS
const int SYM_HASH = 30;
const int SYM_DOT = 31;
const int SYM_COMMA = 32;
const int SYM_SEMICOLON = 33;
// BRACES
const int SYM_LEFTPARENTHESES = 34;
const int SYM_RIGHTPARENTHESES = 35;
const int SYM_LEFTBRACKET = 36;
const int SYM_RIGHTBRACKET = 37;
const int SYM_LEFTCURLYBRACES = 38;
const int SYM_RIGHTCURLYBRACES = 39;

// == == KEYWORDS == ==
const int KEYWORD_OFFSET = 60;
// CONSTRUCTS AND TYPES
const int SYM_BREAK = 60;
const int SYM_CASE = 61;
const int SYM_CHAR = 62;
const int SYM_CONST = 63;
const int SYM_CONTINUE = 64;
const int SYM_DEFAULT = 65;
const int SYM_DO = 66;
const int SYM_ELSE = 67;
const int SYM_IF = 68;
const int SYM_INCLUDE = 69;
const int SYM_INT = 70;
const int SYM_RETURN = 71;
const int SYM_STRUCT = 72;
const int SYM_SWITCH = 73;
const int SYM_TYPEDEF = 74;
const int SYM_VOID = 75;
const int SYM_WHILE = 76;
const int SYM_BOOL = 77;
const int SYM_TRUE = 78;
const int SYM_FALSE = 79;
const int SYM_FUNC = 80;
const int SYM_FILE = 81;
const int SYM_STRING = 82;
const int SYM_PRINT = 83;
const int SYM_ALLOC = 84;
const int SYM_DEALLOC = 85;

const int MAX_KEYWORDS = 26;

const int MAX_STRING_LENGTH = 40;

// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- P A R S E R
// ----------------------------------------------------------------------------

const int CLASS_CONST = 100;
const int CLASS_VAR = 101;
const int CLASS_FUNC = 103;
const int CLASS_PAR  = 104;
const int CLASS_RET_VALUE = 105;

const int TYPE_VOID = 200;
const int TYPE_INT = 201;
const int TYPE_CHAR = 202;
const int TYPE_STRING = 203;
const int TYPE_FILE = 204;
const int TYPE_BOOL = 205;
const int TYPE_ARRAY = 206;
const int TYPE_STRUCT = 207;

const int MODE_VAR = 401;
const int MODE_CONST = 402;
const int MODE_REG = 403;

const int ADR_GLOBAL = 0;
const int ADR_LOCAL = 1;
const int ADR_ABSOLUTE = 2;
// ----------------------------------------------------------------------------
// -- -- -- -- -- -- -- C O D E G E N E R A T O R
// ----------------------------------------------------------------------------

const int INSTR_ADD = 33;
const int INSTR_SUB = 34;
const int INSTR_MUL = 35;
const int INSTR_DIV = 36;
const int INSTR_MOD = 37;
const int INSTR_CMP = 38;
const int INSTR_OR = 39;
const int INSTR_AND = 40;
const int INSTR_BIC = 41;
const int INSTR_XOR = 42;
const int INSTR_SHL = 43;
const int INSTR_SHA = 44;
const int INSTR_CHK = 45;

const int INSTR_ADDI = 46;
const int INSTR_SUBI = 47;
const int INSTR_MULI = 48;
const int INSTR_DIVI = 49;
const int INSTR_MODI = 50;
const int INSTR_CMPI = 51;
const int INSTR_ORI = 52;
const int INSTR_ANDI = 53;
const int INSTR_BICI = 54;
const int INSTR_XORI = 55;
const int INSTR_SHLI = 56;
const int INSTR_SHAI = 57;
const int INSTR_CHKI = 58;

const int INSTR_LDW = 59;
const int INSTR_LDB = 60;
const int INSTR_POP = 61;
const int INSTR_STW = 62;
const int INSTR_STB = 63;
const int INSTR_PSH = 64;

const int INSTR_BEQ = 65;
const int INSTR_BNE = 66;
const int INSTR_BLT = 67;
const int INSTR_BGE = 68;
const int INSTR_BLE = 69;
const int INSTR_BGT = 70;
const int INSTR_BSR = 71;
const int INSTR_JSR = 72;
const int INSTR_RET = 73;

const int INSTR_RD = 74;
const int INSTR_WRD = 75;
const int INSTR_OPF = 76;
const int INSTR_CLF = 77;
const int INSTR_WRBF = 78;
const int INSTR_RDBF = 79;
const int INSTR_PSTR = 80;
const int INSTR_ALL  = 81;
const int INSTR_DALL = 82;
const int INSTR_PINT = 83;

#endif //_SYM_H_
