from agent import Agent
import random

class GeneralMatching(object):

    def __init__(self, tierSizesMen, tierSizesWomen, scoresMen, scoresWomen, quiet=True):
        ''' Params:
        tierSizesMen List[Int]: list of length k, number of men in each tier
        tierSizesWomen List[Int]: list of length k, number of women in each tier
        scoresMen List[Int]: list of length k, fitness scores for each tier of men
        scoresWomen List[Int]: list of length k, fitness scores for each tier of women
        quiet Boolean: print out debug messages and info
        '''
        self.quiet = quiet
        self.tierSizesMen = tierSizesMen
        self.tierSizesWomen = tierSizesWomen
        self.scoresMen = scoresMen
        self.scoresWomen = scoresWomen

        self.men = [Agent(idx, 'Man') for idx in range(nLAgents)]
        self.rAgents = [Agent(i, 'R') for i in range(nRAgents)]
        for lAgent in self.lAgents:
            lAgent.registerPartnerPool(self.rAgents)
        for rAgent in self.rAgents:
            rAgent.registerPartnerPool(self.lAgents)
        self.proposalCounts = {i: 0 for i in range(nRAgents)}

    # Selector function for making proposal
    def proposalSelector(self, tbd):
        return random.choice(tbd)

    # Decider function for proposal acceptance
    def acceptanceDecider(self, preferences, candidate):
        return random.choice([True, False])

    def collectResults(self, verbose=False):
        lAvg = float(sum([len(a.preferences) for a in self.lAgents])) / self.nLAgents
        rAvg = float(sum([len(a.preferences) for a in self.rAgents])) / self.nRAgents

        if not self.quiet or verbose:
            print 'Results'
            print 'L =>'
            for lAgent in self.lAgents:
                lAgent.reportResult()
            print 'L avg. pref. count', lAvg
            # print '<= R'
            # for rAgent in self.rAgents:
            #     rAgent.reportResult()
            print 'R avg. pref. count', rAvg
        return [lAvg, rAvg]

    def run(self, verbose=False):
        matched = lambda a: (a.match != None or len(a.tbd) == 0)
        # matchingChanged = False
        while not all(map(matched, self.lAgents)):
            # matchingChanged = False
            for lAgent in self.lAgents:
                if matched(lAgent):
                    continue
                proposal = lAgent.propose(self.proposalSelector)
                self.proposalCounts[proposal] += 1
                if proposal < 0:
                    raise 'Cannot find a matching'
                self.rAgents[proposal].handleProposal(lAgent, self.acceptanceDecider)
                # matchingChanged = matchingChanged or accepted
                # self.collectResults()
                # raw_input("Next... [ENTER]")
                # print ''
        return self.collectResults(verbose=verbose)
