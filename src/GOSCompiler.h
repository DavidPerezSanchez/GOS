//
// Created by Roger Generoso Masós on 20/04/2020.
// Modified by David Pérez Sánchez on 07/08/2022.
//

#ifndef CSP2SAT_CSP2SATCOMPILER_H
#define CSP2SAT_CSP2SATCOMPILER_H

// antlr4 runtime
#include "antlr4-runtime.h"

// generated lexer and parser
#include <BUPLexer.h>
#include <BUPParser.h>
#include <JSONLexer.h>
#include <JSONParser.h>

#include <controllers/solvingarguments.h>

#include <utility>
#include <controllers/basiccontroller.h>
#include "Symtab/SymbolTable.h"

// custom visitor
#include "Visitors/GOSTypeVarDefinitionVisitor.h"
#include "Visitors/GOSPredVisitor.h"
#include "Visitors/GOSConstraintsVisitor.h"

// custom error
#include "Errors/GOSErrorListener.h"
#include "GOSEncoding.h"
#include "Visitors/Input/GOSJSONInputVisitor.h"
#include "Visitors/Output/GOSOutputVisitor.h"

// std
#include <iostream>
#include <string>

namespace GOS {

class GOSCompiler {
public:
    GOSCompiler(std::string inputFilename, std::string modelFilename, SolvingArguments *sargs) :
        _inputFilename(std::move(inputFilename)), _modelFilename(std::move(modelFilename)), sargs(sargs)
    {
        symbolTable = new SymbolTable();
        _f = new SMTFormula();
    }

    void run(){
        // Parse input file
        ParamJSONRef readParams = runInputVisitor();

        // Create model file parse tree
        bool synError = false;
        BUPFileRef modelFile = BUPFile::Create(_modelFilename);
        symbolTable->parsedFiles.emplace_back(modelFile);
        BUPParser::Csp2satContext *tree = modelFile->getParser()->csp2sat();
        if(modelFile->getParser()->getNumberOfSyntaxErrors() > 0)
            synError = true;

        GOSTypeVarDefinitionVisitor visitor(symbolTable, _f, readParams);
        visitor.visit(tree);

        GOSPredVisitor predVisitor(symbolTable, _f, _modelFilename);
        predVisitor.visit(tree);

        if(!symbolTable->errors){
            GOSConstraintsVisitor constraintsVisitor(symbolTable, _f);
            constraintsVisitor.visit(tree);

            if(!synError){
                if(!symbolTable->errors){
                    if(_f->getNSoftClauses() > 1)
                        sargs->setOption(OPTIMIZER, (std::string)"native");
                    else
                        sargs->setOption(OPTIMIZER, (std::string)"check");


                    GOSEncoding encoding(_f, symbolTable);
                    BasicController c(sargs, &encoding, true, 0, 0);
                    c.run();

                    if(encoding.isSat()){
                        CSP2SATOutputVisitor outputVisitor(symbolTable, _f);
                        bool customOutput = outputVisitor.visit(tree);
                        if(!customOutput)
                            encoding.printModelSolution(std::cout);
                    }
                }
                else {
                    std::cerr << std::endl <<  "Execution stopped due to errors in constraint definition" << std::endl;
                }
            }
            else{
                std::cerr << "Execution stopped" << std::endl;
            }
        }
        else {
            std::cerr << "Execution stopped due to errors in input" << std::endl;
        }
    }

private:
    SMTFormula *_f;
    std::filesystem::path _inputFilename;
    std::filesystem::path _modelFilename;
    SymbolTable *symbolTable;
    SolvingArguments *sargs;

    ParamJSONRef runInputVisitor(){
        std::string inStr;
        try {
            inStr = Utils::readFile(_inputFilename);
        } catch(std::ifstream::failure e) {
            std::cerr << "Error reading file: " << _inputFilename << std::endl;
            abort();
        }
        GOSJSONInputVisitor visitor(_inputFilename);
        antlr4::ANTLRInputStream input(inStr);
        JSONLexer lexer(&input);
        antlr4::CommonTokenStream tokens(&lexer);
        JSONParser parser(&tokens);
        JSONParser::JsonContext *tree = parser.json();
        return visitor.visit(tree);
    }

};

}

#endif //CSP2SAT_CSP2SATCOMPILER_H
