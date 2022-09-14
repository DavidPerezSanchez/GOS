//
// Created by Roger Generoso Masós on 24/04/2020.
// Modified by David Pérez Sánchez on 22/07/2022.
//

#ifndef CSP2SAT_STRINGSYMBOL_H
#define CSP2SAT_STRINGSYMBOL_H


#include <string>
#include "Symbol.h"
#include "../SymbolTable.h"

namespace GOS {

class StringSymbol;
typedef std::shared_ptr<StringSymbol> StringSymbolRef;
class StringSymbol : public Symbol {
public:
    static StringSymbolRef Create(const std::string &str) {
        return StringSymbolRef(new StringSymbol(str));
    }

protected:
    StringSymbol(const std::string &str) : Symbol(str, SymbolTable::_string) {}
};

}

#endif //CSP2SAT_STRINGSYMBOL_H
