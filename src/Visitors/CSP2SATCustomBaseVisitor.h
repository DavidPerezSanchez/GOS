//
// Created by Roger Generoso Masós on 30/03/2020.
//

#ifndef CSP2SAT_CSP2SATCUSTOMBASEVISITOR_H
#define CSP2SAT_CSP2SATCUSTOMBASEVISITOR_H

#include <CSP2SATBaseVisitor.h>
#include "../Symtab/SymbolTable.h"
#include "../Symtab/Value/Value.h"
#include "../Symtab/Value/BoolValue.h"
#include "../Symtab/Value/IntValue.h"
#include "../Visitors/Utils.h"
#include "../Errors/CSP2SATException.h"
#include "../Errors/CSP2SATExceptionsRepository.h"
#include "../Symtab/Scope/LocalScope.h"
#include "../Symtab/Symbol/StringSymbol.h"
#include "../Symtab/Symbol/formulaReturn.h"


using namespace CSP2SAT;
using namespace std;

class CSP2SATCustomBaseVisitor : public CSP2SATBaseVisitor {

protected:
    bool accessingNotLeafVariable = false;
    SymbolTable *st;
    SMTFormula *_f;
    Scope *currentScope;
    Scope *currentLocalScope = nullptr;

public:

    explicit CSP2SATCustomBaseVisitor(SymbolTable *symbolTable, SMTFormula *f) {
        this->st = symbolTable;
        this->_f = f;
        this->currentScope = this->st->gloabls;
    }

    antlrcpp::Any visitCsp2sat(CSP2SATParser::Csp2satContext *ctx) override {
        try{
            return CSP2SATBaseVisitor::visitCsp2sat(ctx);
        }
        catch (CSP2SATException & e) {
            cerr << e.getErrorMessage() << endl;
        }
        catch (exception & e) {
            cerr << e.what() << endl;
        }
        return nullptr;
    }


    antlrcpp::Any visitExprTop(CSP2SATParser::ExprTopContext *ctx) override {
        try {
            return CSP2SATBaseVisitor::visitExprTop(ctx);
        } catch (CSP2SATException &e) {
            throw;
        }
    }

    antlrcpp::Any visitExprTernary(CSP2SATParser::ExprTernaryContext *ctx) override {
        Value *condition = visit(ctx->condition);
        if (condition->getRealValue())
            return visit(ctx->op1);
        else
            return visit(ctx->op2);
    }

    antlrcpp::Any visitExprAnd(CSP2SATParser::ExprAndContext *ctx) override {
        Value *result = visit(ctx->exprOr(0));
        if (ctx->exprOr().size() > 1) {
            BoolValue *res = new BoolValue(true);
            for (int i = 0; i < ctx->exprOr().size(); i++) {
                Value *currValue = visit(ctx->exprOr(i));
                res->setRealValue(res->getRealValue() && currValue->getRealValue());
            }
            return (Value *) res;
        }
        return result;
    }

    antlrcpp::Any visitExprOr(CSP2SATParser::ExprOrContext *ctx) override {
        Value *result = visit(ctx->exprEq(0));
        if (ctx->exprEq().size() > 1) {
            BoolValue *res = new BoolValue(false);
            for (int i = 0; i < ctx->exprEq().size(); i++) {
                Value *currValue = visit(ctx->exprEq(i));
                if (currValue->isBoolean())
                    res->setRealValue(res->getRealValue() || currValue->getRealValue());
                else {
                    throw CSP2SATInvalidExpressionTypeException(
                            ctx->exprEq(i)->getStart()->getLine(),
                            ctx->exprEq(i)->getStart()->getCharPositionInLine(),
                            ctx->getText(),
                            Utils::getTypeName(SymbolTable::tInt),
                            Utils::getTypeName(SymbolTable::tBool)
                    );
                }
            }
            return (Value *) res;
        }
        return result;
    }

    antlrcpp::Any visitExprEq(CSP2SATParser::ExprEqContext *ctx) override {
        Value *lVal = visit(ctx->exprRel(0));
        if (ctx->exprRel().size() > 1) {
            for (int i = 1; i < ctx->exprRel().size(); i++) {
                Value *rVal = visit(ctx->exprRel(i));
                if (lVal->isBoolean() == rVal->isBoolean()) {
                    BoolValue *res = new BoolValue();
                    if (ctx->opEquality(0)->getText() == "==")
                        res->setRealValue(lVal->getRealValue() == rVal->getRealValue());
                    else
                        res->setRealValue(lVal->getRealValue() != rVal->getRealValue());
                    lVal = res;
                } else {
                    throw CSP2SATTypeNotMatchException(
                            ctx->opEquality(0)->start->getLine(),
                            ctx->opEquality(0)->start->getCharPositionInLine(),
                            ctx->getText()
                    );
                }

            }

        }
        return lVal;
    }

    antlrcpp::Any visitExprRel(CSP2SATParser::ExprRelContext *ctx) override {
        Value *lVal = visit(ctx->exprSumDiff(0));
        if (ctx->exprSumDiff().size() == 2) {
            Value *rVal = visit(ctx->exprSumDiff(1));

            if (lVal->isBoolean()) {
                throw CSP2SATInvalidExpressionTypeException(
                        ctx->exprSumDiff(0)->start->getLine(),
                        ctx->exprSumDiff(0)->start->getCharPositionInLine(),
                        ctx->exprSumDiff(0)->getText(),
                        Utils::getTypeName(SymbolTable::tBool),
                        Utils::getTypeName(SymbolTable::tInt)
                );
            }
            if (rVal->isBoolean()) {
                throw CSP2SATInvalidExpressionTypeException(
                        ctx->exprSumDiff(1)->start->getLine(),
                        ctx->exprSumDiff(1)->start->getCharPositionInLine(),
                        ctx->exprSumDiff(1)->getText(),
                        Utils::getTypeName(SymbolTable::tBool),
                        Utils::getTypeName(SymbolTable::tInt)
                );
            };

            BoolValue *res = new BoolValue();
            if (ctx->opRelational(0)->getText() == "<")
                res->setRealValue(lVal->getRealValue() < rVal->getRealValue());
            else if (ctx->opRelational(0)->getText() == ">")
                res->setRealValue(lVal->getRealValue() > rVal->getRealValue());
            else if (ctx->opRelational(0)->getText() == "<=")
                res->setRealValue(lVal->getRealValue() <= rVal->getRealValue());
            else
                res->setRealValue(lVal->getRealValue() >= rVal->getRealValue());
            return (Value *) res;

        } else if (ctx->exprSumDiff().size() > 2) {
            throw CSP2SATInvalidOperationException(
                    ctx->start->getLine(),
                    ctx->start->getCharPositionInLine(),
                    ctx->getText()
            );
        }
        return lVal;
    }

    antlrcpp::Any visitExprSumDiff(CSP2SATParser::ExprSumDiffContext *ctx) override {
        Value *result = visit(ctx->exprMulDivMod(0));
        if (ctx->exprMulDivMod().size() > 1) {
            IntValue *res = new IntValue(result->getRealValue());
            for (int i = 0; i < ctx->opSumDiff().size(); i++) {
                Value *currValue = visit(ctx->exprMulDivMod(i + 1));
                if (!currValue->isBoolean()) {
                    if (ctx->opSumDiff(i)->getText() == "+")
                        res->setRealValue(res->getRealValue() + currValue->getRealValue());
                    else
                        res->setRealValue(res->getRealValue() - currValue->getRealValue());
                } else {
                    throw CSP2SATInvalidExpressionTypeException(
                            ctx->exprMulDivMod(1)->start->getLine(),
                            ctx->exprMulDivMod(1)->start->getCharPositionInLine(),
                            ctx->exprMulDivMod(1)->getText(),
                            Utils::getTypeName(SymbolTable::tBool),
                            Utils::getTypeName(SymbolTable::tInt)
                    );
                }

            }
            return (Value *) res;
        }
        return result;
    }

    antlrcpp::Any visitExprMulDivMod(CSP2SATParser::ExprMulDivModContext *ctx) override {
        Value *result = visit(ctx->exprNot(0));
        if (ctx->exprNot().size() > 1) {
            IntValue *res = new IntValue(result->getRealValue());
            for (int i = 0; i < ctx->opMulDivMod().size(); i++) {
                Value *currValue = visit(ctx->exprNot(i + 1));
                if (!currValue->isBoolean()) {
                    if (ctx->opMulDivMod(i)->getText() == "*")
                        res->setRealValue(res->getRealValue() * currValue->getRealValue());
                    else if (ctx->opMulDivMod(i)->getText() == "/")
                        res->setRealValue(res->getRealValue() / currValue->getRealValue());
                    else
                        res->setRealValue(res->getRealValue() % currValue->getRealValue());
                } else {

                }
            }
            return (Value *) res;
        }
        return result;
    }

    antlrcpp::Any visitExprNot(CSP2SATParser::ExprNotContext *ctx) override {
        Value *result = visit(ctx->expr_base());
        if (ctx->op) {
            if (result->isBoolean()) {
                return (Value *) new BoolValue(!result->getRealValue());
            } else {
                throw CSP2SATInvalidExpressionTypeException(
                        ctx->start->getLine(),
                        ctx->start->getCharPositionInLine(),
                        ctx->getText(),
                        Utils::getTypeName(SymbolTable::tInt),
                        Utils::getTypeName(SymbolTable::tBool)
                );
            }
        }
        return result;
    }

    antlrcpp::Any visitExpr_base(CSP2SATParser::Expr_baseContext *ctx) override {
        if (ctx->expr()) {
            return visit(ctx->expr());
        } else if (ctx->varAccess()) {
            Symbol *value = visit(ctx->varAccess());
            if (value->isAssignable()) {
                return (Value *) ((AssignableSymbol *) value)->getValue();
            } else {
                throw CSP2SATInvalidExpressionTypeException(
                        ctx->getStart()->getLine(),
                        ctx->getStart()->getCharPositionInLine(),
                        ctx->getText(),
                        Utils::getTypeName(value->type->getTypeIndex()),
                        Utils::getTypeName(SymbolTable::tInt)
                );
            }
        }
        return CSP2SATBaseVisitor::visitExpr_base(ctx);
    }


    antlrcpp::Any visitVarAccessObjectOrArray(CSP2SATParser::VarAccessObjectOrArrayContext *ctx) override {
        if (ctx->attr) {
            return (Symbol *) this->currentScope->resolve(ctx->attr->getText());
        } else if (ctx->index) {
            Scope *prev = this->currentScope;
            this->currentScope = this->currentLocalScope;
            Value *index = visit(ctx->index);
            this->currentScope = prev;

            Symbol *res = this->currentScope->resolve(to_string(index->getRealValue()));
            return (Symbol *) this->currentScope->resolve(to_string(index->getRealValue()));
        }
        return nullptr;
    }


    antlrcpp::Any visitVarAccess(CSP2SATParser::VarAccessContext *ctx) override {
        string a = ctx->TK_IDENT()->getText();
        Symbol *var = this->currentScope->resolve(ctx->TK_IDENT()->getText());

        if (var == nullptr) {
            throw CSP2SATNotExistsException(
                    ctx->start->getLine(),
                    ctx->start->getCharPositionInLine(),
                    ctx->getText()
            );
        }

        if (!ctx->varAccessObjectOrArray().empty()) {
            for (int i = 0; i < ctx->varAccessObjectOrArray().size(); i++) {
                auto nestedScope = ctx->varAccessObjectOrArray(i);
                string nest = nestedScope->getText();
                if (var != nullptr && var->isScoped()) {
                    this->currentLocalScope = this->currentScope;
                    if (!nestedScope->underscore) {
                        this->currentScope = (ScopedSymbol *) var;
                        var = visit(nestedScope);
                    } else {
                        ScopedSymbol *result = (ScopedSymbol *) var;
                        for (int j = i + 1; j < ctx->varAccessObjectOrArray().size(); j++) {
                            if (ctx->varAccessObjectOrArray(j)->underscore) {
                                throw CSP2SATInvalidExpressionTypeException(
                                        ctx->start->getLine(),
                                        ctx->start->getCharPositionInLine(),
                                        ctx->getText(),
                                        "matrix",
                                        "list"
                                );
                            }
                            ArraySymbol *aux = new ArraySymbol(
                                    "aux",
                                    result,
                                    ((ArraySymbol *) result)->getElementsType()
                            );
                            for (auto currDimElem : ((ScopedSymbol *) result)->getScopeSymbols()) {
                                if (currDimElem.second->type->getTypeIndex() == SymbolTable::tArray) {
                                    this->currentScope = (ScopedSymbol *) currDimElem.second;
                                    Symbol *currDimSymElem = visit(ctx->varAccessObjectOrArray(j));
                                    aux->add(currDimSymElem);
                                } else {
                                    throw CSP2SATBadAccessException(
                                            ctx->start->getLine(),
                                            ctx->start->getCharPositionInLine(),
                                            ctx->getText()
                                    );
                                }
                            }
                            result = aux;
                        }
                        var = result;
                        this->currentScope = this->currentLocalScope;
                        break;
                    }
                    this->currentScope = this->currentLocalScope;
                } else {
                    throw CSP2SATBadAccessException(
                            ctx->start->getLine(),
                            ctx->start->getCharPositionInLine(),
                            ctx->getText()
                    );
                }
            }
        }
        if (var == nullptr || (!this->accessingNotLeafVariable && var->isScoped())) {
            throw CSP2SATBadAccessException(
                    ctx->start->getLine(),
                    ctx->start->getCharPositionInLine(),
                    ctx->getText()
            );
        }
        return var;
    }


    antlrcpp::Any visitValueBaseType(CSP2SATParser::ValueBaseTypeContext *ctx) override {
        if (ctx->integer) {
            return (Value *) new IntValue(stoi(ctx->integer->getText()));
        } else {
            return (Value *) new BoolValue(ctx->boolean->getText() == "true");
        }
    }

    antlrcpp::Any visitRangList(CSP2SATParser::RangListContext *ctx) override {

        Value *minRange = visit(ctx->min);
        Value *maxRange = visit(ctx->max);

        int minValue = minRange->getRealValue();
        int maxValue = maxRange->getRealValue();

        if (minValue < maxValue) {
            ArraySymbol *result = new ArraySymbol(
                    "auxRangList",
                    this->currentScope,
                    SymbolTable::_integer
            );
            for (int i = 0; i <= (maxValue - minValue); i++) {
                AssignableSymbol *newValue = new AssignableSymbol(to_string(i), SymbolTable::_integer);
                newValue->setValue(new IntValue(minValue + i));
                result->add(newValue);
            }
            return result;
        } else {
            throw CSP2SATException(
                    ctx->start->getLine(),
                    ctx->start->getCharPositionInLine(),
                    "Range must be ascendant"
            );
        }
    }

    antlrcpp::Any visitAuxiliarListAssignation(CSP2SATParser::AuxiliarListAssignationContext *ctx) override {
        ArraySymbol *arrayDefined = visit(ctx->list());
        return pair<string, ArraySymbol *>(ctx->TK_IDENT()->getText(), arrayDefined);
    }



    antlrcpp::Any visitComprehensionList(CSP2SATParser::ComprehensionListContext *ctx) override {
        auto *listLocalScope = new LocalScope(this->currentScope);
        map<string, ArraySymbol *> ranges;
        this->currentScope = listLocalScope;
        for (int i = 0; i < ctx->auxiliarListAssignation().size(); i++) {
            pair<string, ArraySymbol *> currAuxVar = visit(ctx->auxiliarListAssignation(i));
            ranges.insert(currAuxVar);
        }

        vector<map<string, Symbol *>> possibleAssignations = Utils::getAllCombinations(ranges);
        ArraySymbol *newList = nullptr;

        for (const auto &assignation: possibleAssignations) {
            for (const auto &auxVarAssign : assignation)
                listLocalScope->assign(auxVarAssign.first, auxVarAssign.second);

            bool condition = true;
            if (ctx->condExpr) {
                Value *cond = visit(ctx->condExpr);
                if (!cond->isBoolean()) {
                    throw CSP2SATInvalidExpressionTypeException(
                            ctx->condExpr->start->getLine(),
                            ctx->condExpr->start->getCharPositionInLine(),
                            ctx->condExpr->getText(),
                            Utils::getTypeName(SymbolTable::tInt),
                            Utils::getTypeName(SymbolTable::tBool)
                    );
                }
                condition = cond->getRealValue();
            }

            Symbol *exprRes;
            if (ctx->listResultExpr()->varAcc) {
                this->accessingNotLeafVariable = true;
                exprRes = visit(ctx->listResultExpr());
                this->accessingNotLeafVariable = false;
            }
            else if (ctx->listResultExpr()->resExpr) {
                Value *val = visit(ctx->listResultExpr()->resExpr);
                auto *valueResult = new AssignableSymbol(
                        to_string(rand()),
                        val->isBoolean() ? SymbolTable::_boolean : SymbolTable::_integer
                );
                valueResult->setValue(val);
                exprRes = valueResult;
            }
            else if (ctx->listResultExpr()->string()){
                string currStr = visit(ctx->listResultExpr()->string());
                exprRes = new StringSymbol(currStr);
            }
            else {
                formulaReturn * formula = visit(ctx->listResultExpr()->constraint_expression());
                exprRes = (Symbol*) formula;
            }

            if (newList == nullptr)
                newList = new ArraySymbol(
                        "comprehensionListAux",
                        listLocalScope,
                        exprRes->type
                );

            if (condition)
                newList->add(exprRes);
        }
        this->currentScope = listLocalScope->getEnclosingScope();
        return newList;
    }

    antlrcpp::Any visitVarAccessList(CSP2SATParser::VarAccessListContext *ctx) override {
        Symbol *array = nullptr;
        this->accessingNotLeafVariable = true;
        array = visit(ctx->varAccess());
        this->accessingNotLeafVariable = false;

        if (array && array->type && array->type->getTypeIndex() == SymbolTable::tArray) {
            return (ArraySymbol *) array;
        } else {
            throw CSP2SATInvalidExpressionTypeException(
                    ctx->start->getLine(),
                    ctx->start->getCharPositionInLine(),
                    ctx->getText(),
                    Utils::getTypeName(array->type->getTypeIndex()),
                    "array"
            );
        }
    }

    antlrcpp::Any visitExplicitList(CSP2SATParser::ExplicitListContext *ctx) override {
        ArraySymbol *resultList = nullptr;

        for (auto currVal : ctx->listResultExpr()) {
            Symbol *curr = nullptr;
            if (currVal->varAcc) {
                curr = visit(currVal);
            } else if (currVal->resExpr) {
                Value *exprVal = visit(currVal->resExpr);
                curr = new AssignableSymbol(to_string(rand()),
                                            exprVal->isBoolean() ? SymbolTable::_boolean : SymbolTable::_integer);
                ((AssignableSymbol *) curr)->setValue(exprVal);
            } else if (currVal->string()) {
                string currStr = visit(currVal->string());
                curr = new StringSymbol(currStr);
            } else {
                formulaReturn * a =  visit(currVal->constraint_expression());
                curr = (Symbol*) a;
            }

            if (resultList == nullptr) {
                resultList = new ArraySymbol(
                        to_string(rand()),
                        this->currentScope,
                        curr->type
                );
            }

            if (curr->type->getTypeIndex() == resultList->getElementsType()->getTypeIndex()) {
                resultList->add(curr);
            } else {
                throw CSP2SATInvalidExpressionTypeException(
                        currVal->start->getLine(),
                        currVal->start->getCharPositionInLine(),
                        currVal->getText(),
                        Utils::getTypeName(curr->type->getTypeIndex()),
                        Utils::getTypeName(resultList->getElementsType()->getTypeIndex())
                );
            }
        }
        return resultList;
    }
};


#endif //CSP2SAT_CSP2SATCUSTOMBASEVISITOR_H
