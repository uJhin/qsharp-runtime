//----------------------------------------------------------------------------------------------------------------------
// <auto-generated />
// This code was generated by the Microsoft.Quantum.Qir.Runtime.Tools package.
// The purpose of this source code file is to provide an entry-point for executing a QIR program.
// It handles parsing of command line arguments, and it invokes an entry-point function exposed by the QIR program.
//----------------------------------------------------------------------------------------------------------------------

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include "CLI11.hpp"

#include "QirRuntime.hpp"
#include "QirContext.hpp"

#include "SimFactory.hpp"

using namespace Microsoft::Quantum;
using namespace std;

using RangeTuple = tuple<int64_t, int64_t, int64_t>;
struct InteropRange
{
    int64_t Start;
    int64_t Step;
    int64_t End;

    InteropRange() :
        Start(0),
        Step(0),
        End(0){}

    InteropRange(RangeTuple rangeTuple) :
        Start(get<0>(rangeTuple)),
        Step(get<1>(rangeTuple)),
        End(get<2>(rangeTuple)){}
};

InteropRange* TranslateRangeTupleToInteropRangePointer(RangeTuple& rangeTuple)
{
    return new InteropRange(rangeTuple);
}

const char* TranslateStringToCharBuffer(string& s)
{
    return s.c_str();
}

map<string, uint8_t> EnumMap {
    {"false", static_cast<uint8_t>(0)},
    {"true", static_cast<uint8_t>(1)},
    {"Zero", static_cast<uint8_t>(0)},
    {"One", static_cast<uint8_t>(1)},
    {"PauliI", static_cast<uint8_t>(PauliId::PauliId_I)},
    {"PauliX", static_cast<uint8_t>(PauliId::PauliId_X)},
    {"PauliY", static_cast<uint8_t>(PauliId::PauliId_Y)},
    {"PauliZ", static_cast<uint8_t>(PauliId::PauliId_Z)}
};

extern "C" void UseResultArg(
    uint8_t ResultArg
); // QIR interop function.

int main(int argc, char* argv[])
{
    CLI::App app("QIR Standalone Entry Point");

    // Initialize runtime.
    unique_ptr<IRuntimeDriver> sim = CreateFullstateSimulator();
    QirContextScope qirctx(sim.get(), false /*trackAllocatedObjects*/);

    // Add the --simulation-output option.
    string simulationOutputFile;
    CLI::Option* simulationOutputFileOpt = app.add_option(
        "--simulation-output",
        simulationOutputFile,
        "File where the output produced during the simulation is written");

    // Add a command line option for each entry-point parameter.
    uint8_t ResultArgCli;
    app.add_option("--ResultArg", ResultArgCli, "Option to provide a value for the ResultArg parameter")
        ->required()
        ->transform(CLI::CheckedTransformer(EnumMap, CLI::ignore_case));

    // After all the options have been added, parse arguments from the command line.
    CLI11_PARSE(app, argc, argv);

    // Cast parsed arguments to its interop types.
    uint8_t ResultArgInterop = ResultArgCli;

    // Redirect the simulator output from std::cout if the --simulation-output option is present.
    ostream* simulatorOutputStream = &cout;
    ofstream simulationOutputFileStream;
    if (!simulationOutputFileOpt->empty())
    {
        simulationOutputFileStream.open(simulationOutputFile);
        SetOutputStream(simulationOutputFileStream);
        simulatorOutputStream = &simulationOutputFileStream;
    }

    // Execute the entry point operation.
    UseResultArg(
        ResultArgInterop
    );

    // Flush the output of the simulation.
    simulatorOutputStream->flush();
    if (simulationOutputFileStream.is_open())
    {
        simulationOutputFileStream.close();
    }

    return 0;
}
