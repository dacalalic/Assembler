#include "instructioncoder.h"
#include "instruction.h"
#include "symbolhandler.h"
#include "filewriter.h"
#include <iostream>
#include <iomanip>

InstructionCoder::InstructionCoder(SymbolHandler* csh, FileWriter* cfw) {
    sh = csh;
    fw = cfw;
}

void InstructionCoder::codeInstruction(Instruction *instr) {
    std::vector<unsigned char> instruction;
    instruction.push_back(instr->mnemonic << 3);
    instruction[0] += (instr->sizeOfOperands << 2);

    if (instr->mnemonic > 2) {

        appendOperand(0, instruction, instr);

        if (instr->mnemonic > 10) {

            appendOperand(1, instruction, instr);
        }
    }

    fw->appendToText(instruction);
    sh->increseLocCounter(instruction.size());
    
}

void InstructionCoder::appendOperand(int operNum, std::vector<unsigned char> &instruction, Instruction *instr) {
    instruction.push_back(instr->addrTypes[operNum] << 5);
    if (instr->addrTypes[operNum] < AddressingType::MEM && instr->addrTypes[operNum] > AddressingType::IMME) {
        std::string regNum;
        regNum.push_back(instr->operands[operNum][1]);
        int reg;
        if(regNum == "c") {
            reg = 7;
        }
        else if(regNum == "p") {
            reg = 6;
        }
        else {
            reg = std::stoi(regNum);
        }

        instruction[instruction.size() - 1] += reg << 1;
        if(instr->operands[operNum][instr->operands[operNum].size() - 1] == 'h') {
            instruction[instruction.size() - 1] += 1;
        }
    }
    if (instr->addrTypes[operNum] == AddressingType::REGDISP || instr->addrTypes[operNum] == AddressingType::IMME 
        || instr->addrTypes[operNum] == AddressingType::MEM) {

        short value = 0;
        if (instr->addrTypes[operNum] == AddressingType::REGDISP) {
            if (instr->isOpLit[operNum] == true) {
                value = std::stoi(instr->disp[operNum]);
            }
            else {
                bool isRelative = instr->operands[operNum][1] == '7' || instr->operands[operNum][1] == 'c';
                value = sh->resolveInstrSymbol(instr->disp[operNum], isRelative, instruction.size());
            }
        }
        else {
            if (instr->isOpLit[operNum] == true) {
                value = std::stoi(instr->operands[operNum]);
            }
            else {
                value = sh->resolveInstrSymbol(instr->operands[operNum], false, instruction.size());
            }
        }
        if(instr->addrTypes[operNum] == AddressingType::IMME && instr->sizeOfOperands == 0) {
            instruction.push_back(value);
        }
        else {
            instruction.push_back(value);
            instruction.push_back(value >> 8);
        }
    }
}