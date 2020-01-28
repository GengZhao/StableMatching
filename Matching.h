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

        bool verbose;
        std::vector<int> tierSizesProp;
        std::vector<int> tierSizesRec;
        int nTiersProp;
        int nAgentsProp;
        std::vector<double> scoresProp;
        std::vector<double> scoresRec;
        int nTiersRec;
        int nAgentsRec;

        std::vector<Agent*> agentsProp;
        std::vector<Agent*> agentsRec;

        std::queue<Agent*> agentsToPropose;

        std::vector<std::vector<int> > proposalCountMatrix;
        std::vector<std::vector<int> > matchCountMatrix;
        std::vector<int> numProposalsMadeByPropTier;
        std::vector<int> numMatchesByPropTier;
        int totalNumProposals;

    public:
        Matching(
            int nTiersProp,
            int nTiersRec,
            std::vector<int> tierSizesProp,
            std::vector<int> tierSizesRec,
            std::vector<double> scoresProp,
            std::vector<double> scoresRec,
            bool verbose=false
        );
        ~Matching();
        void run();
        std::vector<double> avgRankForProposerByTier();
        void result();
};

#endif
