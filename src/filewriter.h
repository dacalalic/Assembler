#ifndef _FILE_WRITER_H_
#define _FILE_WRITER_H_

#include <vector>
#include <string>
#include <fstream>
#include "symbolhandler.h"

class FileWriter {

private:

    std::vector<unsigned char> text;
    std::vector<unsigned char> data;

    std::ofstream file;

    std::vector<SymbolHandler::STEntry> st;
    std::vector<SymbolHandler::RTEntry> rt;
    std::vector<SymbolHandler::RATEntry> rat;

    bool isRelPresent(std::string section);
    bool isRelaPresent(std::string section);
    int getSectionSize(std::string section);
    int numOfSecRelocs(std::string section);
    int numOfSecRelas(std::string section);

public:

    void appendToText(std::vector<unsigned char> content);
    void appendToData(std::vector<unsigned char> content);

    void changeText(short increment, int offset);
    void changeData(short increment, int offset);

    void generateELF();

    void setFile(std::string fileName);
    void setST(std::vector<SymbolHandler::STEntry>);
    void setRT(std::vector<SymbolHandler::RTEntry>);
    void setRAT(std::vector<SymbolHandler::RATEntry>);

};

#endif