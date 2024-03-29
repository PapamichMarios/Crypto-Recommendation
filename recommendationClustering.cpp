#include <ctime>

#include "recommendationClustering.h"
#include "clustering_utilities.h"
#include "clustering.h"
#include "utilities.h"
#include "validation.h"

using namespace std;

vector<double> Clustering_calculateRatings(vector<double> user, vector<double> normalisedUser, int userIndex, vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<int> labels, bool normalised)
{
	/*== find the cryptos the user has referenced*/
	vector<int> cryptos_referenced(user.size());
	for(unsigned int i=0; i<user.size(); i++)
	{
		if(user.at(i) != INT_MAX)
			cryptos_referenced.at(i) = 1;
	}

	/*== turn INT_MAX to 0s*/
	vector<double> eliminatedUser = eliminateUnknown(user);

	/*== make user equal to normalised user*/
	user = normalisedUser;

	double zeta=0;
	for(unsigned int i=0; i<labels.size(); i++)
	{
		/*== get same cluster users*/
		if(labels[i] != labels[userIndex] || userIndex == i)
			continue;

		double similarity;
		if(normalised)
			similarity = euclidean_similarity(normalisedUser, normalisedUsers[i]);
		else
			similarity = euclidean_similarity(eliminatedUser, eliminateUnknown(users[i]));

		zeta += abs(similarity);

		for(unsigned int j=0; j<user.size(); j++)
		{
			/*== If a user has mentioned a crypto, we dont want to recommend it by accident.
				 We make it equal to INT_MIN so when searching for the max sentiment score 
				 on the vector we ll have no chance to pick it
			==*/
			if(cryptos_referenced.at(j) == 1)
			{
				user.at(j) = INT_MIN;
				continue;
			}

			user[j] += similarity * normalisedUsers[i][j];
		}
	}

	/*== in case no neighbours were found*/
	if(zeta == 0)
	{
		vector<double> zero_neighbours_user;
		return zero_neighbours_user;
	}
	
	/*== else calculate the vector based on neighbours*/
	zeta = 1/zeta;

	for(unsigned int j=0; j<user.size(); j++)
	{
		if(cryptos_referenced.at(j) == 1)
			continue;

		user[j] *= zeta;
	}

	return user;
}

/*==== 1. */
void recommendationClustering(vector<vector<double>> users, vector<vector<double>> normalisedUsers, map<int, string> cryptosIndex, string outfile)
{
	printRecommendationTitle(outfile, "Clustering");
	time_t start_time = clock();

	/*== cluster users*/
	vector<int> labels = k_meanspp(normalisedUsers, users, CLUSTER_USERS);

	/*== recommendation*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		bool normalised = true;
		vector<double> normalisedUser = normalisedUsers.at(i);
		if (!recommendationEligibility(normalised, normalisedUser, users.at(i)))
		{
			printUnmatched(cryptosIndex, i, outfile);
			continue;
		}

		/*== get the user ratings*/
		vector<double> average_user = Clustering_calculateRatings(users.at(i), normalisedUser, i, users, normalisedUsers, labels, normalised);

		/*== in case the user has no neighbours*/
		if(average_user.size() == 0)
		{
			printUnmatched(cryptosIndex, i , outfile);
			continue;
		}

		/*== get the 5 best results*/
		vector<int> recommendations = cryptosRecommendedByNeighbourhood(average_user);

		/*== print prediction*/
		if(recommendations.size() > 0)
			printRecommendation(cryptosIndex, recommendations, i, outfile, RECOMMENDATION_A);
		else
			printUnmatched(cryptosIndex, i, outfile);
	}

	printRecommendationTimer(outfile, (double)(clock() - start_time)/CLOCKS_PER_SEC);

}

/*==== 2. */
void recommendationClustering(vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<vector<double>> virtual_users, vector<vector<double>> normalised_virtual_users, map<int, string> cryptosIndex, string outfile)
{
	printRecommendationTitle(outfile, "Clustering");
	time_t start_time = clock();

	/*== recommendation*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		bool normalised = true;
		vector<double> normalisedUser = normalisedUsers.at(i);
		if ( !recommendationEligibility(normalised, normalisedUser, users.at(i)) )
		{
			printUnmatched(cryptosIndex, i, outfile);
			continue;
		}

		/*== create a vector with virtual users + the user we want to recommend*/
		vector<vector<double>> all_users;
		all_users = normalised_virtual_users;
		all_users.push_back(normalisedUser);

		vector<int> labels = k_meanspp(all_users, virtual_users, CLUSTER_VIRTUAL);

		/*== get the user ratings*/
		vector<double> average_user = Clustering_calculateRatings(users.at(i), normalisedUser, labels.size()-1, virtual_users, normalised_virtual_users, labels, normalised);

		/*== in case the user has no neighbours*/
		if(average_user.size() == 0)
		{
			printUnmatched(cryptosIndex, i , outfile);
			continue;
		}

		/*== get the 5 best results*/
		vector<int> recommendations = cryptosRecommendedByNeighbourhood(average_user);

		/*== print prediction*/
		if(recommendations.size() > 0)
			printRecommendation(cryptosIndex, recommendations, i, outfile, RECOMMENDATION_B);
		else
			printUnmatched(cryptosIndex, i, outfile);
	}

	printRecommendationTimer(outfile, (double)(clock() - start_time)/CLOCKS_PER_SEC);
}
