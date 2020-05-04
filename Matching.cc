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
    pregeneratePreferences(pregeneratePreferences),
    savePreferences(savePreferences),
    recordingProposalCounts(true),
    totalNumProposals(0)
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
    assert(this->agentsToPropose.empty());
    for (Agent* p : this->agentsProp) this->agentsToPropose.push(p);

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

    for (Agent* r : this->agentsRec) {
        if (!r->matchedPartner()) {
            r->markOptimal(); // these will never be matched (ref. rural hospital)
        } else {
            this->suboptimalReceivers.push(r);
        }
    }
}

// can only run from already stable state
bool Matching::runFromCurrent()
{
    assert(this->pregeneratePreferences && this->savePreferences);
    this->recordingProposalCounts = false;

    RejectionChain rejectionChain; // TODO: preserve and reuse this
    while (!(this->suboptimalReceivers.empty())) {
        this->stashAll(); // save previous matching

        Agent* suboptimalReceiver = this->suboptimalReceivers.front();

        Agent* rejected = suboptimalReceiver->rejectMatched();
        assert(rejected);
        rejectionChain.add(suboptimalReceiver, rejected);
        while (!(rejectionChain.empty())) {
            Agent* proposer = rejectionChain.nextProposer();
            while (true) {
                Agent* receiver = proposer->propose(this->agentsRec, this->rng);
                if (receiver && !receiver->isOptimal()) {
                    Agent* rejected = receiver->handleProposal(proposer, this->rng, true);
                    if (rejected == proposer) { // failed proposal
                        receiver->reject(proposer);
                        continue;
                    }
                    if (rejectionChain.contains(receiver)) {
                        // found a cycle
                        int position = rejectionChain.positionOf(receiver);
                        for (int pos = 0; pos < position; pos++) {
                            RejectionChainEntry entry = rejectionChain.at(pos);
                            entry.fromRejecter->stashPop();
                            entry.toRejectee->stashPop();
                        }
                        receiver->stashPop();
                        Agent* nextNextInChain = receiver->matchedPartner()->matchedPartner();
                        Agent* nextInChain = receiver->handleProposal(proposer, this->rng); // actually run the proposal
                        nextInChain->matchWith(nextNextInChain); // hack since we mis-rejected this agent in handling the proposal above
                        return true;
                    }
                    receiver->handleProposal(proposer, this->rng); // actually run the proposal
                    rejectionChain.add(receiver, rejected);
                    break;
                } else {
                    // proposed to someone already at optimum
                    // => cannot find a new stable matching
                    this->stashPopAll(); // restore previous matching
                    suboptimalReceiver->markOptimal();
                    this->suboptimalReceivers.pop();
                    break;
                }
            }
        }

        /*
        this->agentsToPropose.push(rejected);

        while (!(this->agentsToPropose.empty())) {
            Agent* proposer = this->agentsToPropose.front();
            this->agentsToPropose.pop();
            Agent* receiver = proposer->propose(this->agentsRec, this->rng);
            if (receiver && !receiver->isOptimal()) {
                Agent* rejected = receiver->handleProposal(proposer, this->rng);
                if (rejected) { // including when proposer is worse than the previous match for the initial suboptimalReceiver
                    this->agentsToPropose.push(rejected);
                } else {
                    // the initial suboptimalReceiver finds a better match, i.e. rejection chain complete
                    // => a new stable matching
                    return true;
                }
            } else {
                // proposed to someone already at optimum
                // => cannot find a new stable matching
                this->stashPopAll(); // restore previous matching
                suboptimalReceiver->markOptimal();
                this->suboptimalReceivers.pop();
            }
        }
        */
    }
    return false;
}

// can only reverse run after a normal run
void Matching::reverseRun()
{
    assert(this->pregeneratePreferences && this->savePreferences);
    this->recordingProposalCounts = false;

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
            if (rejected) {
                this->agentsToPropose.push(rejected);
                if (rejected->tier != proposer->tier) {
                    this->matchCountMatrix[receiver->tier][proposer->tier]++;
                    this->matchCountMatrix[receiver->tier][rejected->tier]--;
                    this->numMatchesByRecTier[proposer->tier]++;
                    this->numMatchesByRecTier[rejected->tier]--;
                }
            } else {
                this->matchCountMatrix[receiver->tier][proposer->tier]++;
                this->numMatchesByPropTier[receiver->tier]++;
                this->numMatchesByRecTier[proposer->tier]++;
            }
        }
    }
}

void Matching::runExperimental()
{
    assert(this->pregeneratePreferences && this->savePreferences);
    this->recordingProposalCounts = false;

    // forward proposing for tier 1 proposers
    for (int i = 0; i < this->tierSizesProp[0]; i++) {
        this->agentsToPropose.push(this->agentsProp[i]);
    }

    // run regular deferred acceptance for tier 1
    while (!(this->agentsToPropose.empty())) {
        Agent* proposer = this->agentsToPropose.front();
        this->agentsToPropose.pop();
        Agent* receiver = proposer->propose(this->agentsRec, this->rng);
        if (receiver) {
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

    // reverse proposing direction
    for (Agent* p : this->agentsProp) p->reverseRole(true); // preserve currently matched partner
    for (Agent* r : this->agentsRec) {
        if (!(r->matchedPartner())) this->agentsToPropose.push(r);
        r->reverseRole(true); // preserve currently matched partner
    }

    // rerun deferred acceptance
    while (!(this->agentsToPropose.empty())) {
        Agent* proposer = this->agentsToPropose.front();
        this->agentsToPropose.pop();
        Agent* receiver = proposer->propose(this->agentsProp, this->rng);
        if (receiver) {
            Agent* rejected = receiver->handleProposal(proposer, this->rng);
            if (rejected) {
                this->agentsToPropose.push(rejected);
                if (rejected->tier != proposer->tier) {
                    this->matchCountMatrix[receiver->tier][proposer->tier]++;
                    this->matchCountMatrix[receiver->tier][rejected->tier]--;
                    this->numMatchesByRecTier[proposer->tier]++;
                    this->numMatchesByRecTier[rejected->tier]--;
                }
            } else {
                this->matchCountMatrix[receiver->tier][proposer->tier]++;
                this->numMatchesByPropTier[receiver->tier]++;
                this->numMatchesByRecTier[proposer->tier]++;
            }
        }
    }
}

void Matching::stashAll()
{
    for (Agent* p : this->agentsProp) p->stash();
    for (Agent* r : this->agentsRec) r->stash();
}

void Matching::stashPopAll()
{
    for (Agent* p : this->agentsProp) p->stashPop();
    for (Agent* r : this->agentsRec) r->stashPop();
}

void Matching::resetState()
{
    assert(this->agentsToPropose.empty());
    for (Agent* p : this->agentsProp) p->reset();
    for (Agent* r : this->agentsRec) r->reset();

    this->proposalCountMatrix = vector<vector<int> >(this->nTiersProp, vector<int>(this->nTiersRec, 0));
    this->matchCountMatrix = vector<vector<int> >(this->nTiersProp, vector<int>(this->nTiersRec, 0));
    this->numProposalsMadeByPropTier = vector<int>(this->nTiersProp, 0);
    this->numMatchesByPropTier = vector<int>(this->nTiersProp, 0);
    this->numMatchesByRecTier = vector<int>(this->nTiersRec, 0);
    this->recordingProposalCounts = true;
    this->totalNumProposals = 0;
}

/** Result computation **/

vector<double> Matching::avgRankForProposerByTier()
{
    vector<double> avgRanks;
    if (this->recordingProposalCounts) {
        for (int tp = 0; tp < this->nTiersProp; tp++) {
            avgRanks.push_back((this->numProposalsMadeByPropTier[tp] -
                        (this->tierSizesProp[tp] - this->numMatchesByPropTier[tp]) * this->nAgentsRec) /
                    (double) this->numMatchesByPropTier[tp]);
        }
    } else { // not recording proposals, then preferences must be pregenerated
        int index = 0;
        for (int tp = 0; tp < this->nTiersProp; tp++) {
            if (this->numMatchesByPropTier[tp] == 0) {
                avgRanks.push_back(0.0);
                cerr << "[WARNING] Proposing side tier " << tp << " has received no proposals." << endl;
                index += this->tierSizesProp[tp];
                continue;
            }
            double avgRankInTier = 0.0;
            for (int i = 0; i < this->tierSizesProp[tp]; i++) {
                if (this->agentsProp[index]->matchedPartner()) {
                    avgRankInTier += this->agentsProp[index]->rankOfPartnerForProposer() / (double) this->numMatchesByPropTier[tp];
                }
                index++;
            }
            avgRanks.push_back(avgRankInTier);
        }
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

vector<vector<int> > Matching::getMatchCountMatrix()
{
    return this->matchCountMatrix;
}

void Matching::printNProposalsRec(ostream& os)
{
    for (Agent* r : this->agentsRec) {
        if (r->matchedPartner()) {
            os << r->numProposalsReceived() << endl;
        } else {
            os << -1 << endl;
        }
    }
}

void Matching::printRanksRec(ostream& os)
{
    for (Agent* r : this->agentsRec) {
        if (r->matchedPartner()) {
            os << r->rankOfPartnerForReceiver(this->rng) << endl;
        } else {
            os << -1 << endl;
        }
    }
}

void Matching::printMatchSetupInfo()
{
    cout << "Proposer tier sizes:";
    printVectorTsv(this->tierSizesProp);
    cout << "Receiver tier sizes:";
    printVectorTsv(this->tierSizesRec);
    cout << "Proposer scores:";
    printVectorTsv(this->scoresProp);
    cout << "Receiver scores:";
    printVectorTsv(this->scoresRec);
}

void Matching::result()
{
    cout << "Result summary" << endl << "=============="  << endl;
    this->printMatchSetupInfo();

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

    if (this->recordingProposalCounts) {
        cout << endl << "Proposal counts:" << endl;
        printVector2DTsv(this->proposalCountMatrix);
    }

    cout << endl << "Match counts:" << endl;
    printVector2DTsv(this->matchCountMatrix);

    if (this->recordingProposalCounts) {
        cout << endl << "Proposal counts by proposer tier:" << endl;
        printVectorTsv(this->numProposalsMadeByPropTier);
    }

    cout << endl << "Match counts by proposer tier:" << endl;
    printVectorTsv(this->numMatchesByPropTier);

    cout << endl << "Match counts by receiver tier:" << endl;
    printVectorTsv(this->numMatchesByRecTier);

    cout << endl << "Avg rank of match by proposer tier:" << endl;
    printVectorTsv(this->avgRankForProposerByTier());

    cout << endl << "Avg rank of match by receiver tier:" << endl;
    printVectorTsv(this->avgRankForReceiverByTier());
}

