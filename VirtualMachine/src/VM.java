import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;

/**
 * @author Bernhard Sehorz, Dominik Knoll
 */
public class VM {

  // GLOBAL CONSTANTS
  final int DEBUG = 2;

  final int MAX_REGISTERS = 32;

  final int MAX_FILE_HANDLES = 16;

  final int MAX_STRING_LENGTH = 40; //stringLength mod 4 == 0!!!

  // RUNTIME STATE OF VIRTUAL MACHINE
  int[] memory; //our RAM

  int[] cpuRegister; //our registers

  Object[] fileHandles;

  int programLength;

  int heapBase;

  int openHandles;

  Heap heap;

  private ControlUnit cu;

  private BufferedReader br;
  private boolean firstInstruction=true;

  // CONSTRUCTORS
  private VM() {
  }

  public VM(int memSize) {
    cpuRegister = new int[MAX_REGISTERS];
    memory = new int[memSize / 4];
    cu = new ControlUnit(this);
    br = new BufferedReader(new InputStreamReader(System.in));
    fileHandles = new Object[MAX_FILE_HANDLES];
  }

  public static void main(String[] args) {
    if (args.length < 1) {
      System.out.println("Usage: java VM <program>");
      System.exit(0);
    }

    VM vm = new VM(1024 * 1024 * 2); //2 MB
    vm.setupRegAndMem(args);
    vm.loadProgramfile(0, args[0]);
    vm.execute();
  }

  void execute() {
    int a, b, c, next;
    boolean exit = false;
    Object fh;

    try {
      System.out.println("\nStart execution ... ");
      do {
        cu.fetchAndDecode();
        if (DEBUG >= 2) {
          cu.prettyPrint();
        }
        next = cu.getProgramCounter() + 1;
        a = cu.getOperandA();
        b = cu.getOperandB();
        c = cu.getOperandC();

        switch (cu.getOpCode()) { //what do we do?
          // -- -- -- -- -- -- ARITHMETIC / LOGIC - OPERATIONS -- -- -- --
          // -- --
          case ControlUnit.ADD:
          case ControlUnit.ADDI:
            cpuRegister[a] = cpuRegister[b] + c;
            break;
          case ControlUnit.SUB:
          case ControlUnit.SUBI:
            cpuRegister[a] = cpuRegister[b] - c;
            break;
          case ControlUnit.CMP:
          case ControlUnit.CMPI:
            cpuRegister[a] = cpuRegister[b] - c;
            break;
          case ControlUnit.MUL:
          case ControlUnit.MULI:
            cpuRegister[a] = cpuRegister[b] * c;
            break;
          case ControlUnit.DIV:
          case ControlUnit.DIVI:
            cpuRegister[a] = cpuRegister[b] / c;
            break;
          case ControlUnit.MOD:
          case ControlUnit.MODI:
            cpuRegister[a] = cpuRegister[b] % c;
            break;
          case ControlUnit.OR:
          case ControlUnit.ORI:
            cpuRegister[a] = cpuRegister[b] | c;
            break;
          case ControlUnit.AND:
          case ControlUnit.ANDI:
            cpuRegister[a] = cpuRegister[b] & c;
            break;
          case ControlUnit.BIC:
          case ControlUnit.BICI:
            cpuRegister[a] = cpuRegister[b] & ~c;
            break;
          case ControlUnit.XOR:
          case ControlUnit.XORI:
            cpuRegister[a] = cpuRegister[b] * c;
            break;
          case ControlUnit.CHK:
          case ControlUnit.CHKI:
            if (cpuRegister[a] < 0 || cpuRegister[a] >= c) {
              throw new VirtualMachineException(
                  "Invalid array index occured ("
                  + cpuRegister[a] + ", at "
                  + cu.getProgramCounter() + ")");
            }
            break;
            // -- -- -- -- -- -- -- -- MEMORY - OPERATIONS -- -- -- -- -- --
            // -- --
          case ControlUnit.LDB: {
            int byteIndex = (cpuRegister[b] + c) % 4;
            cpuRegister[a] = memory[ (cpuRegister[b] + c) / 4] >>>
                byteIndex * 8;
            break;
          }
          case ControlUnit.STB: {
            int byteIndex = (cpuRegister[b] + c) % 4;
            int tmpVal = memory[ (cpuRegister[b] + c) / 4];
            if(byteIndex==0){
              tmpVal=tmpVal&0xFFFFFF00;
            }else if (byteIndex==1){
              tmpVal=tmpVal&0xFFFF00FF;
            }else if (byteIndex==2){
              tmpVal=tmpVal&0xFF00FFFF;
            }else if (byteIndex==3){
              tmpVal=tmpVal&0x00FFFFFF;
            }
            tmpVal = tmpVal | ((cpuRegister[a] & 0xFF) << byteIndex * 8);
            memory[ (cpuRegister[b] + c) / 4] = tmpVal;
            break;
          }
          case ControlUnit.LDW:
            cpuRegister[a] = memory[ (cpuRegister[b] + c) / 4];
            if (DEBUG >= 1) {
              System.out.println("loaded " + cpuRegister[a]
                                 + " from " + (cpuRegister[b] + c));
            }
            break;
          case ControlUnit.POP:
            cpuRegister[a] = memory[cpuRegister[b] / 4];
            cpuRegister[b] += c;
            break;
          case ControlUnit.STW:
            memory[ (cpuRegister[b] + c) / 4] = cpuRegister[a];
            if (DEBUG >= 1) {
              System.out.println("stored " + cpuRegister[a] + " at "
                                 + (cpuRegister[b] + c));
            }
            break;
          case ControlUnit.PSH:
            cpuRegister[b] -= c;
            memory[cpuRegister[b] / 4] = cpuRegister[a];
            break;
            // -- -- -- -- -- -- -- -- BRANCH - OPERATIONS -- -- -- -- -- --
            // --
          case ControlUnit.BEQ:
            if (cpuRegister[a] == 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BNE:
            if (cpuRegister[a] != 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BLT:
            if (cpuRegister[a] < 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BGE:
            if (cpuRegister[a] >= 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BLE:
            if (cpuRegister[a] <= 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BGT:
            if (cpuRegister[a] > 0) {
              next = cu.getProgramCounter() + c;
            }
            break;
          case ControlUnit.BSR:
            next = cu.getProgramCounter() + c;
            cpuRegister[31] = (cu.getProgramCounter() + 1);
            break;
          case ControlUnit.JSR:
            next = c;
            cpuRegister[31] = (cu.getProgramCounter() + 1);
            break;
          case ControlUnit.RET:
            next = cpuRegister[c & 0x0000001F];
            exit = (next == 0);
            break;
            // -- -- -- -- -- -- -- -- I/O - OPERATIONS -- -- -- -- -- -- --
            // --
          case ControlUnit.RD:
            System.out.print("enter number: ");
            cpuRegister[a] = Integer.parseInt(br.readLine());
            break;
          case ControlUnit.WRD:
            System.out.println("r[" + c + "]: " + cpuRegister[c]);
            break;
          case ControlUnit.OPF:
            String filename = stringFromMemory(cpuRegister[b]);
            if (c == 5) { //read
              FileInputStream fis = new FileInputStream(filename);
              fileHandles[openHandles] = fis;
              cpuRegister[a] = openHandles;
              openHandles += 1;
            }
            else { // write
              FileOutputStream fos = new FileOutputStream(filename);
              fileHandles[openHandles] = fos;
              cpuRegister[a] = openHandles;
              openHandles += 1;
            }
            break;
          case ControlUnit.CLF:
            fh = fileHandles[cpuRegister[a]];
            if (fh instanceof FileInputStream) {
              ( (FileInputStream) fh).close();
            }
            else {
              ( (FileOutputStream) fh).close();
            }
            break;
          case ControlUnit.WRBF:
            fh = fileHandles[cpuRegister[a]];
            if (fh instanceof FileInputStream) {
              throw new IOException(
                  "Error when trying to write to a input file");
            }
            ( (FileOutputStream) fh).write(cpuRegister[b]);
            break;
          case ControlUnit.RDBF:
            fh = fileHandles[cpuRegister[a]];
            if (fh instanceof FileOutputStream) {
              throw new IOException(
                  "Error when trying to read from output file");
            }
            cpuRegister[a] = (byte) ( (FileInputStream) fh).read();
            break;
          case ControlUnit.PSTR:
            System.out
                .println(stringFromMemory( (cpuRegister[a] + c) / 4));
            break;
          case ControlUnit.ALL:
            cpuRegister[a] = heap.allocate(c);
            break;
          case ControlUnit.DALL:
            heap.deAllocate(cpuRegister[a], c);
            break;
          case ControlUnit.PINT:
            System.out.println(cpuRegister[a]);
            break;


          default:
            throw new VirtualMachineException(
                "Unknown opcode encountered (" + cu.getOpCode()
                + ", at " + cu.getProgramCounter() + ")");
        }
        if(this.firstInstruction==true){
          firstInstruction = false;
          if(cpuRegister[1]<=0){
            throw new MainNotFoundException(
                  "main not found");
          }
        }

        cu.setProgramcounter(next);
        if (DEBUG >= 2) {
          displayRegisterContent();
        }
      }
      while (!exit);

     // displayDataMemoryContent();

      System.out.println(" ... execution finished.");

    }
    catch (MainNotFoundException e) {
         System.out.flush();
         System.err.println("Main not found ... exiting");
         e.printStackTrace();
         System.exit(0);
       }
    catch (OutOfHeapMemException e) {
      System.out.flush();
      System.err.println("Illegal memory access ... exiting");
      e.printStackTrace();
      System.exit(0);
    }
    catch (ArrayIndexOutOfBoundsException e) {
      System.out.flush();
      System.err.println("Illegal memory access ... exiting");
      e.printStackTrace();
      System.exit(0);
    }
    catch (NumberFormatException e) {
      System.out.flush();
      System.err.println("Wrong number format ... exiting");
      e.printStackTrace();
      System.exit(0);
    }
    catch (IOException e) {
      System.out.flush();
      System.err.println("I/O-Exception encountered ... exiting");
      e.printStackTrace();
      System.exit(0);
    }
    catch (VirtualMachineException e) {
      System.out.flush();
      System.err.println("VirtualMachineException occured  ... exiting");
      System.err.println(e.getMessage());
      e.printStackTrace();
      System.exit(0);
    }
  }

  /**
   * This method shows the numerical content of all non-zero registers. NOTE:
   * used only for debugging purpose
   */
  private void displayRegisterContent() {
    for (int i = 0; i < MAX_REGISTERS; i++) {
      if (cpuRegister[i] != 0) {
        System.out.print("R" + i + ":" + cpuRegister[i] + "|");
      }
    }
    System.out.println("\n");
  }

  /**
   * This method shows the numerical content of all non-zero memory cells.
   * NOTE: used only for debugging purpose
   */
  private void displayDataMemoryContent() {
    for (int i = programLength; i < memory.length; i++) {
      if (memory[i] != 0) {
        System.out.println(Integer.toHexString(i * 4) + "(" + (i * 4)
                           + ")" + ":  " + Integer.toHexString(memory[i]));
      }
    }
  }

  /**
   * This method reads the file as asequence of integers, where each encodes
   * one instruction (opcode and parameters) and stores this in the virtual
   * RAM.
   *
   * @param offset
   *            specifies the memory offset to place the program
   * @param filename
   *            specifies the file where to read the 'byte'-code
   */
  void loadProgramfile(int offset, String filename) {
    System.out.println("\nStart loading " + filename + " at " + offset
                       + " ... ");
    try {
      File file = new File(filename);
      programLength = (int) file.length() / 4;
      if (this.memory.length < offset + programLength) {
        System.out.flush();
        System.err
            .println("Virtual machine memory is too small ... exiting");
        System.exit( -1);
      }
      DataInputStream dataStream = new DataInputStream(
          new FileInputStream(file));
      for (int i = 0; i < programLength; i++) {
        memory[offset + i] = dataStream.readInt();
      }
    }
    catch (FileNotFoundException e) {
      System.err.println("File not found ... exiting");
      e.printStackTrace();
      System.exit( -1);
    }
    catch (EOFException e) {
      System.err.println("End of program file reached ... exiting");
      e.printStackTrace();
      System.exit( -1);
    }
    catch (IOException e) {
      System.err.println("I/O error ... exiting");
      e.printStackTrace();
      System.exit( -1);
    }
    System.out.println(" ... loading finished.");
  }

  void setupRegAndMem(String[] args) {
    int arrayIndex =0;
    int memForPar = MAX_STRING_LENGTH * args.length + 4 + 4 + 8; //space for
    // number of arguments, referenz to string array,stringArray, FP, R31
    cpuRegister[30] = memory.length * 4 - memForPar; //base for
    // FramePointer
    cpuRegister[29] = programLength * 4
        + (cpuRegister[30] - programLength * 4) / 3;
    cpuRegister[29] = cpuRegister[29] - (cpuRegister[29] % 4); //global
    // area
    heapBase = programLength * 4 + (cpuRegister[30] - programLength * 4)
        / 3 * 2;
    heapBase = heapBase - (heapBase % 4);
    this.heap = new Heap(cpuRegister[29], heapBase);

    arrayIndex = (cpuRegister[30] + 8) / 4;
    memory[arrayIndex] = args.length; //number of parameters
    arrayIndex++;

    memory[arrayIndex] = (arrayIndex+1)*4; //address stringArray
    arrayIndex++;


    // now copy the parameter strings onto the callers parameter stack
    for (int i = 0; i < args.length; i++) {
      if (args[i].length() < MAX_STRING_LENGTH - 1) {
        stringToMemory(args[i], arrayIndex);
        arrayIndex = arrayIndex + 10;
      }
      else {
        System.err.println("passed parameter " + i + " is too long, "
                           + args[i].length() + " where only " +
                           MAX_STRING_LENGTH
                           + " is allowed");
        System.exit( -1);
      }
    }
  }

  /**
   * Writes a string recoded in bytes to the memory, starting at the specified
   * memory position.
   *
   * @param data
   * @param startIndex
   */
  void stringToMemory(String data, int startIndex) {
    byte[] str = new byte[MAX_STRING_LENGTH];
    System.arraycopy(data.getBytes(), 0, str, 0, data.length());
    for (int i = 0; i < str.length; i += 4) {
      int word = ( (toUnsignedInt(str[i])) & 0x000000FF);
      word |= ( (toUnsignedInt(str[i + 1]) << 8) & 0x0000FF00);
      word |= ( (toUnsignedInt(str[i + 2]) << 16) & 0x00FF0000);
      word |= ( (toUnsignedInt(str[i + 3]) << 24) & 0xFF000000);

      memory[startIndex] = word;
      startIndex++;
    }
  }

  /**
   * Reads a string out of the memory by extracting bytes and instantiating a
   * String from these until the first zero.
   *
   * @param startIndex
   *            is the index in the memory array
   * @return a new String object containig the bytes in memory
   */
  String stringFromMemory(int startIndex) {
    byte[] chars = new byte[MAX_STRING_LENGTH];
    for (int i = 0; i < MAX_STRING_LENGTH / 4; i++) {
      int mc = memory[startIndex + i];
      chars[i * 4] = toSignedByte(mc & 0xFF);
      chars[i * 4 + 1] = toSignedByte( (mc >> 8) & 0xFF);
      chars[i * 4 + 2] = toSignedByte( (mc >> 16) & 0xFF);
      chars[i * 4 + 3] = toSignedByte( (mc >> 24) & 0xFF);
    }
    int j = 0;
    while (j < chars.length)  {
      if(chars[j] == 0){
        break;
      }
     j++;
    }

    return new String(chars, 0, j);
  }

  byte toSignedByte(int value) {
    return (byte) ( (value & 0x7F) - (value >= 128 ? 128 : 0));
  }

  int toUnsignedInt(byte value) {
    return (value & 0x7F) + (value < 0 ? 128 : 0);
  }
}