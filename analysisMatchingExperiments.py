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

def readMatchingStatisticsFromFile(inFile, nSeries=1):
    matchingConfigs = []
    matchingStatistics = [[] for _ in range(nSeries)]
    for l in inFile:
        nTiersProp = int(l)
        tierSizesProp = [int(next(inFile)) for tr in range(nTiersProp)]
        scoresProp = [float(next(inFile)) for tr in range(nTiersProp)]
        nTiersRec = int(next(inFile))
        tierSizesRec = [int(next(inFile)) for tr in range(nTiersRec)]
        scoresRec = [float(next(inFile)) for tr in range(nTiersRec)]
        matchingConfigs.append(MatchingConfig(nTiersProp, nTiersRec, tierSizesProp, tierSizesRec, scoresProp, scoresRec))

        nIters = int(next(inFile))
        for s in range(nSeries):
            matchingStatistics[s].append([int(next(inFile)) for i in range(nIters)])

    return [matchingConfigs] + matchingStatistics

if __name__ == '__main__':
    '''
    Main program to analyze data and produce plots. Hacky at some places as I hardcoded some specific data shapes for convenience.
    '''
    def reformat_large_tick_values(tick_val, pos):
        '''
        Turns large tick values (in the billions, millions and thousands) such as 4500 into 4.5K and also appropriately turns
        4000 into 4K (no zero after the decimal).

        Adopted from https://dfrieds.com/data-visualizations/how-format-large-tick-values
        '''
        if tick_val >= 1000000000:
            val = round(tick_val/1000000000, 1)
            new_tick_format = '{:}B'.format(val)
        elif tick_val >= 1000000:
            val = round(tick_val/1000000, 1)
            new_tick_format = '{:}M'.format(val)
        elif tick_val >= 1000:
            val = round(tick_val/1000, 1)
            new_tick_format = '{:}K'.format(val)
        elif tick_val < 1000:
            new_tick_format = round(tick_val, 1)
        else:
            new_tick_format = tick_val

        # make new_tick_format into a string value
        new_tick_format = str(new_tick_format)

        # code below will keep 4.5M as is but change values such as 4.0M to 4M since that zero after the decimal isn't needed
        index_of_decimal = new_tick_format.find(".")

        if index_of_decimal != -1:
            value_after_decimal = new_tick_format[index_of_decimal+1]
            if value_after_decimal == "0":
                # remove the 0 after the decimal point since it's not needed
                new_tick_format = new_tick_format[0:index_of_decimal] + new_tick_format[index_of_decimal+2:]

        return new_tick_format

    '''
    Adopted based on http://aeturrell.com/2018/01/31/publication-quality-plots-in-python/
    '''
    plt.rc('font', family='serif')
    plt.rc('xtick', labelsize=13)
    plt.rc('ytick', labelsize=13)
    plt.rc('legend', fontsize=13)
    plt.rc('axes', labelsize=14)
    plt.rc('axes', titlesize=13)
    plt.rc('lines', linewidth=1.0)
    plt.rc('lines', markersize=3.5)
    plt.rc('figure', autolayout=True)
    plt.rc('figure', figsize=[6.4, 4.8])
    colourWheel =['#329932',
                '#ff6961',
                '#6a3d9a',
                '#fb9a99',
                '#e31a1c',
                '#fdbf6f',
                'b',
                '#ff7f00',
                '#cab2d6',
                '#6a3d9a',
                '#ffff99',
                '#b15928',
                '#67001f',
                '#b2182b',
                '#053061']

    '''
    ## Growing market exponential
    m1,p1,r1 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_00:06:27.out', 'r'))
    m2,p2,r2 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_00:26:18.out', 'r'))
    m3,p3,r3 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_00:27:55.out', 'r'))
    m4,p4,r4 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_00:29:31.out', 'r'))
    m5,p5,r5 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_01:19:03.out', 'r'))
    m6,p6,r6 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_19:27:15.out', 'r'))
    m7,p7,r7 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_19:28:09.out', 'r'))
    m8,p8,r8 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market-2020-01-30_19:33:01.out', 'r'))
    p1 = np.array(p1)
    p2 = np.array(p2)
    p3 = np.array(p3)
    p4 = np.array(p4)
    p5 = np.array(p5)
    p6 = np.array(p6)
    p7 = np.array(p7)
    p8 = np.array(p8)
    r1 = np.array(r1)
    r2 = np.array(r2)
    r3 = np.array(r3)
    r4 = np.array(r4)
    r5 = np.array(r5)
    r6 = np.array(r6)
    r7 = np.array(r7)
    r8 = np.array(r8)
    plog = np.concatenate((np.concatenate((p1, p2, p3, p4), 2), np.concatenate((p5, p6, p7, p8), 2)), 0)
    rlog = np.concatenate((np.concatenate((r1, r2, r3, r4), 2), np.concatenate((r5, r6, r7, r8), 2)), 0)

    alphaV = np.array([1.0, 2.0, 3.0])
    betaV = np.array([3.0, 1.0])
    epsilonV = np.array([1.0/16, 5.0/16, 10.0/16])
    deltaV = np.array([0.25, 0.75])

    plt.plot(np.logspace(4,19,16,base=2),np.log(np.logspace(4,19,16,base=2)) * (alphaV.dot(epsilonV)) / (deltaV.dot(1.0/betaV)) / 3, ls='--', color=colourWheel[0])
    plt.plot(np.logspace(4,19,16,base=2),np.log(np.logspace(4,19,16,base=2)) * (alphaV.dot(epsilonV)) / (deltaV.dot(1.0/betaV)), ls='--', color=colourWheel[1])
    plt.plot(np.logspace(4,19,16,base=2),np.log(np.logspace(4,19,16,base=2)/16) * (alphaV.dot(epsilonV)) / (deltaV.dot(1.0/betaV)) / 3, ls=':', color=colourWheel[0])
    plt.plot(np.logspace(4,19,16,base=2),np.log(np.logspace(4,19,16,base=2)/16) * (alphaV.dot(epsilonV)) / (deltaV.dot(1.0/betaV)), ls=':', color=colourWheel[1])

    series = []
    for j in range(plog.shape[1]):
        series.append(plt.plot(np.logspace(4,19,16,base=2), [np.mean(plog[i][j]) for i in range(plog.shape[0])], color=colourWheel[j], marker='.')[0])

    plt.xlabel("Number of agents on each side")
    plt.ylabel("Men's average rank of partners")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='lower right')
    plt.xscale('log')
    plt.yscale('log')
    plt.show()

    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV) / 3, ls='--', color=colourWheel[0])
    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV) / 2, ls='--', color=colourWheel[1])
    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV), ls='--', color=colourWheel[2])
    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)/16) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV) / 3, ls=':', color=colourWheel[0])
    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)/16) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV) / 2, ls=':', color=colourWheel[1])
    plt.plot(np.logspace(4,19,16,base=2),np.logspace(4,19,16,base=2) / np.log(np.logspace(4,19,16,base=2)/16) * deltaV.dot(betaV) * deltaV.dot(1.0/betaV), ls=':', color=colourWheel[2])

    series = []
    for j in range(rlog.shape[1]):
        series.append(plt.plot(np.logspace(4,19,16,base=2), [np.mean(rlog[i][-1-j]) for i in range(rlog.shape[0])], color=colourWheel[j], marker='.')[0])

    plt.xlabel("Number of agents on each side")
    plt.ylabel("Women's average rank of partners")
    plt.legend(series, ['Tier 1', 'Tier 2', 'Tier 3'], loc='lower right')
    plt.xscale('log')
    plt.yscale('log')
    plt.show()

    series = []
    series.append(plt.plot(np.logspace(4,19,16,base=2),[np.mean(plog[i][1]) / np.mean(plog[i][0]) for i in range(plog.shape[0])], color=colourWheel[0], marker='.')[0])
    plt.hlines(3.0, plt.xlim()[0], plt.xlim()[1], color=colourWheel[0], linestyles=':')
    plt.ylim(ymin=0.95)
    plt.xlabel("Number of agents on each side")
    plt.ylabel("Ratio of average rank between tiers\non men side")
    plt.legend(series, ['Tier 2 : Tier 1'], loc='lower right')
    plt.xscale('log')
    plt.show()

    series = []
    for j in [0,1]: series.append(plt.plot(np.logspace(4,19,16,base=2),[np.mean(rlog[i][1-j]) / np.mean(rlog[i][2]) for i in range(rlog.shape[0])], color=colourWheel[j], marker='.')[0])
    plt.hlines(1.5, plt.xlim()[0], plt.xlim()[1], color=colourWheel[0], linestyles=':')
    plt.hlines(3.0, plt.xlim()[0], plt.xlim()[1], color=colourWheel[1], linestyles=':')
    plt.ylim(ymin=0.95)
    plt.xlabel("Number of agents on each side")
    plt.ylabel("Ratio of average rank between tiers\non women side")
    plt.legend(series, ['Tier 2 : Tier 1', 'Tier 3 : Tier 1'], loc='lower right')
    plt.xscale('log')
    plt.show()

    ## Varying tier configurations
    mvs1,pvs1,rvs1 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-07_23:55:01.out', 'r'))
    mvs2,pvs2,rvs2 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-07_23:20:17.out', 'r'))
    # mvs1,pvs1,rvs1 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-01_22:32:51.out', 'r'))
    # mvs2,pvs2,rvs2 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-02_16:54:42.out', 'r'))
    # mvs3,pvs3,rvs3 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-02_17:13:34.out', 'r'))
    # mvs4,pvs4,rvs4 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_scores-2020-02-03_20:37:58.out', 'r'))

    mvt1,pvt1,rvt1 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_tier_sizes-2020-02-01_22:32:51.out', 'r'))
    mvt2,pvt2,rvt2 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_tier_sizes-2020-02-02_16:47:20.out', 'r'))
    mvt3,pvt3,rvt3 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_tier_sizes-2020-02-02_17:06:43.out', 'r'))
    mvt4,pvt4,rvt4 = readMatchingResultFromFile(open('Outputs/fixed_market_varying_tier_sizes-2020-02-03_20:16:56.out', 'r'))

    mpvs = np.array(pvs1)
    mrvs = np.array(rvs1)
    wrvs = np.array(rvs2)
    wpvs = np.array(pvs2)

    mpvt = np.concatenate([np.array(pvt1),np.array(pvt2),np.array(pvt3)], 2)
    mrvt = np.concatenate([np.array(rvt1),np.array(rvt2),np.array(rvt3)], 2)
    wrvt = np.array(rvt4)
    wpvt = np.array(pvt4)

    # plt.rc('text', usetex=True)

    # Varying score
    series = []
    for j in range(mpvs.shape[1]): series.append(plt.plot(np.linspace(1,10,19), [np.mean(mpvs[i][j]) for i in range(mpvs.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel("alpha_1")
    plt.ylabel("Men's average rank of partners\nunder MOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(mrvs.shape[1]): series.append(plt.plot(np.linspace(1,10,19), [np.mean(mrvs[i][-1-j]) for i in range(mrvs.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel("alpha_1")
    plt.ylabel("Women's average rank of partners\nunder MOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(wrvs.shape[1]): series.append(plt.plot(np.linspace(1,10,19), [np.mean(wrvs[i][j]) for i in range(wrvs.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel("alpha_1")
    plt.ylabel("Men's average rank of partners\nunder WOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(wpvs.shape[1]): series.append(plt.plot(np.linspace(1,10,19), [np.mean(wpvs[i][-1-j]) for i in range(wpvs.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel("alpha_1")
    plt.ylabel("Women's average rank of partners\nunder WOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    # Varying tier sizes
    series = []
    for j in range(mpvt.shape[1]): series.append(plt.plot(np.linspace(0.05,0.95,19), [np.mean(mpvt[i][j]) for i in range(mpvt.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel(r'delta_1')
    plt.ylabel("Men's average rank of partners\nunder MOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(mrvt.shape[1]): series.append(plt.plot(np.linspace(0.05,0.95,19), [np.mean(mrvt[i][j]) for i in range(mrvt.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel(r'delta_1')
    plt.ylabel("Women's average rank of partners\nunder MOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(wrvt.shape[1]): series.append(plt.plot(np.linspace(0.05,0.95,19), [np.mean(wrvt[i][j]) for i in range(wrvt.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel(r'delta_1')
    plt.ylabel("Men's average rank of partners\nunder WOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    series = []
    for j in range(wpvt.shape[1]): series.append(plt.plot(np.linspace(0.05,0.95,19), [np.mean(wpvt[i][j]) for i in range(wpvt.shape[0])], marker='.', color=colourWheel[j])[0])
    plt.xlabel(r'delta_1')
    plt.ylabel("Women's average rank of partners\nunder WOSM")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='right')
    plt.show()

    ## Unbalanced market
    mi,pi,ri = readMatchingResultFromFile(open('Outputs/imbalanced_market-2020-02-05_01:56:37.out', 'r'))
    pi = np.array(pi)
    ri = np.array(ri)
    series = []
    series.append(plt.plot(np.linspace(-10,10,21), [np.mean(pi[i][0]) for i in range(pi.shape[0])], marker='.', color=colourWheel[0])[0])
    series.append(plt.plot(np.linspace(-10,10,21), [np.mean(pi[i][1]) for i in range(pi.shape[0])], marker='.', color=colourWheel[1])[0])
    series.append(plt.plot(np.linspace(-10,10,21),[np.mean(ri[i][0]) for i in range(ri.shape[0])], marker='.', color=colourWheel[2])[0])
    plt.vlines(0.0, plt.ylim()[0], plt.ylim()[1], linewidth=1, color='lightgrey', linestyles='--')
    # plt.rc('text', usetex=True)
    plt.xlabel('|M|-|W|')
    plt.ylabel("Average rank of partners under MOSM")
    plt.legend(series, ['Tier 1 men', 'Tier 2 men', 'Women'], loc='right')
    plt.show()

    mu,pu = readUniqueCountResultFromFile(open('Outputs/unique_partner_count-2020-02-02_17:26:15.out','r'))
    pu = np.array(pu)
    series = []
    series.append(plt.plot(np.linspace(-10,10,21), [np.mean(pu[i][0])/3.0 for i in range(pu.shape[0])], marker='.', color=colourWheel[0])[0])
    series.append(plt.plot(np.linspace(-10,10,21), [np.mean(pu[i][1])/7.0 for i in range(pu.shape[0])], marker='.', color=colourWheel[1])[0])
    plt.vlines(0.0, plt.ylim()[0], plt.ylim()[1], linewidth=1, color='lightgrey', linestyles='--')
    # plt.rc('text', usetex=True)
    plt.xlabel('|M|-|W|')
    plt.ylabel("Percentage of matched men\nwith unique stable partners")
    plt.legend(series, ['Tier 1 men', 'Tier 2 men'], loc='right')
    plt.show()

    ## Growing market linear
    ml1,pl1,rl1 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market_lin-2020-02-01_17:08:07.out', 'r'))
    ml2,pl2,rl2 = readMatchingResultFromFile(open('Outputs/fixed_tiers_growing_market_lin-2020-02-01_17:38:50.out', 'r'))
    pl1 = np.array(pl1)
    rl1 = np.array(rl1)
    pl2 = np.array(pl2)
    rl2 = np.array(rl2)
    pl = np.concatenate([pl1,pl2], 2)
    rl = np.concatenate([rl1,rl2], 2)

    plt.plot(np.linspace(80,10000,125),np.log(np.linspace(80,10000,125)) * 2.5625 * 0.4, ls='--', color=colourWheel[0])
    plt.plot(np.linspace(80,10000,125),np.log(np.linspace(80,10000,125)) * 2.5625 * 1.2, ls='--', color=colourWheel[1])
    plt.plot(np.linspace(80,10000,125),np.log(np.linspace(80,10000,125)/16.0) * 2.5625 * 0.4, ls=':', color=colourWheel[0])
    plt.plot(np.linspace(80,10000,125),np.log(np.linspace(80,10000,125)/16.0) * 2.5625 * 1.2, ls=':', color=colourWheel[1])

    series = []
    for j in range(pl.shape[1]):
        series.append(plt.plot(np.linspace(80,10000,125), [np.mean(pl[i][j]) for i in range(pl.shape[0])], color=colourWheel[j], marker='.')[0])

    plt.xlabel("Number of agents on each side")
    plt.ylabel("Men's average rank of partners")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='upper left')
    plt.show()

    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)) * 1.25, ls='--', color=colourWheel[0])
    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)) * 1.25/2, ls='--', color=colourWheel[1])
    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)) * 1.25/3, ls='--', color=colourWheel[2])
    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)/16.0) * 1.25, ls=':', color=colourWheel[0])
    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)/16.0) * 1.25/2, ls=':', color=colourWheel[1])
    plt.plot(np.linspace(80,10000,125),np.linspace(80,10000,125) / np.log(np.linspace(80,10000,125)/16.0) * 1.25/3, ls=':', color=colourWheel[2])

    series = []
    for j in range(rl.shape[1]):
        series.append(plt.plot(np.linspace(80,10000,125), [np.mean(rl[i][j]) for i in range(rl.shape[0])], color=colourWheel[j], marker='.')[0])

    plt.xlabel("Number of agents on each side")
    plt.ylabel("Women's average rank of partners")
    plt.legend(series, ['Tier 1', 'Tier 2', 'Tier 3'], loc='upper left')
    plt.show()

    series = []
    series.append(plt.plot(np.linspace(80,10000,125),[np.mean(pl[i][1]) / np.mean(pl[i][0]) for i in range(pl.shape[0])], color=colourWheel[0], marker='.')[0])
    plt.hlines(3.0, plt.xlim()[0], plt.xlim()[1], color=colourWheel[0], linestyles=':')
    plt.ylim(ymin=0.95)
    plt.xlabel("Number of agents on each side")
    plt.ylabel("Ratio of average rank between tiers of men")
    plt.legend(series, ['Tier 2 : Tier 1'], loc='lower right')
    plt.show()

    series = []
    for j in [0,1]: series.append(plt.plot(np.linspace(80,10000,125),[np.mean(rl[i][j]) / np.mean(rl[i][2]) for i in range(rl.shape[0])], color=colourWheel[j], marker='.')[0])
    plt.hlines(3.0, plt.xlim()[0], plt.xlim()[1], color=colourWheel[0], linestyles=':')
    plt.hlines(1.5, plt.xlim()[0], plt.xlim()[1], color=colourWheel[1], linestyles=':')
    plt.ylim(ymin=0.95)
    plt.xlabel("Number of agents on each side")
    plt.ylabel("Ratio of average rank between tiers of women")
    plt.legend(series, ['Tier 1 : Tier 3', 'Tier 2 : Tier 3'], loc='lower right')
    plt.show()

    ## Distribution of matched pairs
    mgn1,m11gn1 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_grow_market-2020-02-06_00:03:07.out', 'r'))
    mgn2,m11gn2 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_grow_market-2020-02-06_23:49:26.out', 'r'))
    mgn3,m11gn3 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_grow_market-2020-02-07_02:18:33.out', 'r'))
    mgn4,m11gn4 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_grow_market-2020-02-07_20:22:47.out', 'r'))
    mgn5,m11gn5 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_grow_market-2020-02-07_21:16:52.out', 'r'))

    mgs1,m11gs1 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_vary_scores-2020-02-06_01:05:15.out', 'r'))
    mgs2,m11gs2 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_vary_scores-2020-02-07_00:49:34.out', 'r'))
    mgs3,m11gs3 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_vary_scores-2020-02-07_03:12:42.out', 'r'))
    mgs4,m11gs4 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_vary_scores-2020-02-07_21:15:34.out', 'r'))
    mgs5,m11gs5 = readMatchingStatisticsFromFile(open('Outputs/distribution_of_pairs_vary_scores-2020-02-07_22:08:09.out', 'r'))

    m11gn = np.concatenate([np.array(m11gn1), np.array(m11gn2), np.array(m11gn3), np.array(m11gn4), np.array(m11gn5)], 1)
    m11gs = np.concatenate([np.array(m11gs1), np.array(m11gs2), np.array(m11gs3), np.array(m11gs4), np.array(m11gs5)], 1).reshape((8,8,-1))

    # Growing market
    plt.plot(np.logspace(4,18,15,base=2), m11gn.mean(1)/np.logspace(4,18,15,base=2), color=colourWheel[0], marker='.')
    plt.plot(np.logspace(4,18,15,base=2), np.percentile(m11gn,97,axis=1)/np.logspace(4,18,15,base=2), color=colourWheel[0], ls='--')
    plt.plot(np.logspace(4,18,15,base=2), np.percentile(m11gn,3,axis=1)/np.logspace(4,18,15,base=2), color=colourWheel[0], ls='--')
    plt.hlines(0.5, plt.xlim()[0], plt.xlim()[1], color=colourWheel[1], linestyles=':')
    plt.xlabel("Number of agents on each side")
    plt.ylabel("Percentage of tier 1 men\nmatched to tier 1 women")
    plt.xscale('log')
    plt.show()

    # Varying scores
    # TODO

    ## Varying both parameters
    # Tiers on women
    plt.rc('figure', figsize=[11.2, 4.8])

    mmpvae,pmpvae,rmpvae = readMatchingResultFromFile(open('Outputs/varying_both_tier_params-2020-02-10_22:32:39.out', 'r'))
    p1mpvae = np.array(pmpvae)[:,0,:].mean(1).reshape((39,39))[:-2,:].T
    r1mpvae = np.array(rmpvae)[:,0,:].mean(1).reshape((39,39))[:-2,:].T
    r2mpvae = np.array(rmpvae)[:,1,:].mean(1).reshape((39,39))[:-2,:].T
    alphas = np.tile(np.linspace(1,10,37), [39,1])
    epsilons = np.tile(np.linspace(0.975,0.025,39).reshape((-1,1)), [1,37])

    plt.subplot(1, 2, 1)
    avgMs = alphas * epsilons * np.log(1000)
    plt.imshow(avgMs)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")

    plt.subplot(1, 2, 2)
    plt.imshow(p1mpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")
    plt.show()

    plt.subplot(1, 2, 1)
    avgW1s = 1000.0 / alphas / np.log(1000)
    plt.imshow(avgW1s )
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")

    plt.subplot(1, 2, 2)
    plt.imshow(r2mpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")
    plt.show()

    plt.subplot(1, 2, 1)
    avgW2s = np.ones((39, 37)) * 1000.0 / np.log(1000)
    plt.imshow(avgW2s)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")

    plt.subplot(1, 2, 2)
    plt.imshow(r1mpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("alpha_1")
    plt.ylabel("epsilon_1")
    plt.show()

    # Tiers on men
    mwpvae,pwpvae,rwpvae = readMatchingResultFromFile(open('Outputs/varying_both_tier_params_rev-2020-02-10_22:49:59.out', 'r'))
    p1wpvae = np.array(pwpvae)[:,0,:].mean(1).reshape((39,39))[:-2,:].T
    p2wpvae = np.array(pwpvae)[:,1,:].mean(1).reshape((39,39))[:-2,:].T
    r1wpvae = np.array(rwpvae)[:,0,:].mean(1).reshape((39,39))[:-2,:].T
    betas = np.tile(np.linspace(1,10,37), [39,1])
    deltas = np.tile(np.linspace(0.975,0.025,39).reshape((-1,1)), [1,37])

    plt.subplot(1, 2, 1)
    avgM1s = 1.0 / (deltas / betas + 1 - deltas) * np.log(1000) / betas
    plt.imshow(avgM1s)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")

    plt.subplot(1, 2, 2)
    plt.imshow(p2wpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")
    plt.show()

    plt.subplot(1, 2, 1)
    avgM2s = 1.0 / (deltas / betas + 1 - deltas) * np.log(1000)
    plt.imshow(avgM2s)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")

    plt.subplot(1, 2, 2)
    plt.imshow(p1wpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")
    plt.show()

    plt.subplot(1, 2, 1)
    avgWs = 1000.0 * (deltas * betas + 1 - deltas) * (deltas / betas + 1 - deltas) / np.log(1000)
    plt.imshow(avgWs)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")

    plt.subplot(1, 2, 2)
    plt.imshow(r1wpvae)
    plt.xticks(np.linspace(0,36,10), [int(x) for x in np.linspace(1,10,10)])
    plt.yticks(np.linspace(3,35,9), ['{:.1f}'.format(x) for x in np.linspace(0.9,0.1,9)])
    plt.colorbar()
    plt.xlabel("beta_1")
    plt.ylabel("delta_1")
    plt.show()

    plt.rc('figure', figsize=plt.rcParamsDefault.get('figure.figsize'))
    '''

    ## Almost balanced market (constant difference)
    baseSizes = np.logspace(6,20,15,base=2)

    mab,pab,rab = readMatchingResultFromFile(open('Outputs/almost_balanced_market_avg_rank-2020-04-28_16:13:26.out', 'r'))
    rab = np.array(rab)
    pab = np.array(pab)
    series = []
    series.append(plt.plot(baseSizes, [np.mean(pab[i][0]) for i in range(pab.shape[0])], marker='.', color=colourWheel[0])[0])
    series.append(plt.plot(baseSizes, [np.mean(pab[i][1]) for i in range(pab.shape[0])], marker='.', color=colourWheel[1])[0])
    series.append(plt.plot(baseSizes, [np.mean(rab[i][0]) for i in range(rab.shape[0])], marker='.', color=colourWheel[2])[0])
    series.append(plt.plot(baseSizes, [np.mean(rab[i][1]) for i in range(rab.shape[0])], marker='.', color=colourWheel[3])[0])
    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel("Number of men")
    plt.ylabel("Average rank of partners")
    plt.legend(series, ['Tier 1 men', 'Tier 2 men', 'Tier 1 women', 'Tier 2 women'], loc='upper left')
    plt.show()

    _,m11ab,m21ab = readMatchingStatisticsFromFile(open('Outputs/almost_balanced_market_matched_count-2020-04-28_16:13:26.out','r'), nSeries=2)
    m11ab = np.array(m11ab)
    m21ab = np.array(m21ab)
    series = []
    series.append(plt.plot(baseSizes, m11ab.mean(1) * 2.0 / baseSizes, color=colourWheel[0], marker='.')[0])
    plt.plot(baseSizes, np.percentile(m11ab,97,axis=1) * 2.0 / baseSizes, color=colourWheel[0], ls='--')
    plt.plot(baseSizes, np.percentile(m11ab,3,axis=1) * 2.0 / baseSizes, color=colourWheel[0], ls='--')
    series.append(plt.plot(baseSizes, m21ab.mean(1) * 2.0 / baseSizes, color=colourWheel[1], marker='.')[0])
    plt.plot(baseSizes, np.percentile(m21ab,97,axis=1) * 2.0 / baseSizes, color=colourWheel[1], ls='--')
    plt.plot(baseSizes, np.percentile(m21ab,3,axis=1) * 2.0 / baseSizes, color=colourWheel[1], ls='--')
    plt.hlines(0.5, plt.xlim()[0], plt.xlim()[1], color=colourWheel[2], linestyles=':')
    plt.xlabel("Number of men")
    plt.ylabel("Percentage of men in each tier\nmatched to tier 1 women")
    plt.legend(series, ['Tier 1', 'Tier 2'], loc='upper right')
    plt.xscale('log')
    plt.show()

