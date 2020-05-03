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
    outFile.open("Outputs/unique_partner_count-" + getTime() + ".out");
    for (int s : tierSizeRecRange) {
        vector<int> tierSizesRec{s};

        outFile << nTiersProp << "\n";
        printVector(tierSizesProp, outFile);
        printVector(scoresProp, outFile);
        outFile << nTiersRec << "\n";
        printVector(tierSizesRec, outFile);
        printVector(scoresRec, outFile);
        outFile << nIter * nThreads << "\n";

        vector<int> uniquePartnerCountTier1, uniquePartnerCountTier2; // hardcoded 2 tiers
        mutex lock1, lock2;

        vector<thread> threads;
        for (int th = 0; th < nThreads; th++) {
            threads.push_back(thread([&] {
                for (int it = 0; it < nIter; it++) {
                    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, true, true);
                    M.run();
                    vector<vector<int> > uniqueMatches = M.reverseRunCountUniquePartners();
                    lock1.lock();
                    uniquePartnerCountTier1.push_back(uniqueMatches[0][0]);
                    lock1.unlock();
                    lock2.lock();
                    uniquePartnerCountTier2.push_back(uniqueMatches[1][0]);
                    lock2.unlock();
                }
            }));
        }
        for (thread& th : threads) th.join();

        assert(uniquePartnerCountTier1.size() == (unsigned int) nIter * nThreads);
        printVector(uniquePartnerCountTier1, outFile);
        assert(uniquePartnerCountTier2.size() == (unsigned int) nIter * nThreads);
        printVector(uniquePartnerCountTier2, outFile);
        cout << "Receiving side size: " << s << endl;
    }
    outFile.close();
    return 0;
}
