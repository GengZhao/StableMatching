CXX=g++
OPTFLAG=-O3
CXXFLAGS=-Wall -g $(OPTFLAG) -std=c++11

EXECUTABLES=main experimentFixedTiersGrowingMarket

all: $(EXECUTABLES)

main: main.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -o main main.o Matching.o Agent.o utils.o

experimentFixedTiersGrowingMarket: experimentFixedTiersGrowingMarket.o Matching.o Agent.o utils.o
	$(CXX) $(CXXFLAGS) -pthread -o experimentFixedTiersGrowingMarket experimentFixedTiersGrowingMarket.o Matching.o Agent.o utils.o

main.o: main.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c main.cc

experimentFixedTiersGrowingMarket.o: experimentFixedTiersGrowingMarket.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c experimentFixedTiersGrowingMarket.cc

Matching.o: Matching.cc Matching.h Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c Matching.cc

Agent.o: Agent.cc Agent.h utils.h
	$(CXX) $(CXXFLAGS) -c Agent.cc

utils.o: utils.cc
	$(CXX) $(CXXFLAGS) -c utils.cc

clean:
	$(RM) $(EXECUTABLES) *.o *~ 
