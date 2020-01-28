#include <iostream>
#include <fstream>
#include <vector>

#include "Matching.h"

using namespace std;

int main()
{
    vector<int> tierSizesProp{100000, 50000};
    vector<int> tierSizesRec{40000, 80000, 30000};
    vector<double> scoresProp{3.5, 1};
    vector<double> scoresRec{2.5, 1.5, 1};

    Matching M(2, 3, tierSizesProp, tierSizesRec, scoresProp, scoresRec, false);
    M.run();
    M.result();
    return 0;
}
