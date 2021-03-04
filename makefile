CXX=g++
OPTFLAG=-O3
CXXFLAGS=-Wall -g $(OPTFLAG) -std=c++11

EXECUTABLES=main growingMarketLin growingMarketLog varyingTier varyingTierBothParams uniquePartnerCount imbalancedMarket almostBalancedMarket distributionOfPairs distributionOfRanksAnyStableMatching distributionOfRanksAnyStableMatching_cp

all: $(EXECUTABLES)

main: main.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -o main main.o Matching.o Agent.o utils.o

almostBalancedMarket: almostBalancedMarket.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o almostBalancedMarket almostBalancedMarket.o Matching.o Agent.o utils.o

distributionOfPairs: distributionOfPairs.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o distributionOfPairs distributionOfPairs.o Matching.o Agent.o utils.o

distributionOfRanksAnyStableMatching: distributionOfRanksAnyStableMatching.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o distributionOfRanksAnyStableMatching distributionOfRanksAnyStableMatching.o Matching.o Agent.o utils.o

distributionOfRanksAnyStableMatching_cp: distributionOfRanksAnyStableMatching_cp.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o distributionOfRanksAnyStableMatching_cp distributionOfRanksAnyStableMatching_cp.o Matching.o Agent.o utils.o

growingMarketLin: growingMarketLin.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o growingMarketLin growingMarketLin.o Matching.o Agent.o utils.o

growingMarketLog: growingMarketLog.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o growingMarketLog growingMarketLog.o Matching.o Agent.o utils.o

imbalancedMarket: imbalancedMarket.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o imbalancedMarket imbalancedMarket.o Matching.o Agent.o utils.o

uniquePartnerCount: uniquePartnerCount.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o uniquePartnerCount uniquePartnerCount.o Matching.o Agent.o utils.o

varyingTier: varyingTier.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o varyingTier varyingTier.o Matching.o Agent.o utils.o

varyingTierBothParams: varyingTierBothParams.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o varyingTierBothParams varyingTierBothParams.o Matching.o Agent.o utils.o

main.o: main.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c main.cc

almostBalancedMarket.o: almostBalancedMarket.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c almostBalancedMarket.cc

distributionOfPairs.o: distributionOfPairs.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c distributionOfPairs.cc

distributionOfRanksAnyStableMatching.o: distributionOfRanksAnyStableMatching.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c distributionOfRanksAnyStableMatching.cc

distributionOfRanksAnyStableMatching_cp.o: distributionOfRanksAnyStableMatching_cp.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c distributionOfRanksAnyStableMatching_cp.cc

growingMarketLin.o: growingMarketLin.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c growingMarketLin.cc

growingMarketLog.o: growingMarketLog.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c growingMarketLog.cc

imbalancedMarket.o: imbalancedMarket.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c imbalancedMarket.cc

varyingTier.o: varyingTier.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c varyingTier.cc

varyingTierBothParams.o: varyingTierBothParams.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c varyingTierBothParams.cc

uniquePartnerCount.o: uniquePartnerCount.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c uniquePartnerCount.cc

Matching.o: Matching.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c Matching.cc

Agent.o: Agent.cc Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c Agent.cc

utils.o: utils.cc
	$(CXX) $(CXXFLAGS) -c utils.cc

clean:
	$(RM) $(EXECUTABLES) *.o *~ 
