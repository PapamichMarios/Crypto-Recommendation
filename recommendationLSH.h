#ifndef __RECOMMENDATIONLSH_H__
#define __RECOMMENDATIONLSH_H__

#include <vector>

#include "hash_table.h"

void recommendationLSH(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::map<int, std::string> cryptosIndex, int k, int L, std::string inputfile, std::string outputfile);
void recommendationLSH(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<std::vector<double>> virtual_users, std::vector<std::vector<double>> normalised_virtual_users, std::map<int, std::string> cryptosIndex, int k, int L, std::string inputfile, std::string outputfile);

HashTable<std::vector<double>> ** createAndFillHashTable(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::string inputfile, int k, int L);

std::vector<double> LSH_calculateRatings(HashTable<std::vector<double>> ** hash_tableptr, std::vector<double> user, std::vector<double> normalisedUser, std::vector<std::vector<double>> normalisedUsers, int L, bool normalised);

void LSH_groupNeighboursFromTables(std::vector<std::vector<int>> all_neighbours, std::vector<std::vector<double>> all_distances, std::vector<int> &neighbours_unique, std::vector<double> &neighbours_distance, int L);

void LSH_calculateNeighbourhood(std::vector<std::vector<int>> all_neighbours, std::vector<std::vector<double>> all_distances, std::vector<int> &neighbours, std::vector<double> &similarity, int L);

void LSH_predictUnknownCryptos(std::vector<double> &user, std::vector<double> normalisedUser, std::vector<std::vector<double>> users, std::vector<double> similarity, std::vector<int> neighbours);

void unallocateHashTable(HashTable<std::vector<double>> ** hash_tableptr, int L);
#endif
