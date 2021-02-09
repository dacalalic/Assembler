#ifndef _INSTR_CODER_H_
#define _INSTR_CODER_H_

#include <vector>

class Instruction;
class SymbolHandler;
class FileWriter;

class InstructionCoder {

private:

    SymbolHandler* sh;
    FileWriter* fw;
    void appendOperand(int operNum, std::vector<unsigned char> &instruction, Instruction* instr);

public:

    InstructionCoder(SymbolHandler* csh, FileWriter* cfw);

    void codeInstruction(Instruction* instr);

};

#endif