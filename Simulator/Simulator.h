////////////////////////////////
/// usage : 1.  prepare the input, call the solver through command line interface
///             or class library interface, and then collect output for analysis.
///
/// note :  1.
////////////////////////////////

#ifndef ZQY_TRAVELING_PURCHASER_SIMULATOR_H
#define ZQY_TRAVELING_PURCHASER_SIMULATOR_H


#include <string>
#include <vector>

#include "../Solver/Solver.h"
#include "../Solver/Problem.h"
#include "../Solver/Utility.h"


namespace szx {

using Cmd = Solver::Cli;
using Env = Solver::Environment;

class Simulator {
    #pragma region Type
public:
    // TODO[szx][4]: make it be able to run with serveral env and cfg.
    struct Task {
        String instSet;
        String instId;
        String randSeed;
        String timeout;
        String maxIter;
        String jobNum;
        String cfgPath;
        String logPath;
        String runId;
    };

	struct InstanceTrait {
		// int horizonLen = 24 * 60;
		int nodeNum = 80;
		//Interval<int> nodeNum = Interval<int>(50, Problem::MaxNodeNum);//一般是确定好的
		Interval<int> edgeNum = Interval<int>(nodeNum*(nodeNum - 1) / 4, nodeNum*(nodeNum - 1) / 2);
		Interval<int> Cost = Interval<int>(0, Problem::MaxCost);
		Interval<int> Value = Interval<int>(0, Problem::MaxValue);
		Interval<int> upminTime = Interval<int>(1, Problem::upMinTime);
		Interval<int> requiredNum = Interval<int>(nodeNum, nodeNum * 4);//Problem::MaxRequiredNum);
		int periodLength = 110;
    };
    #pragma endregion Type

    #pragma region Constant
public:
    static String InstanceDir() { return Env::DefaultInstanceDir(); }
    static String SolutionDir() { return Env::DefaultSolutionDir(); }

    static String ProgramName() { return "Simulator.exe";  }

    enum ArgIndex { ExeName = 0, ArgStart };
    #pragma endregion Constant

    #pragma region Constructor
public:
    #pragma endregion Constructor

    #pragma region Method
public:
    static void initDefaultEnvironment();

    // invoke solver with arguments explicitly specifying running information.
    void run(const Task &task);
    // invoke solver with single argument of environment file path.
    void run(const String &envPath = Env::DefaultEnvPath());

    // utility for debugging solver with certain arguments.
    void debug();
    // utility for testing all instances.
    void benchmark(int repeat = 1);
    // utility for testing all instances using a thread pool.
    void parallelBenchmark(int repeat);


    void generateInstance(const InstanceTrait &trait);
    void generateInstance() {
        InstanceTrait trait;
        generateInstance(trait);
    }
    #pragma endregion Method

    #pragma region Field
public:
    #pragma endregion Field
};

}


#endif  // ZQY_TRAVELING_PURCHASER_SIMULATOR_H
