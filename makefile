CXX := g++
CXXFLAGS := -DHAVE_CONFIG_H -std=c++11 -pthread -pedantic -Wall -Wextra -Weffc++ -Werror -fno-default-inline -g -O2
INCLUDES :=

LIBS     := -ljemalloc -lm -pthread -lprotobuf -lpthread -ljemalloc -ludt
OBJECTS  := multi-queue.o network.o pkt-logger.o utilities.o ctcp-sender.o rand-gen.o traffic-generator.o multidelta-queue.o markoviancc.o random.o

all: simulator

simulator: $(OBJECTS) simulator.o
	$(CXX) $(inputs) -o $(output) $(LIBS)

%.o: %.cc
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $(input) -o $(output)

clean:
	trash $(OBJECTS)
	trash simulator

.PHONY: all clean
