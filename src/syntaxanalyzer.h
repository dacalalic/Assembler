#ifndef _SYNTAX_ANALYZER_H_
#define _SYNTAX_ANALYZER_H_

#include <string>
#include "instruction.h"

class InstructionCoder;
class Directive;
class SymbolHandler;
class DirectiveHandler;
class FileWriter;

class SyntaxAnalyzer {

private:

    Instruction* instr;
    Directive* dir;
    InstructionCoder* instrCoder;
    SymbolHandler* sh;
    DirectiveHandler* dh;
    FileWriter* fw;

    bool isLastJmp;
    bool commaBefore;
    bool aoLast;
    int lineNum;
    int numOfOperands;
    std::string section;

    InstrCode getMnemonicCode(std::string instr);
    void checkOperand(AddressingType addrType, bool isJmp);
    void checkInstrOperands(bool isJmp);
    void checkDirOperands(bool isLiteral);

    void setInstrOperand(std::string oper, bool isLiteral, AddressingType addrType);

public:

    SyntaxAnalyzer();
    ~SyntaxAnalyzer();

    bool isLastInstrJmp();

    void processLabel(std::string label);
    void processInstr(std::string instruction);
    void processDir(std::string directive);
    void processImmediateLiteral(std::string literal, bool isJmp);
    void processImmediateSymbol(std::string symbol, bool isJmp);
    void processRegDirect(std::string reg, bool isJmp);
    void processRegIndirect(std::string reg, bool isJmp);
    void processRegIndDispLit(std::string reg, std::string literal, bool isJmp);
    void processRegIndDispSym(std::string reg, std::string symbol, bool isJmp);
    void processMemLit(std::string literal, bool isJmp);
    void processMemSym(std::string symbol, bool isJmp);
    void processSection(std::string section);
    void endOfLine();
    void endOfFile();
    void unknownChar(std::string ch);
    void comma();
    void arithmeticOp(std::string op);

    void setOutFile(std::string file);

};

#endif