#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <random>

#include <cstring>

#include "Simulator.h"
#include "ThreadPool.h"


using namespace std;


namespace szx {

void Simulator::initDefaultEnvironment() {
    Solver::Environment env;
    env.save(Env::DefaultEnvPath());

    Solver::Configuration cfg;
    cfg.save(Env::DefaultCfgPath());
}

void Simulator::run(const Task &task) {
    String instanceName(task.instSet + task.instId + ".json");
    String solutionName(task.instSet + task.instId + ".json");

    char argBuf[Cmd::MaxArgNum][Cmd::MaxArgLen];
    char *argv[Cmd::MaxArgNum];
    for (int i = 0; i < Cmd::MaxArgNum; ++i) { argv[i] = argBuf[i]; }
    strcpy(argv[ArgIndex::ExeName], ProgramName().c_str());

    int argc = ArgIndex::ArgStart;

    strcpy(argv[argc++], Cmd::InstancePathOption().c_str());
    strcpy(argv[argc++], (InstanceDir() + instanceName).c_str());

    System::makeSureDirExist(SolutionDir());
    strcpy(argv[argc++], Cmd::SolutionPathOption().c_str());
    strcpy(argv[argc++], (SolutionDir() + solutionName).c_str());

    if (!task.randSeed.empty()) {
        strcpy(argv[argc++], Cmd::RandSeedOption().c_str());
        strcpy(argv[argc++], task.randSeed.c_str());
    }

    if (!task.timeout.empty()) {
        strcpy(argv[argc++], Cmd::TimeoutOption().c_str());
        strcpy(argv[argc++], task.timeout.c_str());
    }

    if (!task.maxIter.empty()) {
        strcpy(argv[argc++], Cmd::MaxIterOption().c_str());
        strcpy(argv[argc++], task.maxIter.c_str());
    }

    if (!task.jobNum.empty()) {
        strcpy(argv[argc++], Cmd::JobNumOption().c_str());
        strcpy(argv[argc++], task.jobNum.c_str());
    }

    if (!task.runId.empty()) {
        strcpy(argv[argc++], Cmd::RunIdOption().c_str());
        strcpy(argv[argc++], task.runId.c_str());
    }

    if (!task.cfgPath.empty()) {
        strcpy(argv[argc++], Cmd::ConfigPathOption().c_str());
        strcpy(argv[argc++], task.cfgPath.c_str());
    }

    if (!task.logPath.empty()) {
        strcpy(argv[argc++], Cmd::LogPathOption().c_str());
        strcpy(argv[argc++], task.logPath.c_str());
    }

    Cmd::run(argc, argv);
}

void Simulator::run(const String &envPath) {
    char argBuf[Cmd::MaxArgNum][Cmd::MaxArgLen];
    char *argv[Cmd::MaxArgNum];
    for (int i = 0; i < Cmd::MaxArgNum; ++i) { argv[i] = argBuf[i]; }
    strcpy(argv[ArgIndex::ExeName], ProgramName().c_str());

    int argc = ArgIndex::ArgStart;

    strcpy(argv[argc++], Cmd::EnvironmentPathOption().c_str());
    strcpy(argv[argc++], envPath.c_str());

    Cmd::run(argc, argv);
}

void Simulator::debug() {
    Task task;
    task.instSet = "";
    task.instId = "rand.n80e1908r288p110";
    task.randSeed = "1500972793";
    //task.randSeed = to_string(RandSeed::generate());
    task.timeout = "180";
    //task.maxIter = "1000000000";
    task.jobNum = "1";
    task.cfgPath = Env::DefaultCfgPath();
    task.logPath = Env::DefaultLogPath();
    task.runId = "0";

    run(task);
}

void Simulator::benchmark(int repeat) {
    Task task;
    task.instSet = "";
    //task.timeout = "180";
    //task.maxIter = "1000000000";
    task.timeout = "3600";
    //task.maxIter = "1000000000";
    task.jobNum = "1";
    task.cfgPath = Env::DefaultCfgPath();
    task.logPath = Env::DefaultLogPath();

    random_device rd;
    mt19937 rgen(rd());
    // EXTEND[szx][5]: read it from InstanceList.txt.
    vector<String> instList({ "rand.n4e5r5p40", "rand.n80e1908r288p110" });
    for (int i = 0; i < repeat; ++i) {
        //shuffle(instList.begin(), instList.end(), rgen);
        for (auto inst = instList.begin(); inst != instList.end(); ++inst) {
            task.instId = *inst;
            task.randSeed = to_string(Random::generateSeed());
            task.runId = to_string(i);
            run(task);
        }
    }
}

void Simulator::parallelBenchmark(int repeat) {
    Task task;
    task.instSet = "";
    //task.timeout = "180";
    //task.maxIter = "1000000000";
    task.timeout = "3600";
    //task.maxIter = "1000000000";
    task.jobNum = "1";
    task.cfgPath = Env::DefaultCfgPath();
    task.logPath = Env::DefaultLogPath();

    ThreadPool<> tp(4);

    random_device rd;
    mt19937 rgen(rd());
    // EXTEND[szx][5]: read it from InstanceList.txt.
    vector<String> instList({ "rand.n4e5r5p40", "rand.n80e1908r288p110" });
    for (int i = 0; i < repeat; ++i) {
        //shuffle(instList.begin(), instList.end(), rgen);
        for (auto inst = instList.begin(); inst != instList.end(); ++inst) {
            task.instId = *inst;
            task.randSeed = to_string(Random::generateSeed());
            task.runId = to_string(i);
            tp.push([=]() { run(task); });
            this_thread::sleep_for(1s);
        }
    }
}

void Simulator::generateInstance(const InstanceTrait &trait) {//算例生成
    Random rand;
    //int gateNum = rand.pick(trait.gateNum.begin, trait.gateNum.end);
    //int flightNum = rand.pick(trait.flightNum.begin, trait.flightNum.end);

    Problem::Input input;
    int nodeNum = trait.nodeNum;
    int edgeNum = rand.pick(trait.edgeNum.begin, trait.edgeNum.end);
    input.set_periodlength(trait.periodLength);
    input.set_sourcenode(0);
    input.set_targetnode(0);
    //input.mutable_airport()->set_bridgenum(rand.pick(trait.bridgeNum.begin, trait.bridgeNum.end));

     /*
       1.将边分批构造，且边的初始节点大于终止节点：
       1.第一批确保有解,构成一个闭合回路,第二批确保回到起点的边有多条
       3.第三批边数随机生成不重复边
     */
    for (int i = 0; i != nodeNum; ++i) {
        input.add_nodeid(i);
    }
    cout << "ok" << endl;

    vector <vector<int>> repectflag(nodeNum, vector<int>(nodeNum, 0));
    for (int i = 0; i != edgeNum; ++i) {
        auto &edge(*input.add_edges());
        edge.set_id(i);
        int Cost = rand.pick(trait.Cost.begin, trait.Cost.end);
        edge.set_cost(Cost);
        int minTime = rand.pick(trait.upminTime.begin, trait.upminTime.end);
        edge.set_mintime(minTime);
        if (nodeNum < 20) {//构造小算例
            if (i < nodeNum) {
                edge.set_source(i);
                if (i != nodeNum - 1)
                    edge.set_target(i + 1);
                else
                    edge.set_target(0);
            } else {
                int source = rand.pick(0, nodeNum);
                int target = rand.pick(0, nodeNum);
                while (target == source || target == source - 1 || repectflag[source][target] == 1) {
                    target = rand.pick(0, nodeNum);
                }
                edge.set_source(source);
                edge.set_target(target);
            }
        } else//构造大算例
        {
            int temp2 = nodeNum + (int)(nodeNum * 0.2);
            if (i < nodeNum) {
                edge.set_source(i);
                if (i != nodeNum - 1)
                    edge.set_target(i + 1);
                else
                    edge.set_target(0);

            } else if (i <= temp2) {
                int j = 10;
                int source = rand.pick(2, nodeNum);
                while ((repectflag[source][0] == 1) || (repectflag[0][source] == 1)) {
                    source = rand.pick(2, nodeNum);
                }
                edge.set_source(source);
                edge.set_target(0);
            } else {
                int source = rand.pick(0, nodeNum);
                int target = rand.pick(0, nodeNum);
                while (repectflag[source][target] == 1 || repectflag[target][source] == 1) {
                    target = rand.pick(0, nodeNum);
                }
                edge.set_source(source);
                edge.set_target(target);
            }
        }
        repectflag[edge.source()][edge.target()] = 1;
    }

    //产生需求,每个需求的有效时间均为30min
    int requriedNum = rand.pick(trait.requiredNum.begin, trait.requiredNum.end);
    for (int r = 0; r != requriedNum; ++r) {
        auto &required(*input.add_noderequireds());
        //int Cost = rand.pick(trait.Cost.begin, trait.Cost.end);
        required.set_id(r);
        if (r < nodeNum) {//保证每个点至少一个需求
            required.set_nodeid(r);
            int moment = rand.pick(0, 30);//价值产生时间，衰减周期确定
            int  value = rand.pick(trait.Value.begin, trait.Value.end);
            int temp = 0;
            for (int t = moment, i = 0; i != 30; ++t, ++i) {
                auto &singleR(*required.add_valueatmoments());
                singleR.set_moment(t);
                temp = (int)(value - (value*i / 29));//递减
                singleR.set_value(temp);
            }
        } else {
            required.set_nodeid(rand.pick(nodeNum));
            int moment = rand.pick(0, trait.periodLength - 30);
            int  value = rand.pick(trait.Value.begin, trait.Value.end);
            int temp = 0;
            for (int t = moment, i = 0; i != 30; ++t, ++i) {
                auto &singleR(*required.add_valueatmoments());
                singleR.set_moment(t);
                temp = (int)(value - (value*i / 29));//递减
                singleR.set_value(temp);
            }
        }

    }


    ostringstream path;
    path << InstanceDir() << "rand.n" << input.nodeid().size()
        << "e" << input.edges().size()
        << "r" << input.noderequireds().size()
        << "p" << trait.periodLength << ".json";
    save(path.str(), input);
}
}

