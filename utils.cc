#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cassert>
#include <map>
#include <queue>
#include <numeric>
#include <iterator>

#include "utils.h"

using namespace std;

string getTime()
{
    const int MAXLEN = 80;
    char s[MAXLEN];
    time_t t = time(0);
    strftime(s, MAXLEN, "%Y-%m-%d_%H:%M:%S", localtime(&t));
    return string(s);
}

