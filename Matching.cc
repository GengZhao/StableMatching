#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cassert>

#include "Matching.h"

using namespace std;

Matching::Matching(
        int nTiersProp,
        int nTiersRec,
        vector<int> tierSizesProp,
        vector<int> tierSizesRec,
        vector<double> scoresProp,
        vector<double> scoresRec,
        bool verbose) :
    rng(rd()),
    proposalCountMatrix(nTiersProp, vector<int>(nTiersRec, 0)),
    matchCountMatrix(nTiersProp, vector<int>(nTiersRec, 0)),
    numProposalsMadeByPropTier(nTiersProp, 0),
    numMatchesByPropTier(nTiersProp, 0),
    numMatchesByRecTier(nTiersRec, 0),
    totalNumProposals(0)
{
    this->nTiersProp = nTiersProp;
    this->nTiersRec = nTiersRec;
    this->tierSizesProp = tierSizesProp;
    this->tierSizesRec = tierSizesRec;
    this->scoresProp = scoresProp;
    this->scoresRec = scoresRec;
    this->verbose = verbose;

    assert(this->nTiersProp == (int) tierSizesProp.size());
    assert(this->nTiersRec == (int) tierSizesRec.size());
    assert(this->nTiersProp == (int) scoresProp.size());
    assert(this->nTiersRec == (int) scoresRec.size());

    this->nAgentsProp = accumulate(tierSizesProp.begin(), tierSizesProp.end(), 0);
    this->nAgentsRec = accumulate(tierSizesRec.begin(), tierSizesRec.end(), 0);
    bool longSideProposing = this->nAgentsProp > this->nAgentsRec;
    int indexProp = 0;
    for (int tp = 0; tp < nTiersProp; tp++) {
        for (int i = 0; i < tierSizesProp[tp]; i++) {
            Agent* prop = new Agent(indexProp, scoresProp[tp], tp, tierSizesRec, scoresRec, PROPOSER, longSideProposing, verbose);
            this->agentsProp.push_back(prop);
            this->agentsToPropose.push(prop);
            indexProp++;
        }
    }

    int indexRec = 0;
    for (int tr = 0; tr < nTiersRec; tr++) {
        for (int i = 0; i < tierSizesRec[tr]; i++) {
            Agent* rec = new Agent(indexRec, scoresRec[tr], tr, tierSizesProp, scoresProp, RECEIVER, false, verbose);
            this->agentsRec.push_back(rec);
            indexRec++;
        }
    }
}

Matching::~Matching()
{
    for (Agent* p : this->agentsProp) {
        delete p;
    }
    for (Agent* r : this->agentsRec) {
        delete r;
    }
}

void Matching::run()
{
    while (!(this->agentsToPropose.empty())) {
        Agent* proposer = this->agentsToPropose.front();
        this->agentsToPropose.pop();
        Agent* receiver = proposer->propose(this->agentsRec, this->rng);
        if (receiver) {
            this->proposalCountMatrix[proposer->tier][receiver->tier]++;
            this->numProposalsMadeByPropTier[proposer->tier]++;
            this->totalNumProposals++;
            Agent* rejected = receiver->handleProposal(proposer, this->rng);
            if (rejected) {
                this->agentsToPropose.push(rejected);
                if (rejected->tier != proposer->tier) {
                    this->matchCountMatrix[proposer->tier][receiver->tier]++;
                    this->matchCountMatrix[rejected->tier][receiver->tier]--;
                    this->numMatchesByPropTier[proposer->tier]++;
                    this->numMatchesByPropTier[rejected->tier]--;
                }
            } else {
                this->matchCountMatrix[proposer->tier][receiver->tier]++;
                this->numMatchesByPropTier[proposer->tier]++;
                this->numMatchesByRecTier[receiver->tier]++;
            }
        }
    }
}

template<typename T>
static void printVec(const vector<T>& vec)
{
    for (typename vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
        cout << "\t" << *it;
    }
    cout << endl;
}

template<typename T>
static void printVec2D(const vector<T>& vec2d)
{
    for (typename vector<T>::const_iterator it = vec2d.begin(); it != vec2d.end(); ++it) {
        printVec(*it);
    }
}

vector<double> Matching::avgRankForProposerByTier()
{
    vector<double> avgRanks;
    for (int tp = 0; tp < this->nTiersProp; tp++) {
        avgRanks.push_back((this->numProposalsMadeByPropTier[tp] -
                    (this->tierSizesProp[tp] - this->numMatchesByPropTier[tp]) * this->nAgentsRec) /
                (double) this->numMatchesByPropTier[tp]);
    }
    return avgRanks;
}

vector<double> Matching::avgRankForReceiverByTier()
{
    vector<double> avgRanks;
    int index = 0;
    for (int tr = 0; tr < this->nTiersRec; tr++) {
        if (this->numMatchesByRecTier[tr] == 0) {
            avgRanks.push_back(0.0);
            cerr << "[WARNING] Receiving side tier " << tr << " has received no proposals." << endl;
            index += this->tierSizesRec[tr];
            continue;
        }
        double avgRankInTier = 0.0;
        for (int i = 0; i < this->tierSizesRec[tr]; i++) {
            if (this->agentsRec[index]->matchedPartner()) {
                avgRankInTier += this->agentsRec[index]->rankOfPartnerForReceiver(this->agentsProp, this->rng) / (double) this->numMatchesByRecTier[tr];
            }
            index++;
        }
        avgRanks.push_back(avgRankInTier);
    }
    return avgRanks;
}

void Matching::result()
{
    cout << "Result summary" << endl << "=============="  << endl;
    cout << "Proposer tier sizes:";
    printVec(this->tierSizesProp);
    cout << "Receiver tier sizes:";
    printVec(this->tierSizesRec);
    cout << "Proposer scores:";
    printVec(this->scoresProp);
    cout << "Receiver scores:";
    printVec(this->scoresRec);

    if (this->verbose) {
        cout << "Matched pairs:" << endl;
        for (Agent* p : this->agentsProp) {
            int matchedIndex = -1;
            int partnerRankOfPartner = -1;
            if (p->matchedPartner()) {
                matchedIndex = p->matchedPartner()->index;
                partnerRankOfPartner = p->matchedPartner()->rankOfPartnerForReceiver(this->agentsProp, this->rng);
            }
            cout << "(P)" << p->index << " - (R)" << matchedIndex
                 << " [Rank " << p->rankOfPartnerForProposer() << " and " << partnerRankOfPartner << "]" << endl;
        }
    }

    cout << endl << "Proposal counts:" << endl;
    printVec2D(this->proposalCountMatrix);

    cout << endl << "Match counts:" << endl;
    printVec2D(this->matchCountMatrix);

    cout << endl << "Proposal counts by proposer tier:" << endl;
    printVec(this->numProposalsMadeByPropTier);

    cout << endl << "Match counts by proposer tier:" << endl;
    printVec(this->numMatchesByPropTier);

    cout << endl << "Match counts by receiver tier:" << endl;
    printVec(this->numMatchesByRecTier);

    cout << endl << "Avg rank of match by proposer tier:" << endl;
    printVec(this->avgRankForProposerByTier());

    cout << endl << "Avg rank of match by receiver tier:" << endl;
    printVec(this->avgRankForReceiverByTier());

}
