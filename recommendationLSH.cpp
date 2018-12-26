#include <iostream>
#include <getopt.h>
#include <map>
#include <fstream>
#include <sstream>
#include <climits>
#include <cmath>

#include "utilities.h"
#include "help_functions.h"
#include "recommendationLSH.h"

#define MAX_NEIGHBOURS  	 20
#define RECOMMENDATION_USERS 5
#define CRYPTO_NUMBER 		 100

using namespace std;

HashTable<vector<double>> ** createAndFillHashTable(vector<vector<double>> users, char ** argv, short int inputFileIndex, int k, int L)
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
			hash_tableptr[j]->put(users.at(i), to_string(i));
	}

	infile.close();

	return hash_tableptr;
}

void recommendationLSH(HashTable<vector<double>> ** hash_tableptr, vector<vector<double>> users, map<int, string> cryptos, int L, string outputfile)
{
	/*== find closest neighbours*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== if user is zero vector recommend 5 cryptos*/
		if( vectorIsZero(users.at(i)) )
		{
			/*== print cryptos*/
			vector<int> recommendation_ids(RECOMMENDATION_USERS);
			for(unsigned int j=0; j<RECOMMENDATION_USERS; j++)
				recommendation_ids.at(j) = j;

			printRecommendation(cryptos, recommendation_ids, i, outputfile);

			continue;
		}

		/*== find neighbours for every hash table*/
		vector<vector<int>> all_neighbours(L);
		for(int j=0; j<L; j++)
			all_neighbours.at(j) = hash_tableptr[j]->recommendationANN(users.at(i), MAX_NEIGHBOURS);

		/*== group neighbours into one vector with no duplicates*/
		vector<int> neighbours_unique = groupNeighboursFromTables(all_neighbours, L);

		/*== get the closest MAX_NEIGHBOURS*/
		vector<double> neighbours_distance(neighbours_unique.size());
		for(unsigned int j=0; j<neighbours_unique.size(); j++)
			neighbours_distance.at(j) = help_functions::cosine_distance(users.at(i), users.at(neighbours_unique.at(j)));

		vector<int> neighbours;
		vector<double> similarity;
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
		
		/*== estimate the unknown cryptos, using neighbourhood similarity*/
		predictUnknownCryptosPreferences(users, similarity, neighbours, i);

		/*== get the 5 best results*/
		vector<int> recommendation_ids = cryptosRecommendedByNeighbourhood(users.at(i));

		/*== print cryptos*/
		printRecommendation(cryptos, recommendation_ids, i, outputfile);
	}
}

vector<int> groupNeighboursFromTables(vector<vector<int>> all_neighbours, int L)
{
	vector<int> neighbours_unique;
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
				neighbours_unique.push_back(all_neighbours.at(j).at(z));
		}
	}

	return neighbours_unique;
}

void predictUnknownCryptosPreferences(vector<vector<double>> &users, vector<double> similarity, vector<int> neighbours, unsigned int i)
{
	/*== calculate normalisation factor z*/
	double zeta;
	for(unsigned int j=0; j<similarity.size(); j++)
		zeta += abs(similarity.at(j));

	zeta = 1/zeta;

	/*== prediction*/ 
	for(unsigned int j=0; j<users.at(i).size(); j++)
	{
		if(users.at(i).at(j) != 0)
			continue;

		double similarity_factor=0;
		for(unsigned int z=0; z<neighbours.size(); z++)
			similarity_factor += similarity.at(z) * users.at(neighbours.at(z)).at(j);

		users.at(i).at(j) = zeta * similarity_factor;
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
