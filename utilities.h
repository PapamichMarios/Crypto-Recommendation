#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>

#include "hash_table.h"

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag);

/*== maps for sentimental score*/
std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();
std::map<int, std::string> cryptosIndexMap();

/*== user vectors && normalisation*/
std::vector<std::vector<double>> createUserVector(char ** argv, short int inputFileIndex, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
void normalisation(std::vector<std::vector<double>> &users);
bool vectorIsZero(std::vector<double> v);

/*== lsh functions*/
HashTable<std::vector<double>> ** createAndFillHashTable(std::vector<std::vector<double>> users, char ** argv, short int inputFileIndex, int k, int L);
void recommendationLSH(HashTable<std::vector<double>> ** hash_tableptr, std::vector<std::vector<double>> users, std::map<int, std::string> cryptos, int L);

#endif
