#include "directivehandler.h"
#include "directive.h"
#include "symbolhandler.h"
#include "filewriter.h"
#include <iostream>

DirectiveHandler::DirectiveHandler(SymbolHandler* csh, FileWriter* cfw) {
    sh = csh;
    fw = cfw;
}

void DirectiveHandler::handleDirective(Directive* dir) {
    
    if(dir->mnemonic == ".global") {
        for(int i = 0; i < dir->symbolList.size(); i++) {
            sh->handleGlobalSymbol(dir->symbolList[i].second, false);
        }
    }
    else if(dir->mnemonic == ".extern") {
        for(int i = 0; i < dir->symbolList.size(); i++) {
            sh->handleGlobalSymbol(dir->symbolList[i].second, true);
        }
    }
    else if(dir->mnemonic == ".byte") {
        int symInd = 0;
        int litInd = 0;
        std::vector<unsigned char> mem;

        while(litInd < dir->literalList.size() && symInd < dir->symbolList.size()) {
            if(dir->literalList[litInd].first < dir->symbolList[symInd].first) {
                mem.push_back(std::stoi(dir->literalList[litInd++].second));
            }
            else {
                mem.push_back((unsigned char)sh->resolveInstrSymbol(dir->symbolList[symInd++].second, false, 0));
            }
            sh->increseLocCounter(1);
        }

        while(litInd < dir->literalList.size()) {
            mem.push_back(std::stoi(dir->literalList[litInd++].second));
            sh->increseLocCounter(1);
        }
        while(symInd < dir->symbolList.size()) {
            mem.push_back((unsigned char)sh->resolveInstrSymbol(dir->symbolList[symInd++].second, false, 0));
            sh->increseLocCounter(1);
        }
        fw->appendToData(mem);
    }
    else if(dir->mnemonic == ".word") {

        int symInd = 0;
        int litInd = 0;
        std::vector<unsigned char> mem;


        while(litInd < dir->literalList.size() && symInd < dir->symbolList.size()) {
            if(dir->literalList[litInd].first < dir->symbolList[symInd].first) {
                unsigned int lit = std::stoi(dir->literalList[litInd++].second);
                mem.push_back(lit);
                mem.push_back(lit >> 8);
            }
            else {
                int symVal = sh->resolveInstrSymbol(dir->symbolList[symInd++].second, false, 0);
                mem.push_back(symVal);
                mem.push_back(symVal >> 8);
            }
            sh->increseLocCounter(2);
            std::cout << litInd << " " << dir->literalList.size() << std::endl;
        }

        while(litInd < dir->literalList.size()) {
            unsigned int lit = std::stoi(dir->literalList[litInd++].second);
            mem.push_back(lit);
            mem.push_back(lit >> 8);
            sh->increseLocCounter(2);
        }
        while(symInd < dir->symbolList.size()) {
            int symVal = sh->resolveInstrSymbol(dir->symbolList[symInd++].second, false, 0);
            mem.push_back(symVal);
            mem.push_back(symVal >> 8);
            sh->increseLocCounter(2);
        }
        
        fw->appendToData(mem);
    }
    else if(dir->mnemonic == ".skip") {
        sh->increseLocCounter(std::stoi(dir->literalList[0].second));
    }
    else if(dir->mnemonic == ".equ") {

        sh->putEqu(dir);
    }
}