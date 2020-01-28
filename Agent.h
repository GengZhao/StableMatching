#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <cmath>
#include <cassert>
#include <random>
#include <limits>

struct PreferenceEntry
{
    int index;
    float invHappiness;
};

class Agent
{
    private:
        bool verbose;

        float invHappiness; // only relevant for receivers
        float sumScoresForPool; // only relevant for proposers
        int poolSize; // only relevant for proposers
        Agent* curPartner;
        std::vector<Agent*> proposalsMade; // only for proposers
        std::vector<int> poolSizesByTier; // for both sides
        std::vector<float> partnerSideScores;
        int partnerSideNTiers;
        bool isLongSideAndProposing;

        std::vector<PreferenceEntry> preferences; // only for proposers for performance reason when long side proposes

    public:
        int index;
        float score;
        int tier;

        Agent(int index,
                float score,
                int tier,
                std::vector<int> tierSizesPool,
                std::vector<float> scoresPool,
                bool isLongSideAndProposing,
                bool verbose=false);
        ~Agent() {};
        Agent* matchedPartner();
        int numProposalsMade(); // only for proposers
        Agent* propose(std::vector<Agent*>& fullPool, std::mt19937& rng);
        Agent* handleProposal(Agent*, std::mt19937& rng); // returns the rejected agent
        void reject(Agent*);
        void matchWith(Agent*);
};

#endif
