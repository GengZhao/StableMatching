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
        const int partnerSideNAgents;
        const int partnerSideNTiers;
        const Role role;
        bool roleReversed;
        const bool pregeneratePreferences; // for long side proposing
        const bool savePreferences; // for reverse proposing

        double sumScoresForPool; // only maintained and used for proposers
        int poolSize; // only maintained and used for proposers
        Agent* curPartner;
        std::vector<Agent*> stashedPartners;
        std::vector<Agent*> proposalsMade; // only and used for proposers
        std::vector<Agent*> stashedProposalsMade;
        std::vector<int> poolSizesByTier; // for both sides
        double invHappiness; // only relevant for receivers
        bool optimal; // currently only used for receivers, but can potentially be implemented also for proposers

        int simulatedRankOfPartner; // only for receivers (simulation cache)
        std::vector<PreferenceEntry> preferences; // only for proposers for performance reason when long side proposes
        bool preferencesCompleted; // avoid re-completion of preferences data structures
        std::map<int, double> invHappinessForPartners; // only for receivers to handle comparison

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
        double invHappinessFor(Agent*, std::mt19937& rng);
        bool prefer(Agent*, std::mt19937& rng, const bool useStash=false);
        Agent* handleProposal(Agent*, std::mt19937& rng); // returns the rejected agent
        void reject(Agent*);
        Agent* rejectMatched();
        void matchWith(Agent*);

        bool isOptimal() const;
        void markOptimal();
        void stash();
        void stashPop();
        void reverseRole(const bool preservePartner=false);
        void completePreferences();
        void reset();

        Agent* matchedPartner() const;
        bool hasUniqueMatch() const;

        int rankOfPartnerForAgent(std::mt19937& rng);
        int rankOfPartnerForProposer(); // only for proposers
        int rankOfPartnerForReceiver(std::mt19937& rng); // only for receivers
        double scoreFor(double); // temp
        double invHappinessOfAgent() const;
        int numProposalsReceived() const; // only for receivers
        void printPreferences(std::ostream& os=std::cout) const;
};

#endif
