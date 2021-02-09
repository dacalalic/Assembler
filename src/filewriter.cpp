#include "filewriter.h"
#include <iomanip>
#include <vector>
#include <iostream>

void FileWriter::appendToText(std::vector<unsigned char> content) {
    
    text.insert(text.end(), content.begin(), content.end());
}

void FileWriter::appendToData(std::vector<unsigned char> content) {

    data.insert(data.end(), content.begin(), content.end());
    
}

void FileWriter::changeText(short increment, int offset) {

    short value = text[offset];
    value += text[offset + 1] << 8;
    value += increment;
    text[offset]  = (unsigned char) value;
    text[offset + 1]  = (unsigned char) (value >> 8);

}

void FileWriter::changeData(short increment, int offset) {
    
    short value = data[offset];
    value += data[offset + 1] << 8;
    value += increment;
    data[offset]  = (unsigned char) value;
    data[offset + 1]  = (unsigned char) (value >> 8);

}

void FileWriter::generateELF() {

    bool relTextPresent = isRelPresent(".text");
    bool relDataPresent = isRelPresent(".data");
    bool relaTextPresent = isRelaPresent(".text");
    bool relaDataPresent = isRelaPresent(".data");

    file << "Magic:                            " << "7f 45 4c 46" << std::endl;
    file << "Class:                            " << "ELF16" << std::endl;
    file << "Data:                             " << "little-endian" << std::endl;
    file << "Version:                          " << "1" << std::endl;
    file << "Type:                             " << "Relocatable" << std::endl;
    file << "Machine:                          " << "ETF16" << std::endl;
    file << "Version:                          " << "1" << std::endl;
    file << "Entry point address:              " << "0x0" << std::endl;
    file << "Start of program headers:         " << "0x0" << std::endl;
    file << "Start of section headers:         " << "64" << std::endl;
    file << "Flags:                            " << "0x0" << std::endl;
    file << "Size of this header:              " << "64" << std::endl;
    file << "Size of program headers:          " << "0" << std::endl;
    file << "Number of program headers:        " << "0" << std::endl;
    file << "Size of section headers:          " << "40" << std::endl;
    file << "Number of section headers:        " << 5 + relTextPresent + relDataPresent + relaTextPresent + relDataPresent << std::endl;
    file << "Section header string table index:" << "0" << std::endl << std::endl;
    
    file << "Section headers:" << std::endl;
    file << "Name              Type              Address           Offset  " << std::endl;
    file << "Size              EntSize           Flags Link Info   Align   " << std::endl;

    file << "0                 NULL              0                 0       " << std::endl;
    file << "0                 0                       0    0      0       " << std::endl;

    file << ".text             PROGBITS          0                 0       " << std::endl;
    file << std::setw(17) << getSectionSize(".text") << " 0                 AX    0    0      1       " << std::endl;

    if(relTextPresent) {
        file << ".rel.text         REL               0                 0       " << std::endl;
        file << std::setw(17) << 16*numOfSecRelocs(".text") << " 16                I     " << 4 + relTextPresent + relDataPresent + relaTextPresent + relaDataPresent
            << "    1      1       " << std::endl;
    }

    if(relaTextPresent) {
        file << ".rela.text        RELA              0                 0       " << std::endl;
        file << std::setw(17) << 20*numOfSecRelas(".text") << " 20                I     " << 4 + relTextPresent + relDataPresent + relaTextPresent + relaDataPresent
            << "    1      1       " << std::endl;
    }

    file << ".data             PROGBITS          0                 0       " << std::endl;
    file << std::setw(17) << getSectionSize(".data") << " 0                 WA    0    0      1       " << std::endl;

    if(relDataPresent) {
        file << ".rel.data         REL               0                 0       " << std::endl;
        file << std::setw(17) << 16*numOfSecRelocs(".data") <<" 16                I     " << 4 + relDataPresent + relTextPresent + relaTextPresent + relaDataPresent
            << "    3      1       " << std::endl;
    }

    if(relaDataPresent) {
        file << ".rela.data        RELA              0                 0       " << std::endl;
        file << std::setw(17) << 20*numOfSecRelas(".data") <<" 20                I     " << 4 + relDataPresent + relTextPresent + relaTextPresent + relaDataPresent
            << "    3      1       " << std::endl;
    }

    file << ".bss              PROGBITS          0                 SHT_NOBITS" << std::endl;
    file << std::setw(17) << getSectionSize(".bss") << " 0                 WA    0    0      1       " << std::endl;

    file << ".symtab           SYMTAB            0                 0       " << std::endl;
    file << std::setw(17) << st.size()*18 <<" 18                      0    0      1       " << std::endl;

    file << std::hex << "Sections:" << std::endl;
    file << ".text" << std::endl;
    for(int i = 0; i < text.size(); i++) {
        file << std::setw(2) << std::setfill('0') << (int)text[i];
        if((i + 1)%10 == 0) {
            file << std::endl;
        }
        if(i % 2 == 1) {
            file << " ";
        }
    }
    file << std::endl;

    file << ".data" << std::endl;

    for(int i = 0; i < data.size(); i++) {
        file << std::setw(2) << std::setfill('0') << (int)data[i];
        if((i + 1)%10 == 0) {
            file << std::endl;
        }
        if(i % 2 == 1) {
            file << " ";
        }
    }
    file << std::endl;

    file << std::dec << std::setfill(' ');

    file << "Symbol table:" << std::endl;
    file << "Name                   Addr Size T L/G Section" << std::endl;

    for(SymbolHandler::STEntry& ste : st) {
        if(ste.type == 0) {
            file << std::setw(20) << ste.name << " ";
            file << std::setw(5) << ste.value << " ";
            file << std::setw(5) << ste.size << " ";
            file << (int)ste.type << "  ";
            file << (int)ste.binding << "   ";
            file << ste.section << std::endl;
        }
    }
    for(SymbolHandler::STEntry& ste : st) {
        if(ste.type != 0) {
            file << std::setw(20) << ste.name << " ";
            file << std::setw(5) << ste.value << " ";
            file << std::setw(5) << ste.size << " ";
            file << (int)ste.type << "  ";
            file << (int)ste.binding << "   ";
            file << ste.section << std::endl;
        }
    }

    file << "Relocations tables" << std::endl;
    file << "Section   Offset    Type    Symbol name" << std::endl;

    file << ".rel.text" << std::endl;
    for(SymbolHandler::RTEntry& rte : rt) {
        if(rte.section == ".text") {
            file << std::setw(6) << rte.section << "    ";
            file << std::setw(5) << rte.offset << "     ";
            file << rte.type << "           " << rte.symbolIndex << std::endl;
        }
    }
    file << ".rel.data" << std::endl;
    for(SymbolHandler::RTEntry& rte : rt) {
        if(rte.section == ".data") {
            file << std::setw(6) << rte.section << "    ";
            file << std::setw(5) << rte.offset << "     ";
            file << rte.type << "           " << rte.symbolIndex << std::endl;
        }
    }
    file << "Section   Offset    Type    Symbol name       Addend" << std::endl;
    file << ".rela.text" << std::endl;
    for(SymbolHandler::RATEntry& rate : rat) {
        if(rate.section == ".text") {
            file << std::setw(6) << rate.section << "    ";
            file << std::setw(5) << rate.offset << "     ";
            file << rate.type << "           " << rate.symbolIndex << "         ";
            file << rate.addend << std::endl;
        }
    }
    file << ".rela.data" << std::endl;
    for(SymbolHandler::RATEntry& rate : rat) {
        if(rate.section == ".data") {
            file << std::setw(6) << rate.section << "    ";
            file << std::setw(5) << rate.offset << "     ";
            file << rate.type << "           " << rate.symbolIndex << "         ";
            file << rate.addend << std::endl;
        }
    }

}

void FileWriter::setFile(std::string fileName) {

    file.open(fileName);

}

void FileWriter::setST(std::vector<SymbolHandler::STEntry> st) {
    this->st = st;
}

void FileWriter::setRT(std::vector<SymbolHandler::RTEntry> rt) {
    this->rt = rt;
}

void FileWriter::setRAT(std::vector<SymbolHandler::RATEntry> rat) {
    this->rat = rat;
}

bool FileWriter::isRelPresent(std::string section) {
    for(int i = 0; i < rt.size(); i++) {
        if(rt[i].section == section) {
            return true;
        }
    }

    return false;
}

bool FileWriter::isRelaPresent(std::string section) {
    for(int i = 0; i < rat.size(); i++) {
        if(rat[i].section == section) {
            return true;
        }
    }

    return false;
}

int FileWriter::getSectionSize(std::string section) {
    for(int i = 0; i < st.size(); i++) {
        if(st[i].name == section) {
            return st[i].size;
        }
    }

    return 0;
}

int FileWriter::numOfSecRelocs(std::string section) {
    int n = 0;

    for(int i = 0; i < rt.size(); i++) {
        if(rt[i].section == section) {
            n++;
        }
    }

    return n;
}

int FileWriter::numOfSecRelas(std::string section) {
    int n = 0;

    for(int i = 0; i < rat.size(); i++) {
        if(rat[i].section == section) {
            n++;
        }
    }

    return n;
}