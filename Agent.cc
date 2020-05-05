#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <cmath>
#include <cassert>
#include <random>
#include <limits>

#include "Agent.h"

using namespace std;

Agent::Agent(
        const int index,
        const double score,
        const int tier,
        const vector<int> partnerSideTierSizes,
        const vector<double> partnerSideScores,
        const Role role,
        const bool pregeneratePreferences,
        const bool savePreferences,
        const bool verbose) :
    verbose(verbose),
    partnerSideTierSizes(partnerSideTierSizes),
    partnerSideScores(partnerSideScores),
    partnerSideNAgents(accumulate(partnerSideTierSizes.begin(), partnerSideTierSizes.end(), 0)),
    partnerSideNTiers(partnerSideTierSizes.size()),
    role(role), roleReversed(false),
    pregeneratePreferences(pregeneratePreferences), savePreferences(savePreferences),
    sumScoresForPool(inner_product(partnerSideTierSizes.begin(), partnerSideTierSizes.end(),
                partnerSideScores.begin(), 0.0)),
    poolSize(partnerSideNAgents),
    curPartner(NULL),
    poolSizesByTier(partnerSideTierSizes),
    invHappiness(numeric_limits<double>::max()),
    optimal(false),
    simulatedRankOfPartner(0),
    preferencesCompleted(false),
    index(index), score(score), tier(tier)
{
    // if long side proposes or if proposals run long, pre-generate all preferences by exp distribution for proposers
    if (pregeneratePreferences) {
        random_device rd{};
        default_random_engine generator{rd()};
        int pindex = 0;
        switch (this->role)
        {
            case PROPOSER:
                for (int t = 0; t < partnerSideNTiers; t++) {
                    exponential_distribution<double> distribution(partnerSideScores[t]);
                    for (int i = 0; i < partnerSideTierSizes[t]; i++) {
                        PreferenceEntry pe { pindex, distribution(generator) };
                        this->preferences.push_back(pe);
                        pindex++;
                    }
                }
                sort(this->preferences.begin(), this->preferences.end(), [](PreferenceEntry p1, PreferenceEntry p2) { return p1.invHappiness < p2.invHappiness; });
                break;
            case RECEIVER:
                for (int t = 0; t < partnerSideNTiers; t++) {
                    exponential_distribution<double> distribution(partnerSideScores[t]);
                    for (int i = 0; i < partnerSideTierSizes[t]; i++) {
                        this->invHappinessForPartners[pindex] = distribution(generator);
                        pindex++;
                    }
                }
                break;
        }
    }
}

void Agent::completePreferences()
{
    if (this->preferencesCompleted) return;
    switch (this->role)
    {
        case PROPOSER:
            assert(this->pregeneratePreferences);
            for (const PreferenceEntry& pe : this->preferences) {
                this->invHappinessForPartners[pe.index] = pe.invHappiness;
            }
            break;
        case RECEIVER:
            assert(this->savePreferences || this->pregeneratePreferences);
            random_device rd{};
            default_random_engine generator{rd()};
            int pindex = 0;
            for (int t = 0; t < this->partnerSideNTiers; t++) {
                exponential_distribution<double> distribution(partnerSideScores[t]);
                for (int i = 0; i < this->partnerSideTierSizes[t]; i++) {
                    map<int, double>::iterator it = this->invHappinessForPartners.find(pindex);
                    if (it == this->invHappinessForPartners.end()) {
                        PreferenceEntry pe { pindex, distribution(generator) };
                        this->preferences.push_back(pe);
                    } else {
                        PreferenceEntry pe { pindex, it->second };
                        this->preferences.push_back(pe);
                    }
                    pindex++;
                }
            }
            sort(this->preferences.begin(), this->preferences.end(), [](PreferenceEntry p1, PreferenceEntry p2) { return p1.invHappiness < p2.invHappiness; });
            break;
    }
    this->preferencesCompleted = true;
}

bool Agent::isOptimal() { return this->optimal; }
void Agent::markOptimal() { this->optimal = true; }

void Agent::stash()
{
    this->stashedPartners.push_back(this->curPartner);
    this->stashedProposalsMade = this->proposalsMade;
}

void Agent::stashPop()
{
    this->curPartner = this->stashedPartners.back();
    this->invHappiness = this->invHappinessForPartners[this->curPartner->index];
    this->stashedPartners.pop_back();
    this->poolSize += this->proposalsMade.size() - this->stashedProposalsMade.size(); // TODO: ignore poolSize with pregenerated preferences
    this->proposalsMade = this->stashedProposalsMade;
}

// should be called after a normal run
void Agent::reverseRole(const bool preservePartner)
{
    this->completePreferences();

    this->roleReversed = true;
    if (!preservePartner) {
        this->stash(); // save previous partner
        this->curPartner = NULL;
    } else if (this->curPartner) {
        this->invHappiness = this->invHappinessForPartners[this->curPartner->index];
    }
    this->poolSizesByTier = this->partnerSideTierSizes;
}

void Agent::reset()
{
    this->roleReversed = false;
    this->stash(); // save previous partner
    this->curPartner = NULL;
    this->proposalsMade.clear();
    this->poolSizesByTier = this->partnerSideTierSizes;
    this->sumScoresForPool = inner_product(this->partnerSideTierSizes.begin(), this->partnerSideTierSizes.end(),
                this->partnerSideScores.begin(), 0.0);
    this->poolSize = this->partnerSideNAgents;
    this->invHappiness = numeric_limits<double>::max();
    this->simulatedRankOfPartner = 0;
}

/** Manipulation and key operations **/

Agent* Agent::propose(vector<Agent*>& fullPool, mt19937& rng)
{
    if (this->poolSize == 0) return NULL;

    int indexToProposeTo = -1;
    if (this->pregeneratePreferences || this->roleReversed) {
        // long side proposing or reverse run: just follow the pre-generated preferences
        indexToProposeTo = this->preferences[this->proposalsMade.size()].index;
    } else {
        // first ignore the agents already proposed to, randomly pick in the rest
        uniform_real_distribution<double> distribution(0.0, this->sumScoresForPool);
        double randPositionInPool = distribution(rng);
        double baseScore = 0.0;
        int baseIndex = 0; // excluding already proposed
        for (int t = 0; t < this->partnerSideNTiers; t++) {
            double sumScoreInTier = this->poolSizesByTier[t] * this->partnerSideScores[t];
            if (randPositionInPool - baseScore < sumScoreInTier) {
                indexToProposeTo = baseIndex + int((randPositionInPool - baseScore) / this->partnerSideScores[t]);
                sort(this->proposalsMade.begin(), this->proposalsMade.end(), [](Agent* p1, Agent* p2) { return p1->index < p2->index; });
                // consider the agents already proposed to
                for (Agent* proposedAgent : this->proposalsMade) {
                    if (proposedAgent->index <= indexToProposeTo) indexToProposeTo++;
                }
                break;
            }
            baseScore += sumScoreInTier;
            baseIndex += this->poolSizesByTier[t];
        }
    }
    assert(indexToProposeTo >= 0);
    Agent* agentToProposeTo = fullPool[indexToProposeTo];
    // remove from pool
    this->poolSize--;
    this->poolSizesByTier[agentToProposeTo->tier]--;
    this->sumScoresForPool -= agentToProposeTo->score;
    this->proposalsMade.push_back(agentToProposeTo);

    if (this->verbose) cout << "Proposal: (P)" << this->index << " - (R)" << agentToProposeTo->index << endl;
    return agentToProposeTo;
}

double Agent::invHappinessFor(Agent* potentialMatch, mt19937& rng)
{
    // TODO: when not saving preferences?
    if (this->invHappinessForPartners.count(potentialMatch->index) > 0) return this->invHappinessForPartners[potentialMatch->index];
    exponential_distribution<double> distribution(potentialMatch->score);
    double invHappinessNew = distribution(rng);
    if (this->savePreferences) this->invHappinessForPartners[potentialMatch->index] = invHappinessNew;
    return invHappinessNew;
}

bool Agent::prefer(Agent* potentialMatch, mt19937& rng, const bool useStash)
{
    assert(this->pregeneratePreferences && this->savePreferences); // TODO: this should not be necessary
    double invHappinessNew = invHappinessFor(potentialMatch, rng);
    if (this->verbose && useStash)
        cout << "[Test preference] stashed happiness: " << this->invHappinessForPartners[this->stashedPartners.back()->index] << " - Tested happiness: " << invHappinessNew << endl;
    return invHappinessNew < (useStash ? this->invHappinessForPartners[this->stashedPartners.back()->index] : this->invHappinessForPartners[this->curPartner->index]);
}

Agent* Agent::handleProposal(Agent* proposer, mt19937& rng)
{
    this->poolSizesByTier[proposer->tier]--;

    double invHappinessNew = invHappinessFor(proposer, rng);
    if (invHappinessNew < this->invHappiness) {
        Agent* toReject = this->rejectMatched();
        this->invHappiness = invHappinessNew;
        if (this->verbose) cout << "Acceptance: (R)" << this->index << " - (P)" << proposer->index << endl;
        this->matchWith(proposer);
        proposer->matchWith(this);
        return toReject;
    }
    this->reject(proposer);
    return proposer;
}

void Agent::reject(Agent* agent)
{
    if (this->verbose) cout << "Rejection: (R)" << this->index << " - (P)" << agent->index << endl;
    agent->matchWith(NULL); // for initiating rejection
}

Agent* Agent::rejectMatched()
{
    Agent* toReject = this->curPartner;
    if (toReject) this->reject(toReject);
    return toReject;
}

void Agent::matchWith(Agent* agent)
{
    this->curPartner = agent;
}

/** Result computation **/

Agent* Agent::matchedPartner()
{
    return this->curPartner;
}

int Agent::numProposalsReceived()
{
    return this->partnerSideNAgents - accumulate(this->poolSizesByTier.begin(), this->poolSizesByTier.end(), 0);
}

// not counting the unmatched case
bool Agent::hasUniqueMatch()
{
    assert(this->roleReversed);
    return this->curPartner && !this->stashedPartners.empty() && this->stashedPartners.front() == this->curPartner;
}

int Agent::rankOfPartnerForProposer()
{
    assert(this->role == PROPOSER);
    assert(this->curPartner); // undefined if unmatched
    if (this->pregeneratePreferences) {
        return distance(this->preferences.begin(), find_if(this->preferences.begin(), this->preferences.end(), [&](PreferenceEntry pe) { return pe.index == this->curPartner->index; })) + 1;
    }
    return this->proposalsMade.size();
}

int Agent::rankOfPartnerForReceiver(mt19937& rng)
{
    assert(this->role == RECEIVER);
    assert(this->curPartner); // undefined if unmatched
    if (this->pregeneratePreferences) {
        if (this->preferencesCompleted) {
            return distance(this->preferences.begin(), find_if(this->preferences.begin(), this->preferences.end(), [&](PreferenceEntry pe) { return pe.index == this->curPartner->index; })) + 1;
        }
        return count_if(this->invHappinessForPartners.begin(), this->invHappinessForPartners.end(), [&](pair<int, double> p) { return p.second <= this->invHappiness; });
    }

    if (this->simulatedRankOfPartner > 0)
        return this->simulatedRankOfPartner;

    int rank = 1;
    for (int t = 0; t < this->partnerSideNTiers; t++) {
        if (this->poolSizesByTier[t] == 0) continue;
        binomial_distribution<int> distribution(this->poolSizesByTier[t],
                1 - exp(-this->partnerSideScores[t] * this->invHappiness));
        rank += distribution(rng);
    }
    this->simulatedRankOfPartner = rank;
    return rank;
}

void Agent::printPreferences(ostream& os)
{
    assert(this->preferencesCompleted);
    os << "(" << (this->role == PROPOSER ? "P" : "R") << ")" << this->index;
    for (const PreferenceEntry& pe : this->preferences) {
        os << "," << pe.index << "-" << this->invHappinessForPartners[pe.index];
    }
    os << endl;
}

