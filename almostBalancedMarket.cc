#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <thread>
#include <mutex>
#include <string>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{32, 32};
    vector<int> tierSizesRec{64, 64};
    // tier size for receiving size will be initialized ad hoc
    vector<double> scoresProp{4.0, 1.0};
    vector<double> scoresRec{3.0, 1.0};
    int nTiersProp = scoresProp.size();
    int nTiersRec = scoresRec.size();

    int nStepsN = 15;

    int nThreads = 8;
    int nIter = 10;

    ofstream outFileAvgRank, outFileMatchedCount;
    string timeStr = getTime();
    outFileAvgRank.open("Outputs/almost_balanced_market_avg_rank-" + timeStr + ".out");
    outFileMatchedCount.open("Outputs/almost_balanced_market_matched_count-" + timeStr + ".out");
    for (int s = 0; s < nStepsN; s++) {

        outFileAvgRank << nTiersProp << "\n";
        printVector(tierSizesProp, outFileAvgRank);
        printVector(scoresProp, outFileAvgRank);
        outFileAvgRank << nTiersRec << "\n";
        printVector(tierSizesRec, outFileAvgRank);
        printVector(scoresRec, outFileAvgRank);
        outFileAvgRank << nIter * nThreads << "\n";

        outFileMatchedCount << nTiersProp << "\n";
        printVector(tierSizesProp, outFileMatchedCount);
        printVector(scoresProp, outFileMatchedCount);
        outFileMatchedCount << nTiersRec << "\n";
        printVector(tierSizesRec, outFileMatchedCount);
        printVector(scoresRec, outFileMatchedCount);
        outFileMatchedCount << nIter * nThreads << "\n";

        vector<vector<double> > avgRanksByPropTier(nTiersProp), avgRanksByRecTier(nTiersRec);
        vector<int> m11s, m21s;
        for (vector<double>& v : avgRanksByPropTier) v.reserve(nIter * nThreads);
        for (vector<double>& v : avgRanksByRecTier) v.reserve(nIter * nThreads);
        m11s.reserve(nIter * nThreads);
        m21s.reserve(nIter * nThreads);
        mutex lockP, lockR, lockMatchedCount;

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
                    lockMatchedCount.lock();
                    m11s.push_back(M.getMatchCountMatrix()[0][0]);
                    m21s.push_back(M.getMatchCountMatrix()[1][0]);
                    lockMatchedCount.unlock();
                }
            }));
        }
        for (thread& th : threads) th.join();

        for (vector<double>& v : avgRanksByPropTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFileAvgRank);
        }
        for (vector<double>& v : avgRanksByRecTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFileAvgRank);
        }
        assert(m11s.size() == (unsigned int) nIter * nThreads);
        assert(m21s.size() == (unsigned int) nIter * nThreads);
        printVector(m11s, outFileMatchedCount);
        printVector(m21s, outFileMatchedCount);

        cout << "Prop tier size (x2): " << tierSizesProp[0] << ", rec tier size (x2): " << tierSizesRec[0] << endl;
        transform(tierSizesProp.begin(), tierSizesProp.end(),
                tierSizesProp.begin(), bind1st(multiplies<int>(), 2));
        tierSizesRec[0] = tierSizesProp[0] * (2 * s + 3) / (2 * s + 2);
        tierSizesRec[1] = tierSizesRec[0];
    }
    outFileAvgRank.close();
    outFileMatchedCount.close();
    return 0;
}
