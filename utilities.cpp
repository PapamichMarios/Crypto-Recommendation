#include <iostream>
#include <getopt.h>
#include <map>
#include <fstream>
#include <sstream>
#include <climits>
#include <cmath>

#include "g.h"
#include "utilities.h"
#include "help_functions.h"

#define VADER_LEXICON 		 "./vader_lexicon.csv"
#define CRYPTOS				 "coins_queries.csv"
#define CRYPTO_NUMBER 		 100
#define ALPHA				 15
#define MAX_NEIGHBOURS  	 20
#define RECOMMENDATION_USERS 5

using namespace std;

void rerunCheck(int argc, int args)
{
	if( argc != args && argc != args + 1)
	{
		cout << "Rerun: ./recommendation -d <input_file> -o <output_file> -validate" << endl;
		exit(EXIT_FAILURE);
	}
}

void getInlineArguments(int argc, char** argv, short int &inputFileIndex, short int &outputFileIndex, bool &validateFlag)
{
	int opt;

	/*== struct for getopt long options*/
	static struct option long_options[] = 
	{
		{"d"	 	, required_argument, NULL, 'd'},
		{"o"	 	, required_argument, NULL, 'o'},
		{"validate" , no_argument, NULL, 'v'},
		{0, 0, 0, 0}
	};

	while ((opt = getopt_long_only(argc, argv, "", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'd':
				inputFileIndex = optind -1;
                break;
            case 'o':
                outputFileIndex = optind-1;
                break;
			case 'v':
				validateFlag = true;
				break;
            case '?':
                exit(EXIT_FAILURE);
        }
    }
}

map<string, float> vaderLexiconMap()
{
	/*== map for vader lexicon
		 word : sentimental_value
	  ==*/

	map<string, float> vaderLexicon;
    ifstream infile;

    infile.open(VADER_LEXICON);
    if(!infile.is_open())
    {
        cout << "Could not open " << VADER_LEXICON  << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    string key;
	float value;

    while(getline(infile, line))
    {
        istringstream iss(line);

		iss >> key >> value;
		vaderLexicon.insert(pair<string, float>(key,value));
    }

    infile.close();

	return vaderLexicon;
}

map<string, int> cryptosMap()
{
	/*==map for cryptos
		crypto : index_of_crypto
	  ==*/

	map<string, int> cryptos;
    ifstream infile;
	int i=0;

    infile.open(CRYPTOS);
    if(!infile.is_open())
    {
        cout << "Could not open " << CRYPTOS  << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    string key;

    while(getline(infile, line))
    {
        istringstream iss(line);
		while(getline(iss, key, '\t'))
			cryptos.insert(pair<string, int>(key, i));

		i++;
    }

    infile.close();

	return cryptos;
}

map<int, string> cryptosIndexMap()
{	
	map<int, string> cryptos;
    ifstream infile;
	int i=0;

    infile.open(CRYPTOS);
    if(!infile.is_open())
    {
        cout << "Could not open " << CRYPTOS  << endl;
        exit(EXIT_FAILURE);
    }

    string line;
    string key;

    while(getline(infile, line))
    {
        istringstream iss(line);
		while(getline(iss, key, '\t'))
			cryptos.insert(pair<int, string>(i,key));

		i++;
    }

    infile.close();

	return cryptos;
}

vector<vector<double>> createUserVector(char ** argv, short int inputFileIndex, map<string, float> vaderLexicon, map<string, int> cryptos)
{
	ifstream infile;

	infile.open(argv[inputFileIndex]);
	if(!infile.is_open())
	{
		cout << "Could not open input data file" << endl;
		exit(EXIT_FAILURE);
	}

	/*== find how many different users we have*/
	string line, user, temp_user;
	int user_counter=0;
	while(getline(infile, line))
	{
		istringstream iss(line);
		getline(iss, user, '\t');

		if(user != temp_user)
			user_counter++;

		temp_user = user;
	}

	vector<vector<double>> users(user_counter, vector<double>(CRYPTO_NUMBER, INT_MAX));

	/*== find sentimental value for each user based on his tweets*/
	infile.clear();
	infile.seekg(0, ios::beg);

	int index=-1;
	string twitter_id, word;
	while(getline(infile, line))
	{
		istringstream iss(line);
		getline(iss, user, '\t');
		getline(iss, twitter_id, '\t');

		if(user != temp_user)
			index++;

		/*== eval each word and get the sentimental score using vaderLexicon map*/
		double sentiment_score=0.0;
		vector<int> cryptoReferenced;
		while (getline(iss, word, '\t'))
		{
			try {
				sentiment_score += vaderLexicon.at(word);
			}catch (const std::exception& e) {
				try{
					cryptoReferenced.push_back( cryptos.at(word) );
				}catch (const std::exception& e){
					continue;
				}
			}
		}

		/*== sentiment score normalisation*/
		sentiment_score = sentiment_score/sqrt(sentiment_score*sentiment_score + ALPHA);

		/*== update the user vector depending on the cryptos he referenced in his tweets*/
		for(unsigned int i=0; i<cryptoReferenced.size(); i++)
		{
			if(users.at(index).at(cryptoReferenced.at(i)) == INT_MAX)
				users.at(index).at(cryptoReferenced.at(i)) = 0;

			users.at(index).at(cryptoReferenced.at(i)) += sentiment_score;
		}

		temp_user = user;
	}

	infile.close();	

	return users;
}

void normalisation(vector<vector<double>> &users)
{
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== find R(u)*/
		double average = 0.0;
		int average_counter = 0;
		for(unsigned int j=0; j<CRYPTO_NUMBER; j++)
		{
			if(users.at(i).at(j) == INT_MAX)
				continue;

			average += users.at(i).at(j);
			average_counter++;
		}

		average /= average_counter;

		/*== turn INT_MAXs to 0s and subtract from the other numbers the average*/
		for(unsigned int j=0; j<CRYPTO_NUMBER; j++)
		{
			if(users.at(i).at(j) == INT_MAX)
			{
				users.at(i).at(j) = 0;
				continue;
			}

			users.at(i).at(j) -= average;
		}
	}
}

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

void recommendationLSH(HashTable<vector<double>> ** hash_tableptr, vector<vector<double>> users, map<int, string> cryptos, int L)
{
	/*== find closest neighbours*/
	for(unsigned int i=0; i<users.size(); i++)
	{
		/*== if user is zero vector recommend 5 cryptos*/
		if( vectorIsZero(users.at(i)) )
		{
			/*== print cryptos*/
			cout << "user" << i << ": ";
			for(int j=0; j<RECOMMENDATION_USERS; j++)
				cout << cryptos.at(j) << " " ;

			cout << endl;

			continue;	
		}

		/*== find neighbours for every hash table*/
		vector<vector<int>> all_neighbours(L);
		for(int j=0; j<L; j++)
			all_neighbours.at(j) = hash_tableptr[j]->recommendationANN(users.at(i), MAX_NEIGHBOURS);

		/*== group neighbours into one vector with no duplicates*/
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

				flag=false;
			}
		}

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

		/*== calculate normalisation factor z*/
		double zeta;
		for(unsigned int j=0; j<neighbours.size(); j++)
			zeta += abs(similarity.at(j));

		zeta = 1/zeta;

		
		/*== estimate the unknown cryptos, using neighbourhood similarity*/
		for(unsigned int j=0; j<users.at(i).size(); j++)
		{
			if(users.at(i).at(j) != 0)
				continue;

			double similarity_factor=0;
			for(unsigned int z=0; z<neighbours.size(); z++)
				similarity_factor += similarity.at(z) * users.at(neighbours.at(z)).at(j);

			users.at(i).at(j) = zeta * similarity_factor;
		}

		/*== get the 5 best results*/
		vector<int> recommendation_ids;
		for(unsigned int j=0; j<RECOMMENDATION_USERS; j++)
		{
			double max_distance = INT_MIN;
			int index;
			for(unsigned int z=0; z<users.at(i).size(); z++)
			{
				if(max_distance < users.at(i).at(z))
				{
					max_distance = users.at(i).at(z);
					index = z;
				}
			}

			recommendation_ids.push_back(index);
			users.at(i).at(index) = INT_MIN;
		}

		/*== print cryptos*/
		cout << "user" << i << ": ";
		for(unsigned int j=0; j<RECOMMENDATION_USERS; j++)
			cout << cryptos.at(recommendation_ids.at(j)) << " ";

		cout << endl;
	}
}

bool vectorIsZero(vector<double> v)
{
	double length=0;
	for(unsigned int i=0; i<v.size(); i++)
		length += v[i]*v[i];

	if(length)
		return false;

	return true;
}
