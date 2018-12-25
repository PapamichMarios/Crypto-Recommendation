#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag);

std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();

std::vector<std::vector<double>> createUserVector(char ** argv, short int inputFileIndex, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
void normalisation(std::vector<std::vector<double>> &users);

#endif
