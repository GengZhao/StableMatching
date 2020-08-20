#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <thread>
#include <mutex>
#include <string>

#include "Matching.h"
#include "utils.h"

using namespace std;

int main()
{
    double epsilon = 0.5;
    Matching M(
            1,                      // nTiersProp
            1,                      // nTiersRec
            vector<int>(1, 12),     // tierSizesProp
            vector<int>(1, 12),     // tierSizesRec
            vector<double>(1, 1.0), // scoresProp
            vector<double>(1, 1.0), // scoresRec
            true,                   // pregeneratePreferences
            true,                   // savePreferences
            false                   // verbose
        );
    M.completePreferences();
    vector<int> permutationOfReceiverIndices;
    for (int i = 0; i < 12; i++) {
        permutationOfReceiverIndices.push_back(i);
    }
    sort(permutationOfReceiverIndices.begin(), permutationOfReceiverIndices.end());
    do {
        M.matchByPermutation(permutationOfReceiverIndices);
        string error = M.sanityCheckMatchingWithStabilityCriterion(
                [epsilon](Agent* p, Agent* r, mt19937& rng) {
                    return p->invHappinessFor(r, rng) < p->invHappinessFor(p->matchedPartner(), rng) * (1 - epsilon) &&
                        r->invHappinessFor(p, rng) < r->invHappinessFor(r->matchedPartner(), rng) * (1 - epsilon);
                }
            );
        if (error.empty()) cout << "Found eps stable matching!" << endl;
    } while (next_permutation(permutationOfReceiverIndices.begin(), permutationOfReceiverIndices.end()));
    return 0;
}
