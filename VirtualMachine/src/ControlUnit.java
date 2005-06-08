/*
 * Created on 17.05.2005
 */

/**
 * @author dknoll
 */
public class ControlUnit
    implements Instructions {

  private VM vm;

  private int programCounter; //index of current instruction to execute

  private int[] instructionRegister; //the instruction register


  private int opCode, operandA, operandB, operandC;

  public ControlUnit(VM vm) {
    this.vm = vm;
    programCounter = 0;
    instructionRegister = new int[2];
    instructionRegister[0]=0;
    instructionRegister[1]=1;
    opCode = 0;
    operandA = 0;
    operandB = 0;
    operandC = 0;
  }

  public void fetchAndDecode() {
    instructionRegister[0] = vm.memory[programCounter*2];
    instructionRegister[1] = vm.memory[programCounter*2+1];

    opCode =   (0xFF000000 & instructionRegister[0]) >> 24; //our opcode 8 Bit
    // has 8 Bits
    operandA = (0x00FF0000 & instructionRegister[0]) >> 16;//  8 Bit
    operandB = (0x0000FF00 & instructionRegister[0]) >> 8; //  8 Bit


    operandC = instructionRegister[1];  //32 Bit
    if (opCode < ADDI) { //immediate or not?
      operandC = vm.cpuRegister[operandC & 0x0000001F];
    }
  }

  /**
   * This method prints the current instruction in a human readable format on
   * the screen.
   */
  void prettyPrint() {

    int opA, opB, opC;
    opA = (0x00FF0000 & instructionRegister[0]) >> 16;
    opB = (0x0000FF00 & instructionRegister[0]) >> 8;
    opC =  instructionRegister[1];

    System.out.print(programCounter + ": " + getOpcodeString());
    System.out.print(" R" + opA);
    System.out.print(", R" + opB);
    if (opCode < ADDI) {
      System.out.print(", R" + opC);
    }
    else if (opCode < BEQ) {

      System.out.print(", i" + opC);
    }
    else {
      System.out.print(", a" + opC);
    }
    System.out.println();

  }

  /**
   * @return
   */
  private String getOpcodeString() {
    String returnValue;
    switch (opCode) {
      case ADD:
        return "ADD";
      case ADDI:
        return "ADDI";
      case SUB:
        return "SUB";
      case SUBI:
        return "SUBI";
      case CMP:
        return "CMPI";
      case CMPI:
        return "CMPI";
      case MUL:
        return "MUL";
      case MULI:
        return "MULI";
      case DIV:
        return "DIV";
      case DIVI:
        return "DIVI";
      case MOD:
        return "MOD";
      case MODI:
        return "MODI";
      case OR:
        return "OR";
      case ORI:
        return "ORI";
      case AND:
        return "AND";
      case ANDI:
        return "ANDI";
      case BIC:
        return "BIC";
      case BICI:
        return "BICI";
      case XOR:
        return "XOR";
      case XORI:
        return "XORI";
      case SHL:
        return "SHL";
      case SHLI:
        return "SHLT";
      case SHA:
        return "SHA";
      case SHAI:
        return "SHAI";
      case CHK:
        return "CHK";
      case CHKI:
        return "CHKI";
        // -- -- -- -- -- -- -- -- MEMORY-OPERATIONS -- -- -- -- -- -- -- --
      case LDW:
        return "LDW";
      case POP:
        return "POP";
      case STW:
        return "STW";
      case PSH:
        return "PSH";
      case STB:
        return "STB";
        // -- -- -- -- -- -- -- -- BRANCH - OPERATIONS -- -- -- -- -- -- -- --
      case BEQ:
        return "BEQ";
      case BNE:
        return "BNE";
      case BLT:
        return "BLT";
      case BGE:
        return "BGE";
      case BLE:
        return "BLE";
      case BGT:
        return "BGT";
      case BSR:
        return "BSR";
      case JSR:
        return "JSR";
      case RET:
        return "RET";
        // -- -- -- -- -- -- -- -- I/O - OPERATIONS -- -- -- -- -- -- -- --
      case RD:
        return "RD";
      case WRD:
        return "WRD";
      case PSTR:
        return "PSTR";
      case ALL:
        return "ALL";
      case DALL:
        return "DALL";
      case PINT:
        return "PINT";
      default:
        return "NOT KNOWN";
    }

  }

  public void setProgramcounter(int next) throws VirtualMachineException {
    if (next >= vm.programLength) {
      throw new VirtualMachineException(
          "Invalid program counter value occured (" + next + ", at "
          + programCounter + ")");
    }
    this.programCounter = next;
  }

  public int getProgramCounter() {
    return programCounter;
  }

  public int getOpCode() {
    return opCode;
  }

  public int getOperandA() {
    return operandA;
  }

  public int getOperandB() {
    return operandB;
  }

  public int getOperandC() {
    return operandC;
  }
}