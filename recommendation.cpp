#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "utilities.h"

#define ARGS 5

using namespace std;

int main(int argc, char** argv)
{
	short int inputFileIndex, outputFileIndex;
	bool validateFlag = false;
	int k = 4;
	int L = 6;

	/*== check the #args*/	
	rerunCheck(argc, ARGS);

	/*== get inline arguments*/
	getInlineArguments(argc, argv, inputFileIndex, outputFileIndex, validateFlag);

	/*== create a map with the sentimental value of every word*/
	map<string, float> vaderLexicon = vaderLexiconMap();
	
	/*== create a map with every crypto*/
	map<string, int> cryptos = cryptosMap();
	
	/*== construct user vectors*/
	vector<vector<double>> users = createUserVector(argv, inputFileIndex, vaderLexicon, cryptos);

	/*== normalize user vectors*/
	normalisation(users);

	/*== create and fill hash table*/
	HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(users, argv, inputFileIndex, k, L);

	exit(EXIT_SUCCESS);	
}
