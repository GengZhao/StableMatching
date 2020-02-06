#include <iostream>
#include <fstream>
#include <vector>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{800, 800};
    vector<int> tierSizesRec{100, 500, 1000};
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{1.0, 2.0, 3.0};

    // printVectorTsv(tierSizesProp, cout);
    // printVectorTsv(tierSizesRec, cout);
    // printVectorTsv(scoresProp, cout);
    // printVectorTsv(scoresRec, cout);

    // for (int i = 0; i < 4; i++) {
        // Matching M(2, 3, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false, false);
        // M.run();
        // cout << M.totalNumProposals << endl;
    // }
    Matching M(2, 3, tierSizesProp, tierSizesRec, scoresProp, scoresRec, true, true);

    M.runExperimental();
    M.result();

    M.resetState();
    M.run();
    M.result();

    M.resetState();
    M.reverseRun();
    M.result();

    // vector<vector<int> > uniqueMatches = M.reverseRunCountUniquePartners();
    // cout << endl << "Unique matching pairs (core):" << endl;
    // printVector2DTsv(uniqueMatches);
    return 0;
}
