#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>

#include "hash_table.h"

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag);

/*== maps for sentimental score*/
std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();

/*== user vectors && normalisation*/
std::vector<std::vector<double>> createUserVector(char ** argv, short int inputFileIndex, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
void normalisation(std::vector<std::vector<double>> &users);

/*== lsh functions*/
HashTable<std::vector<double>> ** createAndFillHashTable(std::vector<std::vector<double>> users, char ** argv, short int inputFileIndex, int k, int L);

#endif
