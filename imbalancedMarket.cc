#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <thread>
#include <mutex>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{300, 700};
    // tier size for receiving size will be initialized ad hoc
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{1.0};
    int nTiersProp = scoresProp.size();
    int nTiersRec = scoresRec.size();

    vector<int> tierSizeRecRange{990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010};

    int nThreads = 8;
    int nIter = 125;

    ofstream outFile;
    outFile.open("imbalanced_market-" + getTime() + ".out");
    for (int s : tierSizeRecRange) {
        vector<int> tierSizesRec{s};

        outFile << nTiersProp << "\n";
        printVector(tierSizesProp, outFile);
        printVector(scoresProp, outFile);
        outFile << nTiersRec << "\n";
        printVector(tierSizesRec, outFile);
        printVector(scoresRec, outFile);
        outFile << nIter * nThreads << "\n";

        vector<vector<double> > avgRanksByPropTier(nTiersProp), avgRanksByRecTier(nTiersRec);
        for (vector<double>& v : avgRanksByPropTier) v.reserve(nIter * nThreads);
        for (vector<double>& v : avgRanksByRecTier) v.reserve(nIter * nThreads);
        mutex lockP, lockR;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false, false);
                    M.run();
                    vector<double> avgRankProp = M.avgRankForProposerByTier();
                    vector<double> avgRankRec = M.avgRankForReceiverByTier();
                    for (int tp = 0; tp < nTiersProp; tp++) {
                        lockP.lock();
                        avgRanksByPropTier[tp].push_back(avgRankProp[tp]);
                        lockP.unlock();
                    }
                    for (int tr = 0; tr < nTiersRec; tr++) {
                        lockR.lock();
                        avgRanksByRecTier[tr].push_back(avgRankRec[tr]);
                        lockR.unlock();
                    }
                }
            }));
        }
        for (thread& th : threads) th.join();

        for (vector<double>& v : avgRanksByPropTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFile);
        }
        for (vector<double>& v : avgRanksByRecTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFile);
        }
        cout << "Receiving side size: " << s << endl;
    }
    outFile.close();
    return 0;
}
