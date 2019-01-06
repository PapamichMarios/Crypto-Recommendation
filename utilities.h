#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <vector>

#include "hash_table.h"

void rerunCheck(int argc, int args);
void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag, short int &tweetsFileIndex);
void resetOutput(std::string outputfile);
std::vector<double> eliminateUnknown(std::vector<double>);
int getInputLines(std::string inputfile);
bool recommendationEligibility(bool &normalised, std::vector<double> &normalisedUser, std::vector<double> &user);

/*== maps for sentimental score*/
std::map<std::string, float> vaderLexiconMap();
std::map<std::string, int> cryptosMap();
std::map<int, std::string> cryptosIndexMap();

/*== user vectors && normalisation*/
std::vector<std::vector<double>> createUserVector(std::string inputfile, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
std::vector<std::vector<double>> createTweetVector(std::string inputfile);
std::map<int, std::vector<double>> createTweetMap(std::string inputfile, std::map<std::string, float> vaderLexicon, std::map<std::string, int> cryptos);
std::vector<std::vector<double>> createVirtualUsers(std::map<int, std::vector<double>> tweets, std::vector<int> labels, int cluster_size, int crypto_size);
std::vector<std::vector<double>> createVirtualUsersNormalised(std::map<int, std::vector<double>> tweets, std::vector<int> labels, int cluster_size, int crypto_size);

void normalisation(std::vector<std::vector<double>> &users);
void normalisation(std::map<int, std::vector<double>> &tweets);
bool vectorIsZero(std::vector<double> v);

/*== print functions*/
void printRecommendation(std::map<int, std::string> cryptos, std::vector<int> recommendations, int user, std::string outputfile, int how_many);
void printUnmatched(std::map<int, std::string> cryptos, int user, std::string outputfile);
void printRecommendationTitle(std::string outputfile, std::string title);
void printRecommendationTimer(std::string outputfile, double time);
void printRecommendationMAE(std::string outputfile, std::string text, double MAE);

double cosine_similarity(const std::vector<double> u, const std::vector<double> v);
double euclidean_similarity(const std::vector<double> u, const std::vector<double> v);
#endif
