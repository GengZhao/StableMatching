#include <iostream>
#include <fstream>
#include <vector>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{4001, 2000};
    vector<int> tierSizesRec{5000, 1000};
    vector<double> scoresProp{4.0, 1.0};
    vector<double> scoresRec{1.0, 2.0};

    Matching M(2, 2, tierSizesProp, tierSizesRec, scoresProp, scoresRec, true, true);
    M.run();
    M.result();
    vector<vector<int> > uniqueMatches = M.reverseRunCountUniquePartners();
    cout << endl << "Unique matching pairs (core):" << endl;
    printVector2DTsv(uniqueMatches);
    return 0;
}
