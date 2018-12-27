#include <iostream>
#include <getopt.h>
#include <map>
#include <fstream>
#include <sstream>
#include <climits>
#include <cmath>
#include <ctime>

#include "utilities.h"
#include "help_functions.h"
#include "recommendationLSH.h"

#define MAX_NEIGHBOURS  	 20
#define RECOMMENDATION_USERS 5
#define CRYPTO_NUMBER 		 100

using namespace std;

void recommendationLSHProcess(vector<vector<double>> users, vector<vector<double>> normalisedUsers, map<int, string> cryptosIndex, int k, int L, char ** argv, short int inputFileIndex, short int outputFileIndex) 
{
	/*== create and fill hash table*/
	HashTable<vector<double>> ** hash_tableptr = createAndFillHashTable(users, normalisedUsers, argv, inputFileIndex, k, L);

	printRecommendationTitle(argv[outputFileIndex], "Cosine LSH");
	time_t start_time = clock();

	/*== if a vector after normalisation is a 0 vector, we have to find its neighbours before normalisation*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		bool normalised = true;
		vector<double> user = single_normalisation(users.at(i));
		vector<double> normalisedUser;
		normalisedUser = user;

		if(vectorIsZero(user))
		{
			normalised = false;
			user = eliminateUnknown(users.at(i));

			/*== if a user doesn't mention any bitcoin, we propose the first 5 bitcoins*/
			if(vectorIsZero(user))
			{
				vector<int> recommendation_ids(RECOMMENDATION_USERS);
				for(unsigned int j=0; j<recommendation_ids.size(); j++)
					recommendation_ids.at(j) = j;

				printRecommendation(cryptosIndex, recommendation_ids, i, argv[outputFileIndex]);

				continue;
			}
		}

		recommendationLSH(hash_tableptr, user, normalisedUser, i, users, cryptosIndex, L, argv[outputFileIndex], normalised);
	}

	printRecommendationTimer(argv[outputFileIndex], (double)(clock() - start_time)/CLOCKS_PER_SEC);
}

/*===== Create && Fill hash_tables*/
HashTable<vector<double>> ** createAndFillHashTable(vector<vector<double>> users, vector<vector<double>> normalisedUsers, char ** argv, short int inputFileIndex, int k, int L)
{
	ifstream infile;

	infile.open(argv[inputFileIndex]);
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
		for(int j=0; j<L; j++)
			hash_tableptr[j]->put(normalisedUsers.at(i), users.at(i), to_string(i));
	}

	infile.close();

	return hash_tableptr;
}

/*== Recommendation Functions*/
void recommendationLSH(HashTable<vector<double>> ** hash_tableptr, vector<double> user, vector<double> normalisedUser, int userIndex, vector<vector<double>> users, map<int, string> cryptos, int L, string outputfile, bool normalised)
{
	/*== find neighbours for every hash table*/
	vector<vector<int>> all_neighbours(L);
	vector<vector<double>> all_distances(L);
	for(int j=0; j<L; j++)
	{
		if(normalised)
			hash_tableptr[j]->recommendationANN(user, MAX_NEIGHBOURS, all_neighbours.at(j), all_distances.at(j));
		else
			hash_tableptr[j]->recommendationANN_normalised(user, MAX_NEIGHBOURS, all_neighbours.at(j), all_distances.at(j));
	}

	/*== find the neighbourhood*/
	vector<int> neighbours;
	vector<double> similarity;
	calculateNeighbourhood(all_neighbours, all_distances, neighbours, similarity, L);

	/*== estimate the unknown cryptos, using neighbourhood similarity*/
	if(normalised)
		predictUnknownCryptos_normalised(user, users, similarity, neighbours);
	else
		predictUnknownCryptos(user, normalisedUser, users, similarity, neighbours);

	/*== get the 5 best results*/
	vector<int> recommendation_ids = cryptosRecommendedByNeighbourhood(user);

	/*== print cryptos*/
	printRecommendation(cryptos, recommendation_ids, userIndex, outputfile);
}

void calculateNeighbourhood(vector<vector<int>> all_neighbours, vector<vector<double>> all_distances, vector<int> &neighbours, vector<double> &similarity, int L)
{
	/*== group neighbours into one vector with no duplicates*/
	vector<int> neighbours_unique;
	vector<double> neighbours_distance; 
	groupNeighboursFromTables(all_neighbours, all_distances, neighbours_unique, neighbours_distance, L);

	/*== in case the neighbours we have found are less that MAX_NEIGHBOURS*/
	if(neighbours_distance.size() < MAX_NEIGHBOURS)
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

void groupNeighboursFromTables(vector<vector<int>> all_neighbours, vector<vector<double>> all_distances, vector<int> &neighbours_unique, vector<double> &neighbours_distance, int L)
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

void predictUnknownCryptos(vector<double> &user, vector<double> normalisedUser, vector<vector<double>> users, vector<double> similarity, vector<int> neighbours)
{
	/*== find the cryptos the user has referenced*/
	vector<int> cryptos_referenced(user.size());

	for(unsigned int i=0; i<user.size(); i++)
	{
		if(user.at(i) != INT_MAX)
			cryptos_referenced.at(i) = 1;
	}

	/*== normalise user*/
	user = normalisedUser;

	/*== calculate normalisation factor z*/
	double zeta;
	for(unsigned int j=0; j<similarity.size(); j++)
		zeta += abs(similarity.at(j));

	zeta = 1/zeta;

	/*== prediction*/ 
	for(unsigned int j=0; j<user.size(); j++)
	{
		if(cryptos_referenced.at(j) != 1)
			continue;

		double similarity_factor=0;
		for(unsigned int z=0; z<neighbours.size(); z++)
			similarity_factor += similarity.at(z) * users.at(neighbours.at(z)).at(j);

		user.at(j) = zeta * similarity_factor;
	}
}

void predictUnknownCryptos_normalised(vector<double> &user, vector<vector<double>> users, vector<double> similarity, vector<int> neighbours)
{
	/*== calculate normalisation factor z*/
	double zeta;
	for(unsigned int j=0; j<similarity.size(); j++)
		zeta += abs(similarity.at(j));

	zeta = 1/zeta;

	/*== prediction*/ 
	for(unsigned int j=0; j<user.size(); j++)
	{
		if(user.at(j) != 0)
			continue;

		double similarity_factor=0;
		for(unsigned int z=0; z<neighbours.size(); z++)
			similarity_factor += similarity.at(z) * users.at(neighbours.at(z)).at(j);

		user.at(j) = zeta * similarity_factor;
	}
}

vector<int> cryptosRecommendedByNeighbourhood(vector<double> user)
{
	vector<int> recommendation_ids;
	for(unsigned int j=0; j<RECOMMENDATION_USERS; j++)
	{
		double max_distance = INT_MIN;
		int index;
		for(unsigned int z=0; z<user.size(); z++)
		{
			if(max_distance < user.at(z))
			{
				max_distance = user.at(z);
				index = z;
			}
		}

		recommendation_ids.push_back(index);
		user.at(index) = INT_MIN;
	}

	return recommendation_ids;
}


