#ifndef AGENT_H
#define AGENT_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <cmath>
#include <cassert>
#include <random>
#include <limits>

struct PreferenceEntry
{
    int index;
    double invHappiness;
};

enum Role { PROPOSER, RECEIVER };

class Agent
{
    private:
        const bool verbose;

        double invHappiness; // only relevant for receivers
        double sumScoresForPool; // only maintained for proposers (also used in simulation for receivers)
        int poolSize; // only maintained for proposers (also used in simulation for receivers)
        Agent* curPartner;
        std::vector<Agent*> proposalsMade; // only for proposers
        std::vector<int> poolSizesByTier; // for both sides
        const std::vector<int> partnerSideTierSizes;
        const std::vector<double> partnerSideScores;
        const int partnerSideNTiers;
        const Role role;
        const bool isLongSideAndProposing;

        int simulatedRankOfPartner; // only for receivers (simulation cache)
        std::vector<PreferenceEntry> preferences; // only for proposers for performance reason when long side proposes

    public:
        const int index;
        const double score;
        const int tier;

        Agent(
            int index,
            double score,
            int tier,
            std::vector<int> tierSizesPool,
            std::vector<double> scoresPool,
            Role role,
            bool isLongSideAndProposing,
            bool verbose=false
        );
        ~Agent() {};
        Agent* matchedPartner();
        Agent* propose(std::vector<Agent*>& fullPool, std::mt19937& rng);
        Agent* handleProposal(Agent*, std::mt19937& rng); // returns the rejected agent
        void reject(Agent*);
        void matchWith(Agent*);

        int rankOfPartnerForProposer(); // only for proposers
        int rankOfPartnerForReceiver(std::mt19937& rng); // only for receivers, supplying the proposers vector just for convenience
};

#endif
