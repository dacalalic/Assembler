#include "symbolhandler.h"
#include "filewriter.h"
#include "directive.h"
#include <iostream>

SymbolHandler::SymbolHandler(FileWriter* cfw) {
    STEntry ste;
    ste.name = "UND";
    ste.value = 0;
    ste.size = 0;
    ste.type = 0;
    ste.binding = 0;
    ste.section = "UND";
    ste.frIndex = 0;

    symbolTable.push_back(ste);

    FREntry fre;
    frTable.push_back(fre);

    fw = cfw;
}

void SymbolHandler::handleLabel(std::string label) {
    int index = getSymbolIndex(label);

    if(index != 0) {
        
        symbolTable[index].value = locationCounter;
        symbolTable[index].section = currSection;
        symbolTable[index].type = 1;
    }
    else {
        STEntry ste;
        ste.name = label;
        ste.value = locationCounter;
        ste.size = 0;
        ste.type = 1;
        ste.binding = 0;
        ste.section = currSection;
        ste.frIndex = 0;

        symbolTable.push_back(ste);

        index = symbolTable.size() - 1;
    }

}

int SymbolHandler::resolveInstrSymbol(std::string symbol, bool isRel, int lcOffset){

    int index = getSymbolIndex(symbol);

    if(index == 0) {
        STEntry ste;
        ste.name = symbol;
        ste.value = 0;
        ste.size = 0;
        ste.type = 1;
        ste.binding = 0;
        ste.section = "UND";
        ste.frIndex = 0;

        symbolTable.push_back(ste);

        index = symbolTable.size() - 1;
    }

    FREntry frEntry;
    frEntry.section = currSection;
    frEntry.offset = locationCounter + lcOffset;
    frEntry.next = symbolTable[index].frIndex;
    frEntry.isRel = isRel;

    frTable.push_back(frEntry);
    symbolTable[index].frIndex = frTable.size() - 1;

    //Absolut
    if(!isRel) {
        return 0;
    }
    //PC relative
    else {
        return -2;
    }

    return 0;
}

void SymbolHandler::handleGlobalSymbol(std::string symbol, bool isExtern) {

    int index = getSymbolIndex(symbol);

    if(index == 0) {
        STEntry ste;
        ste.name = symbol;
        ste.value = 0;
        ste.size = 0;
        ste.type = 1;
        ste.binding = 0;
        ste.section = "UND";
        ste.frIndex = 0;

        symbolTable.push_back(ste);

        index = symbolTable.size() - 1;
    }

    symbolTable[index].binding = 1 + isExtern;

}

int SymbolHandler::getSymbolIndex(std::string symbol) {

    for(int i = 1; i < symbolTable.size(); i++) {
        if(symbolTable[i].name == symbol) {
            return i;
        }
    }

    return 0;
}

void SymbolHandler::backpatch() {

    fixEqus();

    int currSectionInd = getSymbolIndex(currSection);
    symbolTable[currSectionInd].size = locationCounter;

    for(STEntry& ste : symbolTable) {

        if(ste.binding == 0) {
            int frIndex = ste.frIndex;

            while(frIndex != 0) {

                if(frTable[frIndex].section == ".text") {
                    fw->changeText(ste.value, frTable[frIndex].offset);
                }
                else {
                    fw->changeData(ste.value, frTable[frIndex].offset);
                }
                frIndex = frTable[frIndex].next;
            }
        }

        int frIndex = ste.frIndex;

        if(ste.binding != 0 || ste.section != "UND") {
            while(frIndex != 0) {
                if(frTable[frIndex].isRel && frTable[frIndex].section == ste.section && ste.binding == 0) {

                    if(frTable[frIndex].section == ".text") {
                        fw->changeText(-frTable[frIndex].offset, frTable[frIndex].offset);
                    }
                    else {
                        fw->changeData(-frTable[frIndex].offset, frTable[frIndex].offset);
                    }
                }
                else if(ste.section != "ABS" || ste.binding != 0) {
                    RTEntry rte;
                    rte.offset = frTable[frIndex].offset;
                    rte.section = frTable[frIndex].section;
                    rte.type = frTable[frIndex].isRel;
                    if(ste.binding == 0) {
                        rte.symbolIndex = ste.section;
                    }
                    else {
                        rte.symbolIndex = ste.name;
                    }
                    relocationTable.push_back(rte);
                }
                else if(ste.section == "ABS" && frTable[frIndex].isRel && ste.binding == 0) {
                    RTEntry rte;
                    rte.offset = frTable[frIndex].offset;
                    rte.section = frTable[frIndex].section;
                    rte.type = 1;
                    rte.symbolIndex = "UND";
                    
                    relocationTable.push_back(rte);
                }

                frIndex = frTable[frIndex].next;
            }
        }
    }

    fw->setRT(relocationTable);
    fw->setST(symbolTable);
    fw->setRAT(relaTable);
    fw->generateELF();

}

void SymbolHandler::increseLocCounter(int increment) {
    locationCounter += increment;
}

void SymbolHandler::setCurrentSection(std::string section) {

    int index;

    if(currSection.size() > 0) {
        index = getSymbolIndex(currSection);
        symbolTable[index].size = locationCounter;    
    }

    currSection = section;

    index = getSymbolIndex(section);

    if(index == 0) {
        STEntry ste;
        ste.name = section;
        ste.value = 0;
        ste.size = 0;
        ste.type = 0;
        ste.binding = 0;
        ste.section = section;
        ste.frIndex = 0;

        symbolTable.push_back(ste);

        locationCounter = 0;
    }
    else {
        locationCounter = symbolTable[index].size;
    }

}

std::vector<SymbolHandler::STEntry> SymbolHandler::getSymbolTable() {

    return symbolTable;

}

void SymbolHandler::putEqu(Directive* dir) {
    equDirs.push_back(dir);
}

void SymbolHandler::fixEqus() {

    //unfurl symbols
    for(int i = 0; i < equDirs.size(); i++) {
        int opNum = 1;
        Directive* newDir = new Directive();
        newDir->mnemonic = equDirs[i]->mnemonic;
        newDir->symbolList.push_back(equDirs[i]->symbolList[0]);

        unfurlEqus(newDir, equDirs[i], false, opNum);

        delete equDirs[i];
        equDirs[i] = newDir;
    }

    //Check correctness
    std::vector<std::string> sections;
    std::vector<std::string> externSymbols;

    for(int i = 0; i < equDirs.size(); i++) {
        int classIndexText = 0;
        int classIndexData = 0;
        int classIndexBss = 0;
        int classIndexExtern = 0;

        for(int j = 1; j < equDirs[i]->symbolList.size(); j++) {
            int opIndex = equDirs[i]->symbolList[j].first - 2;
            bool op;

            if(opIndex == -1) {
                op = true;
            }
            else {
                op = equDirs[i]->expresionOps[opIndex];
            }

            int index = getSymbolIndex(equDirs[i]->symbolList[j].second);

            if(symbolTable[index].binding == 2) {
                classIndexExtern++;
                externSymbols.push_back(symbolTable[index].name);
            }
            else {
                if(symbolTable[index].section == ".text") {                    
                    if(op) {
                        classIndexText++;
                    }
                    else {
                        classIndexText--;
                    }
                }
                else if(symbolTable[index].section == ".data") {
                    if(op) {
                        classIndexData++;
                    }
                    else {
                        classIndexData--;
                    }
                }
                else if(symbolTable[index].section == ".bss") {
                    if(op) {
                        classIndexBss++;
                    }
                    else {
                        classIndexBss--;
                    }
                }
            }
        }

        int sum = classIndexBss + classIndexData + classIndexText + classIndexExtern;

        if(sum > 1 || classIndexText < 0 || classIndexData < 0 || classIndexBss < 0) {
            std::cout << "Bad expression for symbol " << equDirs[i]->symbolList[0].second << "!" << std::endl;
            exit(1);
        }

        if(classIndexText > 0) {
            sections.push_back(".text");
        }
        else if(classIndexData > 0) {
            sections.push_back(".data");
        }
        else if(classIndexBss > 0) {
            sections.push_back(".bss");
        }
        else if(classIndexExtern > 0) {
            sections.push_back("UND");
        }
        else {
            sections.push_back("ABS");
        }
    }

    //Resolve symbols
    for(int i = 0; i < equDirs.size(); i++) {
        int value;
        int l = 0;
        int s = 1;
        if(equDirs[i]->literalList.size() > 0 && equDirs[i]->literalList[l].first == 1) {
            value = std::stoi(equDirs[i]->literalList[l++].second);
        }
        else {
            int index = getSymbolIndex(equDirs[i]->symbolList[s++].second);
            value = symbolTable[index].value;
        }

        while(s < equDirs[i]->symbolList.size()) {
            int opNum = equDirs[i]->symbolList[s].first - 2;
            bool op = equDirs[i]->expresionOps[opNum];

            int index = getSymbolIndex(equDirs[i]->symbolList[s++].second);

            if(op) {
                value += symbolTable[index].value;
            }
            else {
                value -= symbolTable[index].value;
            }
        }

        while(l < equDirs[i]->literalList.size()) {
            int opNum = equDirs[i]->literalList[l].first - 2;
            bool op = equDirs[i]->expresionOps[opNum];

            if(op) {
                value += std::stoi(equDirs[i]->literalList[l++].second);
            }
            else {
                value -= std::stoi(equDirs[i]->literalList[l++].second);
            }
        }

        int symbolIndex = getSymbolIndex(equDirs[i]->symbolList[0].second);

        if(symbolIndex == 0) {
            STEntry ste;
            ste.name = equDirs[i]->symbolList[0].second;
            ste.value = value;
            ste.size = 0;
            ste.type = 1;
            ste.binding = 0;
            ste.section = "";
            ste.frIndex = 0;

            symbolTable.push_back(ste);
            symbolIndex = symbolTable.size() - 1;
        }
        else {
            symbolTable[symbolIndex].value = value;
        }

        symbolTable[symbolIndex].section = sections[i];
    }

    //Deal with relocations
    int externCounter = 0;
    for(int i = 0; i < equDirs.size(); i++) {

        int symbolIndex = getSymbolIndex(equDirs[i]->symbolList[0].second);

        if(symbolTable[symbolIndex].section == "UND" && symbolTable[symbolIndex].binding == 0) {

            int externSymbolIndex = getSymbolIndex(externSymbols[externCounter++]);

            int frIndex = symbolTable[symbolIndex].frIndex;
            
            if(symbolTable[externSymbolIndex].frIndex == 0) {
                symbolTable[externSymbolIndex].frIndex = frIndex;
            }
            else {
                int externFrIndex = symbolTable[externSymbolIndex].frIndex;
                while(frTable[externFrIndex].next != 0) {
                    externFrIndex = frTable[externFrIndex].next;
                }
                frTable[externFrIndex].next = frIndex;
            }
        }

        if(symbolTable[symbolIndex].section != "ABS" && symbolTable[symbolIndex].binding != 0) {
            int frIndex = symbolTable[symbolIndex].frIndex;
            while(frIndex != 0) {
                RATEntry rate;
                rate.offset = frTable[frIndex].offset;
                rate.section = frTable[frIndex].section;
                rate.type = frTable[frIndex].isRel;
                
                if(symbolTable[symbolIndex].section == "UND") {
                    rate.symbolIndex = externSymbols[externCounter++];
                }
                else {
                    rate.symbolIndex = symbolTable[symbolIndex].section;
                }
                rate.addend = symbolTable[symbolIndex].value;

                relaTable.push_back(rate);

                frIndex = frTable[frIndex].next;
            }

            symbolTable[symbolIndex].frIndex = 0;
        }
    }

}

void SymbolHandler::unfurlEqus(Directive* newDir, Directive* oldDir, bool negate, int &opNum) {

    int i = 1;
    int j = 0;
    bool lastOp;

    if(oldDir->symbolList.size() > 1 && oldDir->symbolList[i].first == 1) {
        

        if(oldDir->symbolList[i].second == newDir->symbolList[0].second) {
            std::cout << "Circular dependency for symbol " << newDir->symbolList[0].second << "!" << std::endl;
            exit(1);
        }

        bool isInEqual = false;
        int k;
        for(k = 0; k < equDirs.size(); k++) {
            if(equDirs[k]->symbolList[0].second == oldDir->symbolList[i].second){
                isInEqual = true;
                break;
            }
        }

        if(isInEqual) {
            unfurlEqus(newDir, equDirs[k], negate, opNum);
        }
        else {
            newDir->symbolList.push_back(oldDir->symbolList[i]);
            newDir->symbolList[newDir->symbolList.size() - 1].first = opNum++;
        }
        i++;
    }
    else {
        newDir->literalList.push_back(oldDir->literalList[j++]);
        newDir->literalList[newDir->literalList.size() - 1].first = opNum++;
    }

    while(i < oldDir->symbolList.size() && j < oldDir->literalList.size()) {

        if(negate) {
            lastOp = !oldDir->expresionOps[i + j - 2];           
        }
        else {
            lastOp = oldDir->expresionOps[i + j - 2];
        }
        newDir->expresionOps.push_back(lastOp);

        if(oldDir->symbolList[i].first == i + j) {

            if(oldDir->symbolList[i].second == newDir->symbolList[0].second) {
                std::cout << "Circular dependency for symbol " << newDir->symbolList[0].second << "!" << std::endl;
                exit(1);
            }

            bool isInEqual = false;
            int k;
            for(k = 0; k < equDirs.size(); k++) {
                if(equDirs[k]->symbolList[0].second == oldDir->symbolList[i].second){
                    isInEqual = true;
                    break;
                }
            }

            if(isInEqual) {
                if(lastOp == false) {
                    unfurlEqus(newDir, equDirs[k], !negate, opNum);
                }
            }
            else {
                newDir->symbolList.push_back(oldDir->symbolList[i]);
                newDir->symbolList[newDir->symbolList.size() - 1].first = opNum++;
            }
            i++;
        }
        else {
            newDir->literalList.push_back(oldDir->literalList[j++]);
            newDir->literalList[newDir->literalList.size() - 1].first = opNum++;
        }
    }

    if(i == oldDir->symbolList.size()) {

        while(j < oldDir->literalList.size()) {

            if(negate) {
                lastOp = !oldDir->expresionOps[i + j - 2];           
            }
            else {
                lastOp = oldDir->expresionOps[i + j - 2];
            }
            newDir->expresionOps.push_back(lastOp);

            newDir->literalList.push_back(oldDir->literalList[j++]);
            newDir->literalList[newDir->literalList.size() - 1].first = opNum++;
        }
    }
    if(j == oldDir->literalList.size()) {

        while(i < oldDir->symbolList.size()) {
            if(negate) {
                lastOp = !oldDir->expresionOps[i + j - 2];           
            }
            else {
                lastOp = oldDir->expresionOps[i + j - 2];
            }
            newDir->expresionOps.push_back(lastOp);

            if(oldDir->symbolList[i].second == newDir->symbolList[0].second) {
                std::cout << "Circular dependency for symbol " << newDir->symbolList[0].second << "!" << std::endl;
                exit(1);
            }

            bool isInEqual = false;
            int k;
            for(k = 0; k < equDirs.size(); k++) {
                if(equDirs[k]->symbolList[0].second == oldDir->symbolList[i].second){
                    isInEqual = true;
                    break;
                }
            }

            if(isInEqual) {
                unfurlEqus(newDir, equDirs[k], !negate, opNum);
            }
            else {
                newDir->symbolList.push_back(oldDir->symbolList[i]);
                newDir->symbolList[newDir->symbolList.size() - 1].first = opNum++;
            }
            i++;
        }
    }

}