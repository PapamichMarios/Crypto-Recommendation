#ifndef __RECOMMENDATIONLSH_H__
#define __RECOMMENDATIONLSH_H__

#include <vector>

#include "hash_table.h"

HashTable<std::vector<double>> ** createAndFillHashTable(std::vector<std::vector<double>> users, char ** argv, short int inputFileIndex, int k, int L);

void recommendationLSH(HashTable<std::vector<double>> ** hash_tableptr, std::vector<std::vector<double>> users, std::map<int, std::string> cryptos, int L, std::string outputfile);

std::vector<int> groupNeighboursFromTables(std::vector<std::vector<int>> all_neighbours, int L);

std::vector<int> cryptosRecommendedByNeighbourhood(std::vector<double> user);

void predictUnknownCryptosPreferences(std::vector<std::vector<double>> &users, std::vector<double> similarity,std::vector<int> neighbours, unsigned int i);
#endif
