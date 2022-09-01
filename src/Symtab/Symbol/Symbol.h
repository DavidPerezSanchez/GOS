//
// Created by Roger Generoso Masós on 19/03/2020.
//

#ifndef CSP2SAT_SYMBOL_H
#define CSP2SAT_SYMBOL_H

#include <string>
#include <utility>
#include <memory>

namespace GOS {

class Scope;
typedef std::shared_ptr<Scope> ScopeRef;
class Type;
typedef std::shared_ptr<Type> TypeRef;

class Symbol;
typedef std::shared_ptr<Symbol> SymbolRef;
class Symbol {
public:
    static SymbolRef Create(const std::string &name) {
        return SymbolRef(new Symbol(name));
    }

    static SymbolRef Create(const std::string &name, TypeRef type) {
        return SymbolRef(new Symbol(name, type));
    }

    virtual ~Symbol() {}

    std::string getName() const {
        return this->name;
    }

    virtual TypeRef getType() {
        return TypeRef(type);
    }

    virtual bool isAssignable() {
        return false;
    }

    virtual bool isScoped() {
        return false;
    }

    virtual std::string toString() const {
        return name;
    }

protected:
    Symbol(std::string name) : name(std::move(name)) {} //For constructing Type symbols

    Symbol(std::string name, TypeRef type) : name(std::move(name)), type(type) {} //For constructing var/const declarations

    TypeRef type = nullptr;
    std::string name;
};

}

#endif //CSP2SAT_SYMBOL_H
