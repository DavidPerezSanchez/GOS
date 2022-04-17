//
// Created by Roger Generoso Masós on 20/04/2020.
//

#ifndef CSP2SAT_GOSINPUTEXCEPTIONSREPOSITORY_H
#define CSP2SAT_GOSINPUTEXCEPTIONSREPOSITORY_H

#include "GOSException.h"

namespace GOS {

class CSP2SATBadInputTypeException : public GOSException {
public:
    CSP2SATBadInputTypeException(int line, int pos, const string &name) :
            GOSException(
                    line,
                    pos,
                    "\"" + name + "\":" + "Only int and bool values allowed as params"
            ) {}
};


class CSP2SATInputNotFoundValue : public GOSException {
public:
    CSP2SATInputNotFoundValue(const string &access) :
            GOSException(
                    0,
                    0,
                    "\"" + access + "\":" + " Not found in input file"
            ) {}
};


class CSP2SATBadCardinalityConstraint : public GOSException {
public:
    CSP2SATBadCardinalityConstraint(int line, int pos, const string &name) :
            GOSException(
                    line,
                    pos,
                    name + ": Bad cardinality input"
            ) {}
};

}

#endif //CSP2SAT_GOSINPUTEXCEPTIONSREPOSITORY_H
