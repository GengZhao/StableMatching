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
    vector<int> tierSizesProp{400, 600};
    vector<int> tierSizesRec{50, 950};
    vector<int> defaultTierSizesRec{700, 300};
    vector<double> scoresProp{1.0, 16.0};
    vector<double> defaultScoresProp{2.5, 1.0};
    vector<double> scoresRec{4.0, 1.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    int nStepsTierSize = 19;
    int nStepsScore = 17;

    int nThreads = 8;
    int nIter = 6250;

    ofstream outFileTierSizes, outFileScores;

    // varying tier sizes
    outFileTierSizes.open("fixed_market_varying_tier_sizes-" + getTime() + ".out");
    for (int s = 0; s < nStepsTierSize; s++) {
        outFileTierSizes << nTiersProp << "\n";
        printVector(tierSizesProp, outFileTierSizes);
        printVector(defaultScoresProp, outFileTierSizes);
        outFileTierSizes << nTiersRec << "\n";
        printVector(tierSizesRec, outFileTierSizes);
        printVector(scoresRec, outFileTierSizes);
        outFileTierSizes << nIter * nThreads << "\n";

        vector<vector<double> > avgRanksByPropTier(nTiersProp), avgRanksByRecTier(nTiersRec);
        for (vector<double>& v : avgRanksByPropTier) v.reserve(nIter * nThreads);
        for (vector<double>& v : avgRanksByRecTier) v.reserve(nIter * nThreads);
        mutex lockP, lockR;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, defaultScoresProp, scoresRec, false);
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

        tierSizesRec[0] += 50;
        tierSizesRec[1] -= 50;
        cout << "Tier sizes step: " << s << endl;
    }
    outFileTierSizes.close();

    // varying scores
    outFileScores.open("fixed_market_varying_scores-" + getTime() + ".out");
    for (int s = 0; s < nStepsScore; s++) {
        outFileScores << nTiersProp << "\n";
        printVector(tierSizesProp, outFileScores);
        printVector(scoresProp, outFileScores);
        outFileScores << nTiersRec << "\n";
        printVector(defaultTierSizesRec, outFileScores);
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
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, defaultTierSizesRec, scoresProp, scoresRec, false);
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

        scoresProp[1] /= sqrt(2.0);
        cout << "Scores step: " << s << endl;
    }
    outFileScores.close();

    return 0;
}
