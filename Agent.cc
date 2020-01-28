#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>
#include <random>
#include <limits>

#include "Agent.h"

using namespace std;

Agent::Agent(int index,
        float score,
        int tier,
        std::vector<int> partnerSideTierSizes,
        std::vector<float> partnerSideScores,
        bool isLongSideAndProposing,
        bool verbose)
{
    this->verbose = verbose;

    this->index = index;
    this->score = score;
    this->tier = tier;
    this->curPartner = NULL;
    this->poolSizesByTier = partnerSideTierSizes;
    this->partnerSideNTiers = partnerSideTierSizes.size();
    this->partnerSideScores = partnerSideScores;
    assert(this->partnerSideNTiers == (int) partnerSideScores.size());
    this->invHappiness = numeric_limits<float>::max();

    this->poolSize = 0;
    this->sumScoresForPool = 0.0;
    for (int t = 0; t < this->partnerSideNTiers; t++) {
        this->poolSize += partnerSideTierSizes[t];
        this->sumScoresForPool += partnerSideTierSizes[t] * partnerSideScores[t];
    }

    // if long side proposes, pre-generate all preferences by exp distribution for proposers
    this->isLongSideAndProposing = isLongSideAndProposing;
    if (isLongSideAndProposing) {
        default_random_engine generator;
        for (int t = 0; t < this->partnerSideNTiers; t++) {
            exponential_distribution<float> distribution(this->partnerSideScores[t]);
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

int Agent::numProposalsMade()
{
    return this->proposalsMade.size();
}

Agent* Agent::propose(vector<Agent*>& fullPool, mt19937& rng)
{
    if (this->poolSize == 0) return NULL;

    int indexToProposeTo = -1;
    if (isLongSideAndProposing) {
        // long side proposing: just follow the pre-generated preferences
        indexToProposeTo = this->preferences[this->proposalsMade.size()].index;
    } else {
        // first ignore the agents already proposed to, randomly pick in the rest
        uniform_real_distribution<float> distribution(0.0, this->sumScoresForPool);
        float randPositionInPool = distribution(rng);
        float baseScore = 0.0;
        int baseIndex = 0; // excluding already proposed
        for (int t = 0; t < this->partnerSideNTiers; t++) {
            float sumScoreInTier = this->poolSizesByTier[t] * this->partnerSideScores[t];
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

    exponential_distribution<float> distribution(proposer->score);
    float invHappinessNew = distribution(rng);

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
