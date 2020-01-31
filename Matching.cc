#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cassert>

#include "Matching.h"
#include "utils.h"

using namespace std;

Matching::Matching(
        const int nTiersProp,
        const int nTiersRec,
        const vector<int>& tierSizesProp,
        const vector<int>& tierSizesRec,
        const vector<double>& scoresProp,
        const vector<double>& scoresRec,
        const bool pregeneratePreferences,
        const bool savePreferences,
        const bool verbose) :
    rng(rd()), verbose(verbose),
    nTiersProp(nTiersProp), nTiersRec(nTiersRec),
    tierSizesProp(tierSizesProp), tierSizesRec(tierSizesRec),
    scoresProp(scoresProp), scoresRec(scoresRec),
    nAgentsProp(accumulate(tierSizesProp.begin(), tierSizesProp.end(), 0)),
    nAgentsRec(accumulate(tierSizesRec.begin(), tierSizesRec.end(), 0)),
    proposalCountMatrix(nTiersProp, vector<int>(nTiersRec, 0)),
    matchCountMatrix(nTiersProp, vector<int>(nTiersRec, 0)),
    numProposalsMadeByPropTier(nTiersProp, 0),
    numMatchesByPropTier(nTiersProp, 0),
    numMatchesByRecTier(nTiersRec, 0),
    totalNumProposals(0),
    pregeneratePreferences(pregeneratePreferences),
    savePreferences(savePreferences)
{
    assert(this->nTiersProp == (int) tierSizesProp.size());
    assert(this->nTiersRec == (int) tierSizesRec.size());
    assert(this->nTiersProp == (int) scoresProp.size());
    assert(this->nTiersRec == (int) scoresRec.size());

    bool longSideProposing = this->nAgentsProp > this->nAgentsRec;
    int indexProp = 0;
    for (int tp = 0; tp < nTiersProp; tp++) {
        for (int i = 0; i < tierSizesProp[tp]; i++) {
            Agent* prop = new Agent(indexProp, scoresProp[tp], tp, tierSizesRec, scoresRec, PROPOSER, longSideProposing || pregeneratePreferences, savePreferences, verbose);
            this->agentsProp.push_back(prop);
            this->agentsToPropose.push(prop);
            indexProp++;
        }
    }

    int indexRec = 0;
    for (int tr = 0; tr < nTiersRec; tr++) {
        for (int i = 0; i < tierSizesRec[tr]; i++) {
            Agent* rec = new Agent(indexRec, scoresRec[tr], tr, tierSizesProp, scoresProp, RECEIVER, pregeneratePreferences, savePreferences, verbose);
            this->agentsRec.push_back(rec);
            indexRec++;
        }
    }
}

Matching::~Matching()
{
    for (Agent* p : this->agentsProp) delete p;
    for (Agent* r : this->agentsRec) delete r;
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

// can only reverse run after a normal run
void Matching::reverseRun()
{
    assert(this->pregeneratePreferences && this->savePreferences);

    assert(this->agentsToPropose.empty());
    for (Agent* p : this->agentsProp) p->reverseRole();
    for (Agent* r : this->agentsRec) {
        r->reverseRole();
        this->agentsToPropose.push(r);
    }
    // rerun deferred acceptance
    while (!(this->agentsToPropose.empty())) {
        Agent* proposer = this->agentsToPropose.front();
        this->agentsToPropose.pop();
        Agent* receiver = proposer->propose(this->agentsProp, this->rng);
        if (receiver) {
            Agent* rejected = receiver->handleProposal(proposer, this->rng);
            if (rejected) this->agentsToPropose.push(rejected);
        }
    }
}

/** Result computation **/

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
                avgRankInTier += this->agentsRec[index]->rankOfPartnerForReceiver(this->rng) / (double) this->numMatchesByRecTier[tr];
            }
            index++;
        }
        avgRanks.push_back(avgRankInTier);
    }
    return avgRanks;
}

vector<vector<int> > Matching::reverseRunCountUniquePartners()
{
    this->reverseRun();
    vector<vector<int> > uniquePartners{(unsigned long) this->nTiersProp, vector<int>{this->nTiersRec, 0}};
    for (Agent* p : this->agentsProp) {
        if (p->hasUniqueMatch()) {
            uniquePartners[p->tier][p->matchedPartner()->tier]++;
        }
    }
    return uniquePartners;
}

void Matching::result()
{
    cout << "Result summary" << endl << "=============="  << endl;
    cout << "Proposer tier sizes:";
    printVectorTsv(this->tierSizesProp);
    cout << "Receiver tier sizes:";
    printVectorTsv(this->tierSizesRec);
    cout << "Proposer scores:";
    printVectorTsv(this->scoresProp);
    cout << "Receiver scores:";
    printVectorTsv(this->scoresRec);

    if (this->verbose) {
        cout << "Matched pairs:" << endl;
        for (Agent* p : this->agentsProp) {
            int matchedIndex = -1;
            int partnerRankOfPartner = -1;
            if (p->matchedPartner()) {
                matchedIndex = p->matchedPartner()->index;
                partnerRankOfPartner = p->matchedPartner()->rankOfPartnerForReceiver(this->rng);
            }
            cout << "(P)" << p->index << " - (R)" << matchedIndex
                 << " [Rank " << p->rankOfPartnerForProposer() << " and " << partnerRankOfPartner << "]" << endl;
        }
    }

    cout << endl << "Proposal counts:" << endl;
    printVector2DTsv(this->proposalCountMatrix);

    cout << endl << "Match counts:" << endl;
    printVector2DTsv(this->matchCountMatrix);

    cout << endl << "Proposal counts by proposer tier:" << endl;
    printVectorTsv(this->numProposalsMadeByPropTier);

    cout << endl << "Match counts by proposer tier:" << endl;
    printVectorTsv(this->numMatchesByPropTier);

    cout << endl << "Match counts by receiver tier:" << endl;
    printVectorTsv(this->numMatchesByRecTier);

    cout << endl << "Avg rank of match by proposer tier:" << endl;
    printVectorTsv(this->avgRankForProposerByTier());

    cout << endl << "Avg rank of match by receiver tier:" << endl;
    printVectorTsv(this->avgRankForReceiverByTier());

}
