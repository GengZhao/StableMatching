#include <iostream>
#include <ctime>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>

std::string getTime();

template<typename T>
void printVector(const std::vector<T>& vec, std::ostream& os) {
    std::copy(vec.cbegin(), vec.cend(), std::ostream_iterator<T>(os, "\n"));
}

