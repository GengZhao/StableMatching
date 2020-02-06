#ifndef MATCHING_H
#define MATCHING_H

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <cassert>
#include <random>

#include "Agent.h"

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

        std::vector<std::vector<int> > proposalCountMatrix;
        std::vector<std::vector<int> > matchCountMatrix;
        std::vector<int> numProposalsMadeByPropTier;
        std::vector<int> numMatchesByPropTier;
        std::vector<int> numMatchesByRecTier;

        const bool pregeneratePreferences; // for long running proposing chain
        const bool savePreferences; // for generating full preferences

        bool recordingProposalCounts;

        void printMatchSetupInfo();

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
        void reverseRun();
        void runExperimental();
        std::vector<double> avgRankForProposerByTier(); // only counting matched proposers
        std::vector<double> avgRankForReceiverByTier(); // only counting matched receivers (simulated)
        std::vector<std::vector<int> > reverseRunCountUniquePartners();
        void result();
        void resetState(); // will preserve agents' generated preferences
};

#endif
