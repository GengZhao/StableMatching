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
    vector<int> tierSizesProp{16, 16};
    vector<int> tierSizesRec{16, 16};
    vector<double> scoresProp{1.0, 3.0};
    vector<double> scoresRec{1.0, 5.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    int nStepsN = 15;
    int nStepsScores = 8;

    int nThreads = 8;
    int nIter = 25;

    ofstream outFileGrowN, outFileGrowScores;

    // increase size of market
    outFileGrowN.open("distribution_of_pairs_grow_market-" + getTime() + ".out");
    for (int s = 0; s < nStepsN; s++) {

        outFileGrowN << nTiersProp << "\n";
        printVector(tierSizesProp, outFileGrowN);
        printVector(scoresProp, outFileGrowN);
        outFileGrowN << nTiersRec << "\n";
        printVector(tierSizesRec, outFileGrowN);
        printVector(scoresRec, outFileGrowN);
        outFileGrowN << nIter * nThreads << "\n";

        vector<int> m11s;
        m11s.reserve(nIter * nThreads);
        mutex lock;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
                    M.run();
                    lock.lock();
                    m11s.push_back(M.getMatchCountMatrix()[0][0]);
                    lock.unlock();
                }
            }));
        }
        for (thread& th : threads) th.join();

        assert(m11s.size() == (unsigned int) nIter * nThreads);
        printVector(m11s, outFileGrowN);

        transform(tierSizesProp.begin(), tierSizesProp.end(),
                tierSizesProp.begin(), bind1st(multiplies<int>(), 2));
        transform(tierSizesRec.begin(), tierSizesRec.end(),
                tierSizesRec.begin(), bind1st(multiplies<int>(), 2));

        cout << "Market size step: " << s << endl;
    }
    outFileGrowN.close();

    // reset tier sizes and scores
    tierSizesProp = vector<int>{500, 500};
    tierSizesRec = vector<int>{500, 500};
    scoresProp = vector<double>{1.0, 1.0};
    scoresRec = vector<double>{1.0, 1.0};
    // varying scores
    outFileGrowScores.open("distribution_of_pairs_vary_scores-" + getTime() + ".out");
    for (int s1 = 0; s1 < nStepsScores; s1++) {
        for (int s2 = 0; s2 < nStepsScores; s2++) {
            outFileGrowScores << nTiersProp << "\n";
            printVector(tierSizesProp, outFileGrowScores);
            printVector(scoresProp, outFileGrowScores);
            outFileGrowScores << nTiersRec << "\n";
            printVector(tierSizesRec, outFileGrowScores);
            printVector(scoresRec, outFileGrowScores);
            outFileGrowScores << nIter * nThreads << "\n";

            vector<int> m11s;
            m11s.reserve(nIter * nThreads);
            mutex lock;

            vector<thread> threads;
            for (int th = 0; th < nThreads; th++) {
                threads.push_back(thread([&] {
                    for (int it = 0; it < nIter; it++) {
                        Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
                        M.run();
                        lock.lock();
                        m11s.push_back(M.getMatchCountMatrix()[0][0]);
                        lock.unlock();
                    }
                }));
            }
            for (thread& th : threads) th.join();

            assert(m11s.size() == (unsigned int) nIter * nThreads);
            printVector(m11s, outFileGrowScores);

            scoresProp[1] += 1;
            scoresRec[1] += 1;
            cout << "Scores step: " << s1 << ", " << s2 << endl;
        }
    }
    outFileGrowScores.close();

    return 0;
}
