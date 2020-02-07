#!/usr/bin/env python
import numpy as np
import matplotlib.pyplot as plt
import os, sys

class MatchingConfig:

    def __init__(self, nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec):
        self.nTiersProp = nTiersProp
        self.nTiersRec = nTiersRec
        self.tierSizesProp = tierSizesProp
        self.tierSizesRec = tierSizesRec
        self.scoresProp = scoresProp
        self.scoresRec = scoresRec
        self.nAgentsProp = sum(tierSizesProp)
        self.nAgentsRec = sum(tierSizesRec)

def readMatchingResultFromFile(inFile):
    matchingConfigs = []
    proposerResults = []
    receiverResults = []
    for l in inFile:
        nTiersProp = int(l)
        tierSizesProp = [int(next(inFile)) for tr in range(nTiersProp)]
        scoresProp = [float(next(inFile)) for tr in range(nTiersProp)]
        nTiersRec = int(next(inFile))
        tierSizesRec = [int(next(inFile)) for tr in range(nTiersRec)]
        scoresRec = [float(next(inFile)) for tr in range(nTiersRec)]
        matchingConfigs.append(MatchingConfig(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec))

        nIters = int(next(inFile))
        proposerResults.append([[float(next(inFile)) for i in range(nIters)] for tp in range(nTiersProp)])
        receiverResults.append([[float(next(inFile)) for i in range(nIters)] for tp in range(nTiersRec)])

    return (matchingConfigs, proposerResults, receiverResults)

def readUniqueCountResultFromFile(inFile):
    matchingConfigs = []
    proposerUniquePartnerCountByTier = []
    for l in inFile:
        nTiersProp = int(l)
        tierSizesProp = [int(next(inFile)) for tr in range(nTiersProp)]
        scoresProp = [float(next(inFile)) for tr in range(nTiersProp)]
        nTiersRec = int(next(inFile))
        tierSizesRec = [int(next(inFile)) for tr in range(nTiersRec)]
        scoresRec = [float(next(inFile)) for tr in range(nTiersRec)]
        matchingConfigs.append(MatchingConfig(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec))

        nIters = int(next(inFile))
        proposerUniquePartnerCountByTier.append([[int(next(inFile)) for i in range(nIters)] for tp in range(nTiersProp)])

    return (matchingConfigs, proposerUniquePartnerCountByTier)

def readMatchingStatisticsFromFile(inFile):
    matchingConfigs = []
    matchingStatistics = []
    for l in inFile:
        nTiersProp = int(l)
        tierSizesProp = [int(next(inFile)) for tr in range(nTiersProp)]
        scoresProp = [float(next(inFile)) for tr in range(nTiersProp)]
        nTiersRec = int(next(inFile))
        tierSizesRec = [int(next(inFile)) for tr in range(nTiersRec)]
        scoresRec = [float(next(inFile)) for tr in range(nTiersRec)]
        matchingConfigs.append(MatchingConfig(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec))

        nIters = int(next(inFile))
        matchingStatistics.append([int(next(inFile)) for i in range(nIters)])

    return (matchingConfigs, matchingStatistics)

if __name__ == '__main__':
    dataFile = open(sys.argv[-1], 'r')
