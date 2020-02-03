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
    vector<int> origTierSizesProp{20, 60};
    vector<int> origTierSizesRec{5, 25, 50};
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{1.0, 2.0, 3.0};
    int nTiersProp = scoresProp.size();
    int nTiersRec = scoresRec.size();

    int nSteps = 125;

    int nThreads = 8;
    int nIter = 125;

    ofstream outFile;
    outFile.open("fixed_tiers_growing_market_lin-" + getTime() + ".out");
    for (int s = 0; s < nSteps; s++) {
        vector<int> tierSizesProp;
        vector<int> tierSizesRec;
        transform(origTierSizesProp.begin(), origTierSizesProp.end(),
                back_inserter(tierSizesProp), bind1st(multiplies<int>(), s + 1));
        transform(origTierSizesRec.begin(), origTierSizesRec.end(),
                back_inserter(tierSizesRec), bind1st(multiplies<int>(), s + 1));

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
        cout << "Step: " << s << endl;
    }
    outFile.close();
    return 0;
}
