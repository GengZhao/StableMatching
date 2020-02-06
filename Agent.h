#ifndef AGENT_H
#define AGENT_H

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

        const std::vector<int> partnerSideTierSizes;
        const std::vector<double> partnerSideScores;
        const int partnerSideNTiers;
        const Role role;
        bool roleReversed;
        const bool pregeneratePreferences; // for long side proposing
        const bool savePreferences; // for reverse proposing

        double sumScoresForPool; // only maintained and used for proposers
        int poolSize; // only maintained and used for proposers
        Agent* curPartner;
        Agent* prevRunPartner;
        std::vector<Agent*> proposalsMade; // only and used for proposers
        std::vector<int> poolSizesByTier; // for both sides
        double invHappiness; // only relevant for receivers

        int simulatedRankOfPartner; // only for receivers (simulation cache)
        std::vector<PreferenceEntry> preferences; // only for proposers for performance reason when long side proposes
        bool preferencesCompleted; // avoid re-completion of preferences data structures
        std::map<int, double> invHappinessForPartners; // only for receivers to handle comparison

        void completePreferences();
        void reject(Agent*);
        void matchWith(Agent*);

    public:
        const int index;
        const double score;
        const int tier;

        Agent(
            const int index,
            const double score,
            const int tier,
            const std::vector<int> partnerSideTierSizes,
            const std::vector<double> partnerSideScores,
            const Role role,
            const bool pregeneratePreferences=false,
            const bool savePreferences=false,
            const bool verbose=false
        );
        ~Agent() {};
        Agent* propose(std::vector<Agent*>& fullPool, std::mt19937& rng);
        Agent* handleProposal(Agent*, std::mt19937& rng); // returns the rejected agent
        void reverseRole(const bool preservePartner=false);
        void reset();

        Agent* matchedPartner();
        bool hasUniqueMatch();

        int rankOfPartnerForProposer(); // only for proposers
        int rankOfPartnerForReceiver(std::mt19937& rng); // only for receivers, supplying the proposers vector just for convenience
};

#endif
