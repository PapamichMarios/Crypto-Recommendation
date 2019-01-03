CPP      = g++
OBJ      = recommendation.o recommendationLSH.o utilities.o g.o fi.o hyperplane.o recommendationClustering.o clustering.o clustering_utilities.o validation.o
BIN      = recommendation
CFLAGS   = -g -std=c++11

$(BIN): $(OBJ)
	$(CPP) -o $(BIN) $(OBJ) $(CFLAGS)

recommendation.o: recommendation.cpp
	$(CPP) -c recommendation.cpp $(CFLAGS)

recommendationLSH.o: recommendationLSH.cpp
	$(CPP) -c recommendationLSH.cpp $(CFLAGS)

recommendationClustering.o: recommendationClustering.cpp
	$(CPP) -c recommendationClustering.cpp $(CFLAGS)

clustering.o: clustering.cpp
	$(CPP) -c clustering.cpp $(CFLAGS)

clustering_utilities.o: clustering_utilities.cpp
	$(CPP) -c clustering_utilities.cpp $(CFLAGS)

validation.o: validation.cpp
	$(CPP) -c validation.cpp $(CFLAGS)

utilities.o: utilities.cpp
	$(CPP) -c utilities.cpp $(CFLAGS)

fi.o: fi.cpp
	$(CPP) -c fi.cpp $(CFLAGS)

g.o: g.cpp
	$(CPP) -c g.cpp $(CFLAGS)

hyperplane.o: hyperplane.cpp
	$(CPP) -c hyperplane.cpp $(CFLAGS)

PHONY: clean

clean: 
	rm -f $(OBJ) $(BIN) 
