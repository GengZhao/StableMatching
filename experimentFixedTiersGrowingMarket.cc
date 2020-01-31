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
    vector<int> tierSizesProp{4, 12};
    vector<int> tierSizesRec{1, 5, 10};
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{1.0, 2.0, 3.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    int nStepsInPow2 = 12;

    int nThreads = 8;
    int nIter = 50;

    ofstream outFile;
    outFile.open("fixed_tiers_growing_market-" + getTime() + ".out");
    for (int s = 0; s < nStepsInPow2; s++) {
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

        transform(tierSizesProp.begin(), tierSizesProp.end(),
                tierSizesProp.begin(), bind1st(multiplies<int>(), 2));
        transform(tierSizesRec.begin(), tierSizesRec.end(),
                tierSizesRec.begin(), bind1st(multiplies<int>(), 2));
    }
    outFile.close();
    return 0;
}
