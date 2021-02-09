#include "syntaxanalyzer.h"
#include "directive.h"
#include "instructioncoder.h"
#include "symbolhandler.h"
#include "filewriter.h"
#include "directivehandler.h"
#include <iostream>
#include <cstdlib>

SyntaxAnalyzer::SyntaxAnalyzer() {
    instr = nullptr;
    dir = nullptr;

    fw = new FileWriter();
    sh = new SymbolHandler(fw);
    instrCoder = new InstructionCoder(sh, fw);
    dh = new DirectiveHandler(sh, fw);

    isLastJmp = false;
    aoLast = false;
    commaBefore = false;
    lineNum = 0;
    numOfOperands = 0;
}

SyntaxAnalyzer::~SyntaxAnalyzer() {
    delete fw;
    delete sh;
    delete instrCoder;
    delete dh;
}

bool SyntaxAnalyzer::isLastInstrJmp() {
    return isLastJmp;
}

void SyntaxAnalyzer::processLabel(std::string label) {
    sh->handleLabel(label);
}

void SyntaxAnalyzer::processInstr(std::string instruction) {

    if(instr != nullptr || dir != nullptr) {
        std::cout << "Syntax error: Multiple directives and instructions on line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }
    if(section != ".text") {
        std::cout << "Syntax error: Instructions need to be inside .text section " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }

    instr = new Instruction();

    if(instruction[instruction.size() - 1] == 'b') {
        instr->sizeOfOperands = 0;
        instruction.erase(instruction.end() - 1, instruction.end());
    }
    else {
        instr->sizeOfOperands = 1;
        if(instruction[instruction.size() - 1] == 'w') {
            instruction.erase(instruction.end() - 1, instruction.end());
        }
    }
    instr->mnemonic = getMnemonicCode(instruction);

    if(instr->mnemonic < 9 && instr->mnemonic > 2) {
        isLastJmp = true;
    }
    else {
        isLastJmp = false;
    }
}

void SyntaxAnalyzer::processDir(std::string directive) {

    isLastJmp = false;

    if(instr != nullptr || dir != nullptr) {
        std::cout << "Syntax error: Multiple directives and instructions on line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }
    if((directive == ".byte" || directive == ".word") && section != ".data") {
        std::cout << "Syntax error: Directives .byte and .word need to be inside .data section " << lineNum + 1 << "!" << std::endl;
    }
    if(directive == ".skip" && section == ".text") {
        std::cout << "Syntax error: Directive .skip can't be in .text section " << lineNum + 1 << "!" << std::endl;
    }

    dir = new Directive();
    dir->mnemonic = directive;
}

void SyntaxAnalyzer::processImmediateLiteral(std::string literal, bool isJmp) {

    if(instr != nullptr && (instr->mnemonic == 11 || instr->mnemonic == 10 || 
    (numOfOperands == 1 && (instr->mnemonic > 11 && instr->mnemonic < 17 || instr->mnemonic > 17 && instr->mnemonic < 22 || instr->mnemonic > 22)))) {
        std::cout << "Immediate addressing can't be used for destination operand! Line " << lineNum  + 1 << "." << std::endl;
        exit(1);
    }

    checkOperand(AddressingType::IMME, isJmp);
    setInstrOperand(literal, true, AddressingType::IMME);
}

void SyntaxAnalyzer::processImmediateSymbol(std::string symbol, bool isJmp) {

    if(instr != nullptr && (instr->mnemonic == 11 || instr->mnemonic == 10 || 
    (numOfOperands == 1 && (instr->mnemonic > 11 && instr->mnemonic < 17 || instr->mnemonic > 17 && instr->mnemonic < 22 || instr->mnemonic > 22)))) {
        std::cout << "Immediate addressing can't be used for destination operand! Line " << lineNum + 1 << "." << std::endl;
        exit(1);
    }

    checkOperand(AddressingType::IMME, isJmp);
    setInstrOperand(symbol, false, AddressingType::IMME);
}

void SyntaxAnalyzer::processRegDirect(std::string reg, bool isJmp) {

    checkOperand(AddressingType::REGDIR, isJmp);
    setInstrOperand(reg, false, AddressingType::REGDIR);
}

void SyntaxAnalyzer::processRegIndirect(std::string reg, bool isJmp) {

    checkOperand(AddressingType::REGIND, isJmp);
    setInstrOperand(reg, false, AddressingType::REGIND);
}

void SyntaxAnalyzer::processRegIndDispLit(std::string reg ,std::string literal, bool isJmp) {

    checkOperand(AddressingType::REGDISP, isJmp);
    setInstrOperand(reg, true, AddressingType::REGDISP);

    instr->disp[numOfOperands - 1] = literal;
}

void SyntaxAnalyzer::processRegIndDispSym(std::string reg, std::string symbol, bool isJmp) {

    checkOperand(AddressingType::REGDISP, isJmp);
    setInstrOperand(reg, false, AddressingType::REGDISP);

    instr->disp[numOfOperands - 1] = symbol;
}

void SyntaxAnalyzer::processMemLit(std::string literal, bool isJmp) {

    checkOperand(AddressingType::MEM, isJmp);

    if(instr != nullptr) {
        setInstrOperand(literal, true, AddressingType::MEM);
    }
    else if(dir != nullptr) {
        checkDirOperands(true);
        dir->literalList.push_back(std::pair<int, std::string>(numOfOperands++, literal));
    }
}

void SyntaxAnalyzer::processMemSym(std::string symbol, bool isJmp) {

    checkOperand(AddressingType::MEM, isJmp);

    if(instr != nullptr) {
        setInstrOperand(symbol, false, AddressingType::MEM);
    }
    else if(dir != nullptr) {
        checkDirOperands(false);
        dir->symbolList.push_back(std::pair<int, std::string>(numOfOperands++, symbol));
    }
}

void SyntaxAnalyzer::processSection(std::string section) {
    this->section = section;
    sh->setCurrentSection(section);
}

void SyntaxAnalyzer::endOfLine() {
    if(commaBefore) {
        unknownChar(",");
    }
    if(aoLast) {
        std::cout << "Bad expression at line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }

    lineNum++;
    if(instr != nullptr) {
        if(instr->mnemonic > 2 && numOfOperands == 0 || instr->mnemonic > 10 && numOfOperands < 2) {
            std::cout << "Not enough operands for instruction at line " << lineNum + 1 << "!" << std::endl;
            exit(1);
        }
        instrCoder->codeInstruction(instr);
    }
    else if(dir != nullptr) {
        if(numOfOperands == 0 || (numOfOperands == 1 && dir->mnemonic == ".equ")) {
            std::cout << "Not enough operands for directive at line " << lineNum + 1 << "!" << std::endl;
        }
        dh->handleDirective(dir);
    }

    delete instr;
    if(dir && dir->mnemonic != ".equ") {
        delete dir;
    }

    instr = nullptr;
    dir = nullptr;
    numOfOperands = 0;
}

void SyntaxAnalyzer::endOfFile() {

    sh->backpatch();

}

void SyntaxAnalyzer::unknownChar(std::string ch) {
    std::cout << "Unknown character " << ch << " at line " << lineNum + 1 << "!" << std::endl;
    exit(1);
}

void SyntaxAnalyzer::comma() {
    if(commaBefore || numOfOperands == 0) {
        unknownChar(",");
    }
    commaBefore = true;
}

void SyntaxAnalyzer::arithmeticOp(std::string op) {
    if(dir == nullptr || dir->mnemonic != ".equ" || numOfOperands == 0) {
        unknownChar(op);
    }

    aoLast = true;
    if(op == "+") {
        dir->expresionOps.push_back(true);
    }
    else {
        dir->expresionOps.push_back(false);
    }
}

void SyntaxAnalyzer::setOutFile(std::string file) {
    fw->setFile(file);
}

InstrCode SyntaxAnalyzer::getMnemonicCode(std::string instr){
    if(instr == "HALT") 
        return InstrCode::HALT;
    if(instr == "IRET") 
        return InstrCode::IRET;
    if(instr == "RET") 
        return InstrCode::RET;
    if(instr == "INT") 
        return InstrCode::INT;
    if(instr == "CALL") 
        return InstrCode::CALL;
    if(instr == "JMP") 
        return InstrCode::JMP;
    if(instr == "JEQ") 
        return InstrCode::JEQ;
    if(instr == "JNE") 
        return InstrCode::JNE;
    if(instr == "JGT") 
        return InstrCode::JGT;
    if(instr == "PUSH") 
        return InstrCode::PUSH;
    if(instr == "POP") 
        return InstrCode::POP;
    if(instr == "XCHG") 
        return InstrCode::XCHG;
    if(instr == "MOV") 
        return InstrCode::MOV;
    if(instr == "ADD") 
        return InstrCode::ADD;
    if(instr == "SUB") 
        return InstrCode::SUB;
    if(instr == "MUL") 
        return InstrCode::MUL;
    if(instr == "DIV") 
        return InstrCode::DIV;
    if(instr == "CMP") 
        return InstrCode::CMP;
    if(instr == "NOT") 
        return InstrCode::NOT;
    if(instr == "AND") 
        return InstrCode::AND;
    if(instr == "OR") 
        return InstrCode::OR;
    if(instr == "XOR") 
        return InstrCode::XOR;
    if(instr == "TEST") 
        return InstrCode::TEST;
    if(instr == "SHL") 
        return InstrCode::SHL;
    else
        return InstrCode::SHR;
}

void SyntaxAnalyzer::checkOperand(AddressingType addrType, bool isJmp) {
    if(((dir == nullptr || dir != nullptr && dir->mnemonic != ".equ") && numOfOperands > 0 && !commaBefore) ||
        (dir != nullptr && dir->mnemonic == ".equ" && numOfOperands == 1 && !commaBefore)) {

        std::cout << "Syntax error: Put comma between operands. Line " << lineNum + 1 << "!" << std::endl;
        exit(1); 
    }
    if(instr != nullptr) {
        checkInstrOperands(isJmp);
    }
    else if( dir != nullptr) {
        if(addrType != AddressingType::MEM) {
            std::cout << "Syntax error: Bad symbol or literal format on line " << lineNum + 1 << "!" << std::endl;
            exit(1);
        }
    }
    else {
        std::cout << "Syntax error: Operand without instruction or directive on line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }
    commaBefore = false;
    aoLast = false;
}

void SyntaxAnalyzer::checkInstrOperands(bool isJmp) {
    if(instr->mnemonic < 3 || (instr->mnemonic < 11 && numOfOperands == 1) || numOfOperands == 2) {
        std::cout << "Syntax error: Too many operands for instruction on line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }
    if(isJmp != isLastJmp) {
        std::cout << "Syntax error: Bad operand format for instruction on line " << lineNum + 1 << "!" << std::endl;
        exit(1);
    }
}

void SyntaxAnalyzer::checkDirOperands(bool isLiteral) {
    if(dir->mnemonic == ".skip" && !isLiteral) {
        std::cout << "Syntax error: .skip directive requires literal!" << std::endl;
        exit(1);
    }
    else if(dir->mnemonic == ".skip" && numOfOperands == 1) {
        std::cout << "Syntax error: too many operand for .skip directive!" << std::endl;
        exit(1);
    }
    else if((dir->mnemonic == ".global" || dir->mnemonic == ".extern") && isLiteral) {
        std::cout << "Syntax error: .extern and .global directives require symbol!" << std::endl;
        exit(1);
    }
}

void SyntaxAnalyzer::setInstrOperand(std::string oper, bool isLiteral, AddressingType addrType) {
    instr->operands[numOfOperands] = oper;
    instr->isOpLit[numOfOperands] = isLiteral;
    instr->addrTypes[numOfOperands++] = addrType;
}