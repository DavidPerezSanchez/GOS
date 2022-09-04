#include <string>
#include "api/controllers/solvingarguments.h"
#include "GOSCompiler.h"

using namespace std;
using namespace GOS;

enum ProgramArg {
    SHOWFORMULA
};

int main(int argc, char **argv) {

    Arguments<ProgramArg> *pargs = new Arguments<ProgramArg>(
            {
                    arguments::arg("modelfile", "Instance file path."),
                    arguments::arg("datafile", "Input params file path.")
            },
            2,
            {
                    arguments::bop("pf", "print-formula", SHOWFORMULA, false, "Print CNF formula"),
            },
            "Solve CSP to SAT"
    );

    SolvingArguments *sargs = SolvingArguments::readArguments(argc, argv, pargs);

    if (sargs->getStringOption(FILE_FORMAT) != "dimacs")
        sargs->setOption(FILE_FORMAT, (std::string)"dimacs");

    bool showFormula = pargs->getBoolOption(SHOWFORMULA);
    if (showFormula) {
        SolvingArg print = sargs->getOptionRef("-e");
        sargs->setOption(print, true);
        SolvingArg format = sargs->getOptionRef("-f");
        sargs->setOption(format, (string) "dimacs");
    }

    const std::string modelFilename = pargs->getArgument(0);
    const std::string inputFilename = pargs->getArgument(1);
    GOSCompiler *compiler = new GOSCompiler(inputFilename, modelFilename, sargs);
    compiler->run();

    delete pargs, sargs, compiler;

    return 0;
}