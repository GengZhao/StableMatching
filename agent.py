import random

class Agent:
    def __init__(self, idx, label):
        self.idx = idx
        self.label = label
        self.match = None

    def matchedIdx(self):
        if self.match:
            return self.match.idx
        return None
        
    def registerPartnerPool(self, agents):
        self.partnerPool = agents;
        self.tbd = range(len(agents))
        self.preferences = []
        self.rejections = []

    def propose(self, selector=random.choice):
        if len(self.tbd) == 0:
            return -1

        toPropose = selector(self.tbd)
        self.tbd.remove(toPropose)
        self.preferences.append(toPropose)

        # print '[PROPOSAL]', self.idx, '=>', toPropose

        return toPropose

    def handleProposal(self, agent, acceptanceDecider):
        if agent.idx in self.rejections:
            raise "Circular proposal sequence"
        prevMatch = self.match
        if prevMatch == None:
            self.match = agent
            agent.match = self
            self.tbd.remove(agent.idx)
            self.preferences.insert(0, agent.idx)
            # print '[ACCEPTANCE]', agent.idx, '<=', self.idx
            return True
        self.tbd.remove(agent.idx)
        if acceptanceDecider(self.preferences, agent.idx):
            self.preferences.insert(0, agent.idx)
            self.reject(self.partnerPool[prevMatch.idx])
            # print '[REJECTION]', prevMatch.idx, '<=', self.idx
            self.match = agent
            agent.match = self
            # print '[ACCEPTANCE]', agent.idx, '<=', self.idx
            return True
        self.preferences.append(agent.idx)
        self.reject(agent)
        # print '[REJECTION]', agent.idx, '<=', self.idx
        return False

    def reportResult(self):
        if self.label == 'L':
            print '[MATCH]', self.idx, '=>', self.matchedIdx()
        else:
            print '[MATCH]', self.matchedIdx(), '<=', self.idx
        # print '[PREFERENCES]', self.preferences

    def reject(self, agent):
        agent.match = None
        self.rejections.append(agent.idx)
