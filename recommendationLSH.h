#ifndef __RECOMMENDATIONLSH_H__
#define __RECOMMENDATIONLSH_H__

#include <vector>

#include "hash_table.h"

void recommendationLSHProcess(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::map<int, std::string> cryptosIndex, int k, int L, char ** argv, short int inputFileIndex, short int outputFileIndex);

HashTable<std::vector<double>> ** createAndFillHashTable(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, char ** argv, short int inputFileIndex, int k, int L);

void recommendationLSH(HashTable<std::vector<double>> ** hash_tableptr, std::vector<double> user, std::vector<double> normalisedUser, int userIndex, std::vector<std::vector<double>> users, std::map<int, std::string> cryptos, int L, std::string outputfile, bool normalised);

void groupNeighboursFromTables(std::vector<std::vector<int>> all_neighbours, std::vector<std::vector<double>> all_distances, std::vector<int> &neighbours_unique, std::vector<double> &neighbours_distance, int L);

void calculateNeighbourhood(std::vector<std::vector<int>> all_neighbours, std::vector<std::vector<double>> all_distances, std::vector<int> &neighbours, std::vector<double> &similarity, int L);

std::vector<int> cryptosRecommendedByNeighbourhood(std::vector<double> user);

void predictUnknownCryptos(std::vector<double> &user, std::vector<double> normalisedUser, std::vector<std::vector<double>> users, std::vector<double> similarity, std::vector<int> neighbours);
void predictUnknownCryptos_normalised(std::vector<double> &user, std::vector<std::vector<double>> users, std::vector<double> similarity, std::vector<int> neighbours);
#endif
