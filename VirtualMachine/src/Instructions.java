public interface Instructions {
	/* our instruction set */
	final int ADD = 33;
	final int SUB = 34;
	final int MUL = 35;
	final int DIV = 36;
	final int MOD = 37;
	final int CMP = 38;
	final int OR = 39;
	final int AND = 40;
	final int BIC = 41;
	final int XOR = 42;
	final int SHL = 43;
	final int SHA = 44;
	final int CHK = 45;

	final int ADDI = 46;
	final int SUBI = 47;
	final int MULI = 48;
	final int DIVI = 49;
	final int MODI = 50;
	final int CMPI = 51;
	final int ORI = 52;
	final int ANDI = 53;
	final int BICI = 54;
	final int XORI = 55;
	final int SHLI = 56;
	final int SHAI = 57;
	final int CHKI = 58;

	final int LDW = 59;
	final int LDB = 60;
	final int POP = 61;
	final int STW = 62;
	final int STB = 63;
	final int PSH = 64;

	final int BEQ = 65;
	final int BNE = 66;
	final int BLT = 67;
	final int BGE = 68;
	final int BLE = 69;
	final int BGT = 70;
	final int BSR = 71;
	final int JSR = 72;
	final int RET = 73;

	final int RD = 74;
	final int WRD = 75;
	final int OPF = 76;
	final int CLF = 77;
	final int WRBF = 78;
	final int RDBF = 79;
	final int PSTR = 80;
	final int ALL  = 81;
	final int DALL = 82;
	final int PINT = 83;

}