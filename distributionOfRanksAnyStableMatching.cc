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
    vector<int> tierSizesProp{2048, 2048};
    vector<int> tierSizesRec{2048, 2048};
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{5.0, 1.0};
    int nTiersProp = tierSizesProp.size();
    int nTiersRec = tierSizesRec.size();

    ofstream outFile;
    outFile.open("Outputs/distribution_of_ranks_any_stable_matching-" + getTime() + ".out");

    Matching M(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec, true, true);
    M.run();
    int nMatchings = 1;
    do {
        outFile << nTiersProp << "\n";
        printVector(tierSizesProp, outFile);
        printVector(scoresProp, outFile);
        outFile << nTiersRec << "\n";
        printVector(tierSizesRec, outFile);
        printVector(scoresRec, outFile);
        outFile << 1 << "\n"; // iterations

        M.printRanks(PROPOSER, outFile);
        M.printRanks(RECEIVER, outFile);
        M.printInvHappinesses(PROPOSER, outFile);
        M.printInvHappinesses(RECEIVER, outFile);

        cout << "\rStable matchings found: " << ++nMatchings << flush;
    } while (M.runFromCurrent());

    cout << endl;

    outFile.close();

    return 0;
}
