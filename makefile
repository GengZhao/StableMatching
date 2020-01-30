CXX=g++
OPTFLAG=-O3
CXXFLAGS=-Wall -g $(OPTFLAG) -std=c++11

EXECUTABLES=main experimentFixedTiersGrowingMarket

all: $(EXECUTABLES)

main: main.o Matching.o Agent.o
	$(CXX) $(CXXFLAGS) -o main main.o Matching.o Agent.o

experimentFixedTiersGrowingMarket: experimentFixedTiersGrowingMarket.o Matching.o Agent.o
	$(CXX) $(CXXFLAGS) -pthread -o experimentFixedTiersGrowingMarket experimentFixedTiersGrowingMarket.o Matching.o Agent.o

main.o: main.cc Matching.h Agent.h
	$(CXX) $(CXXFLAGS) -c main.cc

experimentFixedTiersGrowingMarket.o: experimentFixedTiersGrowingMarket.cc Matching.h Agent.h
	$(CXX) $(CXXFLAGS) -c experimentFixedTiersGrowingMarket.cc

Matching.o: Matching.cc Matching.h Agent.h
	$(CXX) $(CXXFLAGS) -c Matching.cc

Agent.o: Agent.cc Agent.h
	$(CXX) $(CXXFLAGS) -c Agent.cc

clean:
	$(RM) $(EXECUTABLES) *.o *~ 

