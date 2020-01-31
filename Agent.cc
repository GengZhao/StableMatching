#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <cmath>
#include <cassert>
#include <random>
#include <limits>

#include "Agent.h"

using namespace std;

Agent::Agent(
        int index,
        double score,
        int tier,
        std::vector<int> partnerSideTierSizes,
        std::vector<double> partnerSideScores,
        Role role,
        bool isLongSideAndProposing,
        bool verbose) :
    verbose(verbose),
    partnerSideTierSizes(partnerSideTierSizes),
    partnerSideScores(partnerSideScores),
    partnerSideNTiers(partnerSideTierSizes.size()),
    role(role), isLongSideAndProposing(isLongSideAndProposing),
    simulatedRankOfPartner(0),
    index(index), score(score), tier(tier)
{
    this->curPartner = NULL;
    this->poolSizesByTier = partnerSideTierSizes;
    this->invHappiness = numeric_limits<double>::max();

    this->poolSize = 0;
    this->sumScoresForPool = 0.0;
    for (int t = 0; t < this->partnerSideNTiers; t++) {
        this->poolSize += partnerSideTierSizes[t];
        this->sumScoresForPool += partnerSideTierSizes[t] * partnerSideScores[t];
    }

    // if long side proposes, pre-generate all preferences by exp distribution for proposers
    if (isLongSideAndProposing) {
        default_random_engine generator;
        for (int t = 0; t < this->partnerSideNTiers; t++) {
            exponential_distribution<double> distribution(partnerSideScores[t]);
            for (int i = 0; i < this->poolSizesByTier[t]; i++) {
                PreferenceEntry pe { t, distribution(generator) };
                this->preferences.push_back(pe);
            }
        }
        sort(this->preferences.begin(), this->preferences.end(), [](PreferenceEntry p1, PreferenceEntry p2) { return p1.invHappiness < p2.invHappiness; });
    }
}

Agent* Agent::matchedPartner()
{
    return this->curPartner;
}

Agent* Agent::propose(vector<Agent*>& fullPool, mt19937& rng)
{
    if (this->poolSize == 0) return NULL;

    int indexToProposeTo = -1;
    if (this->isLongSideAndProposing) {
        // long side proposing: just follow the pre-generated preferences
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

Agent* Agent::handleProposal(Agent* proposer, mt19937& rng)
{
    this->poolSizesByTier[proposer->tier]--;

    exponential_distribution<double> distribution(proposer->score);
    double invHappinessNew = distribution(rng);

    if (invHappinessNew < this->invHappiness) {
        this->invHappiness = invHappinessNew;
        Agent* toReject = this->curPartner;
        if (this->curPartner) this->reject(toReject);
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

void Agent::matchWith(Agent* agent)
{
    this->curPartner = agent;
}

int Agent::rankOfPartnerForProposer()
{
    assert(this->role == PROPOSER);
    return this->proposalsMade.size();
}

int Agent::rankOfPartnerForReceiver(mt19937& rng)
{
    assert(this->role == RECEIVER);
    assert(this->curPartner); // undefined if unmatched
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
