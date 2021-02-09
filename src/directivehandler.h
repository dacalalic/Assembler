#ifndef _DIRECTIVE_HANDLER_H_
#define _DIRECTIVE_HANDLER_H_

class Directive;
class SymbolHandler;
class FileWriter;

class DirectiveHandler {

private:

    SymbolHandler* sh;
    FileWriter* fw;

public:

    DirectiveHandler(SymbolHandler* sh, FileWriter* fw);

    void handleDirective(Directive* dir);

};

#endif