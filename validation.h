#ifndef __VALIDATION_H__
#define __VALIDATION_H__

#include <vector>

#include "hash_table.h"

double F_FoldCrossValidation_LSH(HashTable<std::vector<double>> ** hash_tableptr, int L, std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers);

double F_FoldCrossValidation_Clustering(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<int> labels);

int assignUnknown(std::vector<double> user);

#endif
