#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>

#include "hash_table.h"

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag);
std::vector<double> eliminateUnknown(std::vector<double>);

/*== maps for sentimental score*/
std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();
std::map<int, std::string> cryptosIndexMap();

/*== user vectors && normalisation*/
std::vector<std::vector<double>> createUserVector(std::string inputfile, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
void normalisation(std::vector<std::vector<double>> &users);
std::vector<double> single_normalisation(std::vector<double> user);
bool vectorIsZero(std::vector<double> v);


void printRecommendation(std::map<int, std::string> cryptos, std::vector<int> recommendations, int user, std::string outputfile);
void printRecommendationTitle(std::string outputfile, std::string title);
void printRecommendationTimer(std::string outputfile, double time);
void resetOutput(std::string outputfile);

#endif
