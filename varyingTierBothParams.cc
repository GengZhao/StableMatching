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
    vector<int> tierSizesProp{25, 975}; // just placeholder, actually initialized below
    vector<int> tierSizesRec{1000};
    vector<double> scoresProp{1.0, 1.0};
    vector<double> scoresRec{1.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    int nStepsTierSize = 39;
    int nStepsScore = 39;

    int nThreads = 8;
    int nIter = 25;

    ofstream outFile;

    outFile.open("Outputs/varying_both_tier_params_rev-" + getTime() + ".out");
    for (int sAlpha = 0; sAlpha < nStepsScore; sAlpha++) {
        cout << "Tier size step:";
        tierSizesProp = vector<int>{25, 975};
        for (int sEpsilon = 0; sEpsilon < nStepsTierSize; sEpsilon++) {
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
                        Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
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

            tierSizesProp[0] += 25;
            tierSizesProp[1] -= 25;
            cout << " " << sEpsilon << flush;
        }
        scoresProp[1] += 0.25;
        cout << endl << "Scores step: " << sAlpha << endl;
    }
    outFile.close();

    return 0;
}
