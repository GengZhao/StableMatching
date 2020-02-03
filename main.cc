#include <iostream>
#include <fstream>
#include <vector>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{500, 500};
    vector<int> tierSizesRec{500, 500};
    vector<double> scoresProp{4.0, 1.0};
    vector<double> scoresRec{4.0, 1.0};

    Matching M(2, 2, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false, false);
    M.run();
    M.result();
    // vector<vector<int> > uniqueMatches = M.reverseRunCountUniquePartners();
    // cout << endl << "Unique matching pairs (core):" << endl;
    // printVector2DTsv(uniqueMatches);
    return 0;
}
