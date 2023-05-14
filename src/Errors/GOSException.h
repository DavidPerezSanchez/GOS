//
// Created by Roger Generoso Masós on 19/04/2020.
// Modified by David Pérez Sánchez on 04/09/2022.
//

#ifndef CSP2SAT_GOSEXCEPTION_H
#define CSP2SAT_GOSEXCEPTION_H

#include <exception>
#include <string>
#include <optional>
#include "../Symtab/SymbolTable.h"

namespace GOS {

struct ExceptionLocation {
    std::filesystem::path file;
    size_t line;
    size_t pos;

    std::string toString() const {
        return "In file \"" + file.string() + "\" (" + std::to_string(line) + ":" + std::to_string(pos) + "):";
    }
};

class GOSMessage {
public:
    GOSMessage(ExceptionLocation location, const std::string &message) : _location(location), _message(message) {}

    GOSMessage(const std::string &message) : _message(message) {}

    virtual ~GOSMessage() {}

    virtual std::string getErrorMessage() = 0;

    void setLocation(ExceptionLocation location) {
        _location = location;
    }

protected:
    std::optional<ExceptionLocation> _location;
    std::string _message;
};

class GOSException : public std::exception, public GOSMessage {
public:
    GOSException(ExceptionLocation location, const std::string &message) : GOSMessage(location, message) {
        SymbolTable::errors = true;
    }

    GOSException(const std::string &message) : GOSMessage(message) {
        SymbolTable::errors = true;
    }

    std::string getErrorMessage() override {
        std::string error;
        if(_location.has_value())
            error += _location.value().toString() + " ";
        error += "ERROR: " + _message;
        return error;
    }
};

class GOSWarning : public GOSMessage {
public:
    GOSWarning(ExceptionLocation location, const std::string &message) :  GOSMessage(location, message) { }

    GOSWarning(const std::string &message) : GOSMessage(message){ }

    std::string getErrorMessage() {
        std::string warn;
        if(_location.has_value())
            warn += _location.value().toString() + " ";
        warn += "WARNING: " + _message;
        return warn;
    }
};

}

#endif //CSP2SAT_GOSEXCEPTION_H
