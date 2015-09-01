CXX := g++
CXXFLAGS := -DHAVE_CONFIG_H -std=c++11 -pthread -pedantic -Wall -Wextra -Weffc++ -Werror -fno-default-inline -g -O2 
INCLUDES := 

LIBS     := -ljemalloc -lm -pthread -lprotobuf -lpthread -ljemalloc -ludt
OBJECTS  := simulator.o multi-queue.o network.o pkt-logger.o rand-gen.o

all: simulator

.PHONY: all

sender: $(OBJECTS) simulator.o
	$(CXX) $(inputs) -o $(output) $(LIBS)

%.o: %.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(input) -o $(output)