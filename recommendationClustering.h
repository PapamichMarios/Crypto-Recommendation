#ifndef __RECOMMENDATION_CLUSTERING_H__
#define __RECOMMENDATION_CLUSTERING_H__

#include <vector>
#include <string>
#include <map>

/*== 1 ==*/
void recommendationClustering(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::map<int, std::string> cryptosIndex, std::string outfile);

/*== 2 ==*/
void recommendationClustering(std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<std::vector<double>> virtual_users, std::vector<std::vector<double>> normalised_virtual_users, std::map<int, std::string> cryptosIndex, std::string outfile);

std::vector<double> Clustering_calculateRatings(std::vector<double> user, std::vector<double> normalisedUser, int userIndex, std::vector<std::vector<double>> users, std::vector<std::vector<double>> normalisedUsers, std::vector<int> labels, bool normalised);
#endif
