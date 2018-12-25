#include <cstdlib>
#include <iostream>
#include <map>

#include "utilities.h"

#define ARGS 5

using namespace std;

int main(int argc, char** argv)
{
	short int inputFileIndex, outputFileIndex;
	bool validateFlag = false;

	/*== check the #args*/	
	rerunCheck(argc, ARGS);

	/*== get inline arguments*/
	getInlineArguments(argc, argv, inputFileIndex, outputFileIndex, validateFlag);

	/*== create a map with the sentimental value of every word*/
	map<string, float> vaderLexicon = vaderLexiconMap();
	
	/*== create a map with every crypto*/
	map<string, int> cryptos = cryptosMap();
	

	exit(EXIT_SUCCESS);	
}
