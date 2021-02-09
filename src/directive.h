#ifndef _DIRECTIVE_H_
#define _DIRECTIVE_H_

#include <string>
#include <vector>
#include <utility>

struct Directive {
    std::string mnemonic;
    std::vector<std::pair<int, std::string>> symbolList;
    std::vector<std::pair<int, std::string>> literalList;
    std::vector<bool> expresionOps;
};

#endif