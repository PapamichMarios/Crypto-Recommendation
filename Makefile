CPP      = g++
OBJ      = recommendation.o utilities.o
BIN      = recommendation
CFLAGS   = -g -std=c++11 -Wall

$(BIN): $(OBJ)
	$(CPP) -o $(BIN) $(OBJ) $(CFLAGS)

recommendation.o: recommendation.cpp
	$(CPP) -c recommendation.cpp $(CFLAGS)

utilities.o: utilities.cpp
	$(CPP) -c utilities.cpp $(CFLAGS)

PHONY: clean

clean: 
	rm -f $(OBJ) $(BIN) 
