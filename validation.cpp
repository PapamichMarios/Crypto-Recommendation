#include <climits>
#include <iostream>

#include "validation.h"
#include "recommendationLSH.h"
#include "recommendationClustering.h"
#include "utilities.h"
#include "hash_table.h"

#define FOLDS 10

using namespace std;

/*=== 1 ===*/
double F_FoldCrossValidation_LSH(HashTable<vector<double>> ** hash_tableptr, int L, vector<vector<double>> users, vector<vector<double>> normalisedUsers)
{
	int data_size = users.size();
	
	bool oddFlag = false;
	if(data_size % 2 == 1)
		oddFlag = true;

	int partition_size = data_size/FOLDS;
	double average_MAE=0;
	
	for(int i=0; i<FOLDS; i++)
	{
		double MAE=0;
		int excluded=0;
		int plus_one=0;
		/*== get one part of the users set*/
		for(int j=0; j<partition_size; j++)
		{
			int index = i*partition_size + j + plus_one;

			/*== check wether we need to ignore the vector*/
			bool normalised = true;
			if (!recommendationEligibility(normalised, normalisedUsers[index], users[index]) )
			{
				excluded++;
				continue;
			}

			/*== assign one random mentioned crypto to unknown*/
			int unknownIndex = assignUnknown(users[index]);
			if(unknownIndex == -1)
			{
				excluded++;
				continue;
			}

			double sentiment = normalisedUsers[index][unknownIndex];
			normalisedUsers[index][unknownIndex] = 0;
			users[index][unknownIndex] = INT_MAX;

			/*== calculate the ratings of the user based on the clusters*/
			vector<double> average_user = LSH_calculateRatings(hash_tableptr, users[index], normalisedUsers[index], normalisedUsers, L, normalised);

			/*== compare the crypto we assigned as unknown to its real value*/
			double predicted_sentiment = average_user[unknownIndex];
			MAE += sentiment - predicted_sentiment;

			if( i == FOLDS - 1 && oddFlag && j == partition_size-1)
			{
				j--;
				plus_one++;
			}
		}

		average_MAE += MAE/(partition_size-excluded);
	}

	return (average_MAE / FOLDS);
}

double F_FoldCrossValidation_Clustering(vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<int> labels)
{
	int data_size = users.size();
	
	bool oddFlag = false;
	if(data_size % 2 == 1)
		oddFlag = true;

	int partition_size = data_size/FOLDS;
	double average_MAE=0;
	
	for(int i=0; i<FOLDS; i++)
	{
		double MAE=0;
		int excluded=0;
		int plus_one=0;
		/*== get one part of the users set*/
		for(int j=0; j<partition_size; j++)
		{
			int index = i*partition_size + j + plus_one;

			/*== check wether we need to ignore the vector*/
			bool normalised = true;
			/*== check wether we need to ignore the vector*/
			bool normalised = true;
			if (!recommendationEligibility(normalised, normalisedUsers[index], users[index]) )
			{
				excluded++;
				continue;
			}

			/*== assign one random mentioned crypto to unknown*/
			int unknownIndex = assignUnknown(users[index]);
			if(unknownIndex == -1)
			{
				excluded++;
				continue;
			}

			double sentiment = normalisedUsers[index][unknownIndex];
			normalisedUsers[index][unknownIndex] = 0;
			users[index][unknownIndex] = INT_MAX;

			/*== calculate the ratings of the user based on the clusters*/
			vector<double> average_user = Clustering_calculateRatings(users[index], normalisedUsers[index], index, users, normalisedUsers, labels, normalised);

			/*== compare the crypto we assigned as unkown to its real value*/
			double predicted_sentiment = average_user[unknownIndex];
			MAE += sentiment - predicted_sentiment;

			if( i == FOLDS - 1 && oddFlag && j == partition_size-1)
			{
				j--;
				plus_one++;
			}
		}

		average_MAE += MAE/(partition_size-excluded);
	}

	return (average_MAE / FOLDS);
}

int assignUnknown(vector<double> user)
{
	vector<int> knownIndexes;

	for(unsigned int j=0; j<user.size(); j++)
	{
		if(user[j] != INT_MAX)
			knownIndexes.push_back(j);
	}

	if (knownIndexes.size() == 1)
		return -1;

	int randomIndex = rand() % knownIndexes.size();

	return knownIndexes[randomIndex];
}

/*=== 2 ===*/
