#ifndef _SYMBOL_HANDLER_H_
#define _SYMBOL_HANDLER_H_

#include <vector>
#include <string>

class FileWriter;
class Directive;

class SymbolHandler {

private:

    struct FREntry {

        std::string section;
        int offset;
        int next;
        bool isRel;
    };

    std::vector<FREntry> frTable;
    std::vector<Directive*> equDirs;

    int locationCounter = 0;
    std::string currSection;
    FileWriter* fw;

    void fixEqus();
    void unfurlEqus(Directive* newDir, Directive* oldDir, bool negate, int &opNum);

public:

    struct STEntry {

        std::string name;
        int value;
        int size;
        char type;
        char binding;
        std::string section;
        int frIndex;
    };

    struct RTEntry {

        std::string section;
        int offset;
        int type;
        std::string symbolIndex;
    };

    struct RATEntry {
        std::string section;
        int offset;
        int type;
        std::string symbolIndex;
        int addend;
    };

    std::vector<STEntry> symbolTable;
    std::vector<RTEntry> relocationTable;
    std::vector<RATEntry> relaTable;

    SymbolHandler(FileWriter* cfw);

    void handleLabel(std::string label);
    int resolveInstrSymbol(std::string symbol, bool isRel, int lcOffset);
    void handleGlobalSymbol(std::string symbol, bool isExtern);
    int getSymbolIndex(std::string symbol);
    std::vector<SymbolHandler::STEntry> getSymbolTable();

    void backpatch();

    void increseLocCounter(int increment);

    void setCurrentSection(std::string section);

    void putEqu(Directive* dir);

};

#endif