#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "utilities.h"
#include "recommendationLSH.h"
#include "hash_table.h"

#define ARGS 5

using namespace std;

int main(int argc, char** argv)
{
	short int inputFileIndex, outputFileIndex;
	bool validateFlag = false;
	int k = 8;
	int L = 3;

	/*== check the #args*/	
	rerunCheck(argc, ARGS);

	/*== get inline arguments*/
	getInlineArguments(argc, argv, inputFileIndex, outputFileIndex, validateFlag);

	/*== clear outputfile */
	resetOutput(argv[outputFileIndex]);

	/*== create a map with the sentimental value of every word*/
	map<string, float> vaderLexicon = vaderLexiconMap();
	
	/*== create a map crypto : index*/
	map<string, int> cryptos = cryptosMap();
	
	/*== create a map index : crypto*/
	map<int, string> cryptosIndex = cryptosIndexMap();

	/*== construct user vectors*/
	vector<vector<double>> users = createUserVector(argv[inputFileIndex], vaderLexicon, cryptos);

	/*== normalize user vectors*/
	vector<vector<double>> normalisedUsers = users;
	normalisation(normalisedUsers);

	for(unsigned int i=0; i<users.size(); i++)
		users.at(i) = eliminateUnknown(users.at(i));

	/*== recommendation process using LSH*/
	recommendationLSHProcess(users, normalisedUsers, cryptosIndex, k, L, argv, inputFileIndex, outputFileIndex);

	exit(EXIT_SUCCESS);	
}
