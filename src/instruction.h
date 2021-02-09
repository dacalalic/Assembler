#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

#include <string>

enum InstrCode { HALT, IRET, RET, INT, CALL, JMP, JEQ, JNE, JGT, PUSH, POP,
                    XCHG, MOV, ADD, SUB, MUL, DIV, CMP, NOT, AND, OR, XOR, TEST, SHL, SHR };

enum AddressingType{ IMME, REGDIR, REGIND, REGDISP, MEM };

struct Instruction {
    InstrCode mnemonic;
    int sizeOfOperands;
    std::string operands[2];
    bool isOpLit[2];
    AddressingType addrTypes[2];
    std::string disp[2];
};

#endif