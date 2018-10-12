#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//#include "Visualizer.h"

#include "../Solver/PbReader.h"
#include "../Solver/TravelingPurchase.pb.h"

using namespace std;
using namespace pb;

int main(int argc, char *argv[]) {
    enum CheckerFlag {
        IoError = 0x0,
        FormatError = 0x1,
        DisconnectedError = 0x2,
        MinTimeError = 0x4,
        TotalTimeError = 0x8,
        //TotalValueError = 0x16
    };

    string inputPath;
    string outputPath;

    if (argc > 1) {
        inputPath = argv[1];
    } else {
        cerr << "input path: " << flush;
        cin >> inputPath;
    }

    if (argc > 2) {
        outputPath = argv[2];
    } else {
        cerr << "output path: " << flush;
        cin >> outputPath;
    }
    pb::TravelingPurchase::Input input;
    if (!load(inputPath, input)) { return ~CheckerFlag::IoError; }

    pb::TravelingPurchase::Output output;
    ifstream ifs(outputPath);
    if (!ifs.is_open()) { return ~CheckerFlag::IoError; }

    int periodlength;
    string temp = "";
    for (int i = 0; i != inputPath.size(); ++i) {
        if (inputPath[i] == 'p') {
            i++;
            while (inputPath[i] != '.') {
                temp += inputPath[i];
                ++i;
            }
            break;
        }
    }
    periodlength = atoi(temp.c_str());

    string submission;
    getline(ifs, submission); // skip the first line.
    ostringstream oss;
    oss << ifs.rdbuf();
    jsonToProtobuf(oss.str(), output);

    // check solution.
    int error = 0;

    for (auto temp = output.nodeidatmoment().begin(); temp != output.nodeidatmoment().end(); ++temp) {

        if (temp->nodeid() < 0 || temp->nodeid() >= input.nodeid().size() || temp->moment() >= input.periodlength() || output.nodeidatmoment().begin()->nodeid() == input.sourcenode() || output.nodeidatmoment().begin()->nodeid() == input.targetnode()) {
            error |= CheckerFlag::FormatError;
        }
    }
    //check connectivity.
    int totalValue = 0;
    for (auto temp = output.nodeidatmoment().begin(); temp != output.nodeidatmoment().end() - 1; ++temp) {
        int flag1 = 0, flag2 = 1;
        for (auto in = input.edges().begin(); in != input.edges().end(); ++in) {
            if ((temp->nodeid() == in->source() && (temp + 1)->nodeid() == in->target()) || (temp->nodeid() == in->target() && (temp + 1)->nodeid() == in->source())) {
                flag1 = 1;
                if (((temp + 1)->moment() - temp->moment()) < in->mintime()) flag2 = 0;
            }
        }
        if (flag1 == 0) {
            cerr << "path " << temp->nodeid() << " to " << (temp + 1)->nodeid() << " is not connected!!" << endl;
            error |= CheckerFlag::DisconnectedError;

        }
        //check time.
        if (flag2 == 0) {
            cerr << "path " << temp->nodeid() << " to " << (temp + 1)->nodeid() << " is less than the minimum time!!" << endl;
            error |= CheckerFlag::MinTimeError;
        }

        //check object.
        vector<int> reFlag(input.noderequireds().size(), 0);
        for (auto re = input.noderequireds().begin(); re != input.noderequireds().end(); ++re) {
            if (temp->nodeid() == re->nodeid() && reFlag[re->id()] != 1)
                for (auto vam = re->valueatmoments().begin(); vam != re->valueatmoments().end(); ++vam) {
                    if (temp->moment() == vam->moment()) {
                        totalValue += vam->value();
                        reFlag[re->id()] = 1;//标记已被访问的需求
                        break;
                    } else if (temp->moment() <= vam->moment()) {
                        break;
                    }

                }
        }
        //error |= CheckerFlag::TotalValueError;

    }
    //check period.
    if ((output.nodeidatmoment().end() - 1)->moment() > periodlength)
        error |= CheckerFlag::TotalTimeError;

    int returnCode = (error == 0) ? totalValue : ~error;
    cout << returnCode << endl;
    return returnCode;
}
