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
    vector<int> tierSizesProp{50, 950};
    vector<int> defaultTierSizesProp{700, 300};
    vector<int> tierSizesRec{400, 600};
    vector<double> scoresProp{4.0, 1.0};
    vector<double> scoresRec{1.0, 1.0};
    vector<double> defaultScoresRec{2.5, 1.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    int nStepsTierSize = 19;
    int nStepsScore = 19;

    int nThreads = 8;
    int nIter = 6250;

    ofstream outFileTierSizes, outFileScores;

    // varying tier sizes
    outFileTierSizes.open("Outputs/fixed_market_varying_tier_sizes-" + getTime() + ".out");
    for (int s = 0; s < nStepsTierSize; s++) {
        outFileTierSizes << nTiersProp << "\n";
        printVector(tierSizesProp, outFileTierSizes);
        printVector(scoresProp, outFileTierSizes);
        outFileTierSizes << nTiersRec << "\n";
        printVector(tierSizesRec, outFileTierSizes);
        printVector(defaultScoresRec, outFileTierSizes);
        outFileTierSizes << nIter * nThreads << "\n";

        vector<vector<double> > avgRanksByPropTier(nTiersProp), avgRanksByRecTier(nTiersRec);
        for (vector<double>& v : avgRanksByPropTier) v.reserve(nIter * nThreads);
        for (vector<double>& v : avgRanksByRecTier) v.reserve(nIter * nThreads);
        mutex lockP, lockR;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, defaultScoresRec, false);
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
            printVector(v, outFileTierSizes);
        }
        for (vector<double>& v : avgRanksByRecTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFileTierSizes);
        }

        tierSizesProp[0] += 50;
        tierSizesProp[1] -= 50;
        cout << "Tier sizes step: " << s << endl;
    }
    outFileTierSizes.close();

    // varying scores
    outFileScores.open("Outputs/fixed_market_varying_scores-" + getTime() + ".out");
    for (int s = 0; s < nStepsScore; s++) {
        outFileScores << nTiersProp << "\n";
        printVector(defaultTierSizesProp, outFileScores);
        printVector(scoresProp, outFileScores);
        outFileScores << nTiersRec << "\n";
        printVector(tierSizesRec, outFileScores);
        printVector(scoresRec, outFileScores);
        outFileScores << nIter * nThreads << "\n";

        vector<vector<double> > avgRanksByPropTier(nTiersProp), avgRanksByRecTier(nTiersRec);
        for (vector<double>& v : avgRanksByPropTier) v.reserve(nIter * nThreads);
        for (vector<double>& v : avgRanksByRecTier) v.reserve(nIter * nThreads);
        mutex lockP, lockR;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, defaultTierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
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
            printVector(v, outFileScores);
        }
        for (vector<double>& v : avgRanksByRecTier) {
            assert(v.size() == (unsigned int) nIter * nThreads);
            printVector(v, outFileScores);
        }

        scoresRec[1] += 0.5;
        cout << "Scores step: " << s << endl;
    }
    outFileScores.close();

    return 0;
}
