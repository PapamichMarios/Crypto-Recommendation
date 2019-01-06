#include <climits>
#include <iostream>

#include "validation.h"
#include "recommendationLSH.h"
#include "recommendationClustering.h"
#include "utilities.h"
#include "hash_table.h"
#include "clustering.h"

#define FOLDS 10

using namespace std;

/*=== 1 ===*/
void F_FoldCrossValidation(int k, int L, vector<vector<double>> users, vector<vector<double>> normalisedUsers, string inputfile, string outputfile)
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

		/*== create hash_tables based on the training set*/
		HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(users, normalisedUsers, inputfile, k, L, i*partition_size, (i+1)*partition_size);

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
			MAE += abs(sentiment - predicted_sentiment);

			if( i == FOLDS - 1 && oddFlag && j == partition_size-1)
			{
				j--;
				plus_one++;
			}
		}

		average_MAE += MAE/(partition_size-excluded);

		/*== clear hash tables*/
		unallocateHashTable(hash_tableptr, L);
	}

	printRecommendationMAE(outputfile, "LSH Recommendation MAE", average_MAE/FOLDS);
}

void F_FoldCrossValidation(vector<vector<double>> users, vector<vector<double>> normalisedUsers, string outputfile)
{
	int data_size = users.size();
	
	bool oddFlag = false;
	if(data_size % 2 == 1)
		oddFlag = true;

	int partition_size = data_size/FOLDS;
	double average_MAE=0;
	
	for(int i=0; i<FOLDS; i++)
	{
		cout << i << endl;
		double MAE=0;
		int excluded=0;
		int plus_one=0;
		int plus_one2=0;

		vector<vector<double>> normalisedUsers_temp = normalisedUsers;
		vector<vector<double>> users_temp = users;

		/*== keep the indexes we turned from known to unknown on the training set*/
		vector<int> testing_indexes(partition_size, -1);

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

			testing_indexes[j] = unknownIndex;

			normalisedUsers_temp[index][unknownIndex] = 0;
			users_temp[index][unknownIndex] = INT_MAX;

		}
		
		/*== cluster the training users with the testing users*/
		vector<int> labels = k_meanspp(normalisedUsers_temp, 50);

		for(unsigned int j=0; j<testing_indexes.size(); j++)
			cout << testing_indexes[j] << ", ";
		cout<<endl;
		/*== get one part of the users set*/
		for(int j=0; j<partition_size; j++)
		{
			int index = i*partition_size + j + plus_one2;

			if(testing_indexes[j] == -1)
				continue;

			bool normalised = true;
			if (vectorIsZero(normalisedUsers_temp[index]))
				normalised = false;
			
			/*== calculate the ratings of the user based on the clusters*/
			vector<double> average_user = Clustering_calculateRatings(users_temp[index], normalisedUsers_temp[index], index, users_temp, normalisedUsers_temp, labels, normalised);

			/*== compare the crypto we assigned as unkown to its real value*/
			MAE += abs(normalisedUsers[index][testing_indexes[j]] - average_user[testing_indexes[j]]);
			if( i == FOLDS -1 )
				cout << MAE << " ";

		}

		cout << endl;
		cout << "excluded: " << excluded << endl;
		average_MAE += MAE/(partition_size-excluded);
		cout << "MAE: " << average_MAE << endl;
	}

	printRecommendationMAE(outputfile, "Clustering Recommendation MAE", average_MAE/FOLDS);
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
void virtualValidation(vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<vector<double>> virtual_users, vector<vector<double>> normalised_virtual_users, int k, int L, string inputfile, string outputfile)
{ 
	/*== create hash_tables based on virtual users*/
	HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(virtual_users, normalised_virtual_users, inputfile, k, L);

	int excluded = 0;
	double MAE=0;
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== check wether we need to ignore the vector*/
		bool normalised = true;
		if (!recommendationEligibility(normalised, normalisedUsers[i], users[i]) )
		{
			excluded++;
			continue;
		}

		/*== assign one random mentioned crypto to unknown*/
		int unknownIndex = assignUnknown(users[i]);
		if(unknownIndex == -1)
		{
			excluded++;
			continue;
		}

		/*== keep the sentiment*/
		double sentiment = normalisedUsers[i][unknownIndex];
		normalisedUsers[i][unknownIndex] = 0;
		users[i][unknownIndex] = INT_MAX;

		/*== get the user ratings*/
		vector<double> average_user = LSH_calculateRatings(hash_tableptr, users[i], normalisedUsers[i], normalisedUsers, L, normalised);

		/*== compare the crypto we assigned as unkown to its real value*/
		double predicted_sentiment = average_user[unknownIndex];
		MAE += abs(sentiment - predicted_sentiment);
	}

	MAE /= (users.size() - excluded);

	printRecommendationMAE(outputfile, "LSH Recommendation MAE", MAE);

	/*== unallocate hash table*/
	unallocateHashTable(hash_tableptr, L);
}

void virtualValidation(vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<vector<double>> virtual_users, vector<vector<double>> normalised_virtual_users, string outfile)
{
	int excluded = 0;
	double MAE=0;
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== check wether we need to ignore the vector*/
		bool normalised = true;
		if (!recommendationEligibility(normalised, normalisedUsers[i], users[i]) )
		{
			excluded++;
			continue;
		}

		/*== assign one random mentioned crypto to unknown*/
		int unknownIndex = assignUnknown(users[i]);
		if(unknownIndex == -1)
		{
			excluded++;
			continue;
		}

		/*== keep the sentiment*/
		double sentiment = normalisedUsers[i][unknownIndex];
		normalisedUsers[i][unknownIndex] = 0;
		users[i][unknownIndex] = INT_MAX;

		/*== create a vector with virtual users + the user we want to recommend*/
		vector<vector<double>> all_users;
		all_users = normalised_virtual_users;
		all_users.push_back(normalisedUsers[i]);

		vector<int> labels = k_meanspp(all_users, 5);

		/*== get the user ratings*/
		vector<double> average_user = Clustering_calculateRatings(users[i], normalisedUsers[i], labels.size()-1, virtual_users, normalised_virtual_users, labels, normalised);

		/*== compare the crypto we assigned as unkown to its real value*/
		double predicted_sentiment = average_user[unknownIndex];
		MAE += abs(sentiment - predicted_sentiment);
	}

	MAE /= (users.size() - excluded);

	printRecommendationMAE(outfile, "Clustering Recommendation MAE", MAE);
}
