#include <iostream>
#include <fstream>
#include <vector>

#include "Matching.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{200000, 100000};
    vector<int> tierSizesRec{250000, 50000};
    vector<double> scoresProp{3.0, 1.0};
    vector<double> scoresRec{1.0, 2.0};

    Matching M(2, 2, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
    M.run();
    M.result();
    return 0;
}
