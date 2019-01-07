#include <iostream>
#include <getopt.h>
#include <map>
#include <fstream>
#include <sstream>
#include <climits>
#include <cmath>
#include <ctime>

#include "utilities.h"
#include "clustering_utilities.h"
#include "help_functions.h"
#include "recommendationLSH.h"
#include "validation.h"

using namespace std;

/*===== Create && Fill hash_tables*/
HashTable<vector<double>> ** createAndFillHashTable(vector<vector<double>> users, vector<vector<double>> normalisedUsers, string inputfile, int k, int L)
{
	ifstream infile;

	infile.open(inputfile);
	if(!infile.is_open())
	{
		cout << "Could not open input data file" << endl;
		exit(EXIT_FAILURE);
	}

	/*== find table size*/
	int tableSize = help_functions::calculate_tableSize(infile, "cosine", k);

	/*== find dimensions*/
	int dimensions = CRYPTO_NUMBER;

	/*== construct hash_table*/
	HashTable<vector<double>> ** hash_tableptr = new HashTable<vector<double>>*[L];

	for(int i=0; i<L; i++)
		hash_tableptr[i] = new HashTable_COS<vector<double>>(tableSize, k, dimensions);

	/*== fill the hash table with users vector*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== ignore users with 0 crypto references*/
		users[i] = eliminateUnknown(users[i]);
		if( vectorIsZero(users.at(i)) )
			continue;
		
		for(int j=0; j<L; j++)
			hash_tableptr[j]->put(normalisedUsers.at(i), users.at(i), to_string(i));
	}

	infile.close();

	return hash_tableptr;
}

HashTable<vector<double>> ** createAndFillHashTable(vector<vector<double>> users, vector<vector<double>> normalisedUsers, string inputfile, int k, int L, int start_index, int end_index)
{
	ifstream infile;

	infile.open(inputfile);
	if(!infile.is_open())
	{
		cout << "Could not open input data file" << endl;
		exit(EXIT_FAILURE);
	}

	/*== find table size*/
	int tableSize = help_functions::calculate_tableSize(infile, "cosine", k);

	/*== find dimensions*/
	int dimensions = CRYPTO_NUMBER;

	/*== construct hash_table*/
	HashTable<vector<double>> ** hash_tableptr = new HashTable<vector<double>>*[L];

	for(int i=0; i<L; i++)
		hash_tableptr[i] = new HashTable_COS<vector<double>>(tableSize, k, dimensions);

	/*== fill the hash table with users vector*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== we ignore these users since we have them as tests*/
		if( i >= start_index && i < end_index)
			continue;

		/*== ignore users with 0 crypto references*/
		users[i] = eliminateUnknown(users[i]);
		if( vectorIsZero(users.at(i)) )
			continue;
		
		for(int j=0; j<L; j++)
			hash_tableptr[j]->put(normalisedUsers.at(i), users.at(i), to_string(i));
	}

	infile.close();

	return hash_tableptr;
}

/*== Recommendation Functions*/
vector<double> LSH_calculateRatings(HashTable<vector<double>> ** hash_tableptr, vector<double> user, vector<double> normalisedUser, vector<vector<double>> normalisedUsers, int L, bool normalised)
{
	/*== find neighbours for every hash table*/
	vector<vector<int>> all_neighbours(L);
	vector<vector<double>> all_distances(L);
	for(int j=0; j<L; j++)
	{
		if(normalised)
			hash_tableptr[j]->recommendationANN_normalised(normalisedUser, MAX_NEIGHBOURS, all_neighbours.at(j), all_distances.at(j));
		else
			hash_tableptr[j]->recommendationANN(eliminateUnknown(user), MAX_NEIGHBOURS, all_neighbours.at(j), all_distances.at(j));
	}

	/*== find the neighbourhood*/
	vector<int> neighbours;
	vector<double> similarity;
	LSH_calculateNeighbourhood(all_neighbours, all_distances, neighbours, similarity, L);

	/*== in case no neighbours were found*/
	if(neighbours.size() == 0)
	{
		vector<double> zero_sized_user;
		return zero_sized_user;
	}	

	/*== estimate the unknown cryptos, using neighbourhood similarity*/
	LSH_predictUnknownCryptos(user, normalisedUser, normalisedUsers, similarity, neighbours);

	return user;
}

void LSH_calculateNeighbourhood(vector<vector<int>> all_neighbours, vector<vector<double>> all_distances, vector<int> &neighbours, vector<double> &similarity, int L)
{
	/*== group neighbours into one vector with no duplicates*/
	vector<int> neighbours_unique;
	vector<double> neighbours_distance; 
	LSH_groupNeighboursFromTables(all_neighbours, all_distances, neighbours_unique, neighbours_distance, L);

	/*== in case the neighbours we have found are less that MAX_NEIGHBOURS*/
	if(neighbours_unique.size() < MAX_NEIGHBOURS)
	{
		neighbours = neighbours_unique;
		similarity = neighbours_distance;
	}
	else
	{
		for(unsigned int j=0; j<MAX_NEIGHBOURS; j++)
		{
			double min_distance = INT_MAX;
			int index;
			for(unsigned int z=0; z<neighbours_distance.size(); z++)
			{
				if(min_distance > neighbours_distance.at(z))
				{
					min_distance = neighbours_distance.at(z);
					index = z;
				}
			}

			neighbours.push_back(neighbours_unique.at(index));
			similarity.push_back(neighbours_distance.at(index));
			neighbours_distance.at(index) = INT_MAX;
		}
	}

}

void LSH_groupNeighboursFromTables(vector<vector<int>> all_neighbours, vector<vector<double>> all_distances, vector<int> &neighbours_unique, vector<double> &neighbours_distance, int L)
{
	for(int j=0; j<L; j++)
	{
		for(unsigned z=0; z<all_neighbours.at(j).size(); z++)
		{
			bool flag=false;
			for(unsigned k=0; k<neighbours_unique.size(); k++)
			{
				if(all_neighbours.at(j).at(z) == neighbours_unique.at(k))
					flag=true;
			}

			if(!flag)
			{
				neighbours_unique.push_back(all_neighbours.at(j).at(z));
				neighbours_distance.push_back(all_distances.at(j).at(z));
			}
		}
	}
}

void LSH_predictUnknownCryptos(vector<double> &user, vector<double> normalisedUser, vector<vector<double>> users, vector<double> similarity, vector<int> neighbours)
{
	/*== find the cryptos the user has referenced*/
	vector<int> cryptos_referenced(user.size());

	for(unsigned int i=0; i<user.size(); i++)
	{
		if(user.at(i) != INT_MAX)
		{
			user.at(i) = INT_MIN;
			cryptos_referenced.at(i) = 1;
		}
	}

	/*== turn cosine distances gathered from LSH, to cosine similarities*/
	for (unsigned int i=0; i<similarity.size(); i++)
		similarity[i] = 1 - similarity[i]; 

	/*== normalise user*/
	user = normalisedUser;

	/*== calculate normalisation factor z, using cosine similarity*/
	double zeta=0;
	for(unsigned int j=0; j<similarity.size(); j++)
		zeta += abs(similarity.at(j));

	zeta = 1/(zeta + 0.1);

	/*== prediction*/ 
	for(unsigned int j=0; j<user.size(); j++)
	{
		/*== If a user has mentioned a crypto, we dont want to recommend it by accident.
			 We make it equal to INT_MIN so when searching for the max sentiment score 
			 on the vector we ll have no chance to pick it
		==*/
		if(cryptos_referenced.at(j) == 1)
			continue;

		double similarity_factor=0;
		for(unsigned int z=0; z<neighbours.size(); z++)
			similarity_factor += similarity.at(z) * users.at(neighbours.at(z)).at(j);

		user.at(j) = zeta * similarity_factor;
	}
}

void unallocateHashTable(HashTable<vector<double>> ** hash_tableptr, int L)
{
	for(int i=0; i<L; i++)
	{
		delete hash_tableptr[i];
		hash_tableptr[i] = NULL;
	}
	delete[] hash_tableptr;
	hash_tableptr = NULL;
}

/*=== 1 ===*/
void recommendationLSH(vector<vector<double>> users, vector<vector<double>> normalisedUsers, map<int, string> cryptosIndex, int k, int L, string inputfile, string outputfile) 
{
	/*== create and fill hash table*/
	HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(users, normalisedUsers, inputfile, k, L);

	printRecommendationTitle(outputfile, "Cosine LSH");
	time_t start_time = clock();

	/*== start recommendation for every user*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		bool normalised = true;
		vector<double> normalisedUser = normalisedUsers.at(i);
		if (!recommendationEligibility(normalised, normalisedUser, users.at(i)))
		{
			printUnmatched(cryptosIndex, i, outputfile);
			continue;
		}

		/*== get the user ratings*/
		vector<double> average_user = LSH_calculateRatings(hash_tableptr, users.at(i), normalisedUser, normalisedUsers, L, normalised);

		/*== if the user has no neighbours*/
		if(average_user.size() == 0)
		{
			printUnmatched(cryptosIndex, i, outputfile);
			continue;
		}
	
		/*== return the 5 best results*/
		vector<int> recommendations =  cryptosRecommendedByNeighbourhood(average_user);
		if(recommendations.size() == 0)
			printUnmatched(cryptosIndex, i, outputfile);
		else
			printRecommendation(cryptosIndex, recommendations, i, outputfile, RECOMMENDATION_A);
	}

	printRecommendationTimer(outputfile, (double)(clock() - start_time)/CLOCKS_PER_SEC);

	/*== free hash tables*/
	unallocateHashTable(hash_tableptr, L);
}

/*=== 2 ===*/
void recommendationLSH(vector<vector<double>> users, vector<vector<double>> normalisedUsers, vector<vector<double>> virtual_users, vector<vector<double>> normalised_virtual_users, map<int, string> cryptosIndex, int k, int L, string inputfile, string outputfile) 
{
	/*== create and fill hash table with virtual users*/
	HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(virtual_users, normalised_virtual_users, inputfile, k, L);

	printRecommendationTitle(outputfile, "Cosine LSH");
	time_t start_time = clock();

	/*== start recommendation for every user*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		bool normalised = true;
		vector<double> normalisedUser = normalisedUsers.at(i);
		if ( !recommendationEligibility(normalised, normalisedUser, users.at(i)))
		{
			printUnmatched(cryptosIndex, i, outputfile);
			continue;
		}

		/*== get the user ratings*/
		vector<double> average_user = LSH_calculateRatings(hash_tableptr, users.at(i), normalisedUser, normalisedUsers, L, normalised);

		/*== if the user has no neighbours*/
		if(average_user.size() == 0)
		{
			printUnmatched(cryptosIndex, i, outputfile);
			continue;
		}

		/*== return the 5 best results*/
		vector<int> recommendations =  cryptosRecommendedByNeighbourhood(average_user);

		/*== print the recommended results*/
		if(recommendations.size() == 0)
			printUnmatched(cryptosIndex, i, outputfile);
		else
			printRecommendation(cryptosIndex, recommendations, i, outputfile, RECOMMENDATION_B);
	}

	printRecommendationTimer(outputfile, (double)(clock() - start_time)/CLOCKS_PER_SEC);

	/*== free hash tables*/
	unallocateHashTable(hash_tableptr, L);
}
