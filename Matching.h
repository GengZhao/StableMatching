#ifndef MATCHING_H
#define MATCHING_H

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <cassert>
#include <random>

#include "Agent.h"

struct RejectionChainEntry
{
    Agent* fromRejecter;
    Agent* toRejectee;
};

class RejectionChain
{
    private:
        std::vector<RejectionChainEntry> entries;
        std::map<Agent*, int> rejecterPositions;

    public:
        void add(Agent* fromRejecter, Agent* toRejectee) {
            rejecterPositions[fromRejecter] = entries.size();
            RejectionChainEntry rce { fromRejecter, toRejectee };
            entries.push_back(rce);
        }

        bool empty() { return entries.empty(); }
        bool contains(Agent* receiver) { return rejecterPositions.count(receiver) > 0; }
        int positionOf(Agent* receiver) { return rejecterPositions.at(receiver); }
        RejectionChainEntry at(int position) { return entries.at(position); }
        Agent* nextProposer() { return entries.back().toRejectee; }
};

class Matching
{
    private:
        std::random_device rd;
        std::mt19937 rng;

        const bool verbose;
        const int nTiersProp;
        const int nTiersRec;
        const std::vector<int> tierSizesProp;
        const std::vector<int> tierSizesRec;
        const std::vector<double> scoresProp;
        const std::vector<double> scoresRec;
        const int nAgentsProp;
        const int nAgentsRec;

        std::vector<Agent*> agentsProp;
        std::vector<Agent*> agentsRec;

        std::queue<Agent*> agentsToPropose;
        std::queue<Agent*> suboptimalReceivers; // initialized after the first stable matching is found
        // RejectionChain rejectionChain;

        std::vector<std::vector<int> > proposalCountMatrix;
        std::vector<std::vector<int> > matchCountMatrix;
        std::vector<int> numProposalsMadeByPropTier;
        std::vector<int> numMatchesByPropTier;
        std::vector<int> numMatchesByRecTier;

        const bool pregeneratePreferences; // for long running proposing chain
        const bool savePreferences; // for generating full preferences. This ensures consistency

        bool recordingProposalCounts;

        void printMatchSetupInfo();
        void stashAll();
        void stashPopAll();

    public:
        int totalNumProposals; // for convenience

        Matching(
            const int nTiersProp,
            const int nTiersRec,
            const std::vector<int>& tierSizesProp,
            const std::vector<int>& tierSizesRec,
            const std::vector<double>& scoresProp,
            const std::vector<double>& scoresRec,
            const bool pregeneratePreferences=false,
            const bool savePreferences=false,
            const bool verbose=false
        );
        ~Matching();
        void run();
        bool runFromCurrent();
        void reverseRun();
        void resetState(); // will preserve agents' generated preferences
        void runExperimental();
        std::vector<double> avgRankForProposerByTier(); // only counting matched proposers
        std::vector<double> avgRankForReceiverByTier(); // only counting matched receivers (simulated)
        std::vector<std::vector<int> > reverseRunCountUniquePartners();
        std::vector<std::vector<int> > getMatchCountMatrix();
        void result();
        void printRanksRec(std::ostream& os=std::cout);
        void printNProposalsRec(std::ostream& os=std::cout);
};

#endif

