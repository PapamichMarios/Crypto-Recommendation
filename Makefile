CPP      = g++
OBJ      = recommendation.o recommendationLSH.o utilities.o g.o fi.o hyperplane.o
BIN      = recommendation
CFLAGS   = -g -std=c++11

$(BIN): $(OBJ)
	$(CPP) -o $(BIN) $(OBJ) $(CFLAGS)

recommendation.o: recommendation.cpp
	$(CPP) -c recommendation.cpp $(CFLAGS)

recommendationLSH.o: recommendationLSH.cpp
	$(CPP) -c recommendationLSH.cpp $(CFLAGS)

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
