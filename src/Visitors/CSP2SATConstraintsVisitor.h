//
// Created by Roger Generoso Masós on 01/04/2020.
//

#ifndef CSP2SAT_CSP2SATCONSTRAINTSVISITOR_H
#define CSP2SAT_CSP2SATCONSTRAINTSVISITOR_H


#include "CSP2SATCustomBaseVisitor.h"
#include "smtformula.h"


class CSP2SATConstraintsVisitor : public CSP2SATCustomBaseVisitor {

public:
    explicit CSP2SATConstraintsVisitor(SymbolTable *symbolTable, SMTFormula * form) : CSP2SATCustomBaseVisitor(symbolTable, form) {}

//    antlrcpp::Any visitConstraintDefinition(CSP2SATParser::ConstraintDefinitionContext *ctx) override {
//        if(ctx->expr()){
//            Value * val = visit(ctx->expr());
//            boolvar x;
//            this->_f->addClause(x);
//        }
//    }

};


#endif //CSP2SAT_CSP2SATCONSTRAINTSVISITOR_H
