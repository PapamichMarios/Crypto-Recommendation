#ifndef __VALIDATION_H__
#define __VALIDATION_H__

#include <vector>

#include "hash_table.h"

void F_FoldCrossValidation(int k, int L, std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::string inputfile, std::string outputfile);

void F_FoldCrossValidation(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::string outputfile);

int assignUnknown(std::vector<double> user);

void virtualValidation(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<std::vector<double>> virtual_users, std::vector<std::vector<double>> normalised_virtual_users, std::string outfile);

void virtualValidation(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<std::vector<double>> virtual_users, std::vector<std::vector<double>> normalised_virtual_users, int k, int L, std::string inputfile, std::string outputfile);
#endif
