%{
#include <iostream>
#include <string>
#include <algorithm>
#include "syntaxanalyzer.h"

SyntaxAnalyzer sa;

%}

INSTRUCTION HALT|IRET|RET|INT|CALL|JMP|JEQ|JNE|JGT|PUSH|POP|XCHG|MOV|ADD|SUB|MUL|DIV|CMP|NOT|AND|OR|XOR|TEST|SHL|SHR
DIRECTIVE .global|.extern|.section|.byte|.word|.skip|.equ
LITERAL ([-]{0,1}[1-9][0-9]*)|0
SYMBOL [_a-zA-Z][_a-zA-Z0-9]*
REGISTER r[0-7]|pc|sp

%%
    /*Label*/
^[ \t]*{SYMBOL}/: {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    sa.processLabel(s);
                  }

    /*Instruction*/
([ \t]+|(:[ \t]*)){INSTRUCTION} |

([ \t]+|(:[ \t]*)){INSTRUCTION}b |

([ \t]+|(:[ \t]*)){INSTRUCTION}w |

^{INSTRUCTION}b |
^{INSTRUCTION}w |

^{INSTRUCTION}  {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    if(s[0] == ':') {
                        s.erase(s.begin(), s.begin() + 1);
                    }
                    sa.processInstr(s);
                }

    /*Directive*/
([ \t]+|(:[ \t]*)){DIRECTIVE} |

^{DIRECTIVE}    {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    if(s[0] == ':') {
                        s.erase(s.begin(), s.begin() + 1);
                    }
                    sa.processDir(s);
                }

[ \t]+${LITERAL}    {
                        std::string s(yytext);
                        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                        s.erase(s.begin(), s.begin() + 1);
                        sa.processImmediateLiteral(s, false);
                    }

[ \t]+${SYMBOL} {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    s.erase(s.begin(), s.begin() + 1);
                    sa.processImmediateSymbol(s, false);
                }

[ \t]+%{REGISTER}h |

[ \t]+%{REGISTER}l |

[ \t]+%{REGISTER}   {
                        std::string s(yytext);
                        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                        s.erase(s.begin(), s.begin() + 1);
                        sa.processRegDirect(s, false);
                    }

[ \t]+\(%{REGISTER}\) {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    s.erase(s.begin(), s.begin() + 2);
                    s.erase(s.end() - 1, s.end());
                    sa.processRegIndirect(s, false);
                }

[ \t]+{LITERAL}\(%{REGISTER}\)  {
                                    std::string s(yytext);
                                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                                    std::size_t pos = s.find("(");
                                    std::string reg = s.substr(pos);
                                    s.erase(s.begin() + pos, s.end());
                                    reg.erase(reg.begin(), reg.begin() + 2);
                                    reg.erase(reg.end() - 1, reg.end());
                                    sa.processRegIndDispLit(reg, s, false);
                                }

[ \t]+{SYMBOL}\(%{REGISTER}\)   {
                                    std::string s(yytext);
                                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                                    std::size_t pos = s.find("(");
                                    std::string reg = s.substr(pos);
                                    s.erase(s.begin() + pos, s.end());
                                    reg.erase(reg.begin(), reg.begin() + 2);
                                    reg.erase(reg.end() - 1, reg.end());
                                    sa.processRegIndDispSym(reg, s, false);
                                }

[ \t]+{LITERAL}  {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                    if(!sa.isLastInstrJmp()) {
                        sa.processMemLit(s, false);
                    }
                    else {
                        sa.processImmediateLiteral(s, true);
                    }
                }

[ \t]+{SYMBOL}  {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                    if(!sa.isLastInstrJmp()) {
                        sa.processMemSym(s, false);
                    }
                    else {
                        sa.processImmediateSymbol(s, true);
                    }
                }              

[ \t]+\*%{REGISTER}   {
                        std::string s(yytext);
                        s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                        s.erase(s.begin(), s.begin() + 2);

                        sa.processRegDirect(s, true);
                    }

[ \t]+\*\(%{REGISTER}\)  {
                            std::string s(yytext);
                            s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                            s.erase(s.begin(), s.begin() + 2);
                            s.erase(s.end() - 1, s.end());

                            sa.processRegIndirect(s, true);
                        }

[ \t]+\*{LITERAL}\(%{REGISTER}\)  {
                                    std::string s(yytext);
                                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                                    std::size_t pos = s.find("(");
                                    std::string reg = s.substr(pos);
                                    s.erase(s.begin() + pos, s.end());
                                    s.erase(s.begin(), s.begin() + 1);
                                    reg.erase(reg.begin(), reg.begin() + 2);
                                    reg.erase(reg.end() - 1, reg.end());

                                    sa.processRegIndDispLit(reg, s, true);
                                }

[ \t]+\*{SYMBOL}\(%{REGISTER}\)  {
                                    std::string s(yytext);
                                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                                    std::size_t pos = s.find("(");
                                    std::string reg = s.substr(pos);
                                    s.erase(s.begin() + pos, s.end());
                                    s.erase(s.begin(), s.begin() + 1);
                                    reg.erase(reg.begin(), reg.begin() + 2);
                                    reg.erase(reg.end() - 1, reg.end());

                                    sa.processRegIndDispSym(reg, s, true);
                                }

[ \t]+\*{LITERAL} {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    s.erase(s.begin(), s.begin() + 1);

                    sa.processMemLit(s, true);
                }

[ \t]+\*{SYMBOL}  {
                    std::string s(yytext);
                    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                    s.erase(s.begin(), s.begin() + 1);

                    sa.processMemSym(s, true);
                }

^[ \t]*.section[ \t]+[.](data|text|bss)    {
                                                std::string s(yytext);
                                                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                                                s.erase(s.begin(), s.begin() + 8);

                                                sa.processSection(s);
                                            }


^\n         |
:{0,1}[ \t]*\n  { sa.endOfLine(); }

.end[ \t]*\n    {
                    sa.endOfFile();
                    return 0;
                }

<<EOF>> {
            sa.endOfFile();
            return 0;
        }

,   {
        sa.comma();
    }

[ \t]+(\+|-) {
                std::string s(yytext);
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());

                sa.arithmeticOp(s);
             }

.   {
        std::string s(yytext);
        sa.unknownChar(s);
    }
%%

int yywrap(){
    return 0;
}

int main(int argc, char* argv[]){

    if(argc != 3) {
        std::cout << "Too many or too few arguments!" << std::endl;
        return 1;
    }

    yyin = fopen(argv[1], "r");
    sa.setOutFile(argv[2]);

    yylex();
    return 0;
}