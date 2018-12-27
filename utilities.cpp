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
#define ALPHA				 15
#define MAX_NEIGHBOURS  	 20
#define RECOMMENDATION_USERS 5
#define CRYPTO_NUMBER 		 100

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

vector<vector<double>> createUserVector(string inputfile, map<string, float> vaderLexicon, map<string, int> cryptos)
{
	ifstream infile;

	infile.open(inputfile);
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

		temp_user = user;

		/*== eval each word and get the sentimental score using vaderLexicon map*/
		double sentiment_score=0.0;
		bool wordFound = false;
		vector<int> cryptoReferenced;
		while (getline(iss, word, '\t'))
		{
			try {
				sentiment_score += vaderLexicon.at(word);
				wordFound = true;
			}catch (const std::exception& e) {
				try{
					cryptoReferenced.push_back( cryptos.at(word) );
				}catch (const std::exception& e){
					continue;
				}
			}
		}

		if(wordFound == false)
			continue;

		/*== sentiment score normalisation*/
		sentiment_score = sentiment_score/sqrt(sentiment_score*sentiment_score + ALPHA);

		/*== update the user vector depending on the cryptos he referenced in his tweets*/
		for(unsigned int i=0; i<cryptoReferenced.size(); i++)
		{
			if(users.at(index).at(cryptoReferenced.at(i)) == INT_MAX)
				users.at(index).at(cryptoReferenced.at(i)) = 0;

			users.at(index).at(cryptoReferenced.at(i)) += sentiment_score;
		}
	}

	infile.close();	

	return users;
}

vector<double> single_normalisation(vector<double> user)
{
	/*== find R(u)*/
	double average = 0.0;
	int average_counter = 0;
	for(unsigned int j=0; j<CRYPTO_NUMBER; j++)
	{
		if(user.at(j) == INT_MAX)
			continue;

		average += user.at(j);
		average_counter++;
	}

	average /= average_counter;

	/*== turn INT_MAXs to 0s and subtract from the other numbers the average*/
	for(unsigned int j=0; j<CRYPTO_NUMBER; j++)
	{
		if(user.at(j) == INT_MAX)
		{
			user.at(j) = 0;
			continue;
		}

		user.at(j) -= average;
	}

	return user;
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

void printRecommendation(map<int, string> cryptos, vector<int> recommendations, int user, string outputfile)
{
	ofstream outfile;

	outfile.open(outputfile, ofstream::app);
	if(!outfile.is_open())
	{
		cout << "Could not open output file" << endl;
		exit(EXIT_FAILURE);
	}

	outfile << "<u" << user+1 << ">";
	for (unsigned int j=0; j<RECOMMENDATION_USERS; j++)
		outfile << cryptos.at(recommendations.at(j)) << " ";

	outfile << endl;

	outfile.close();
}

void printRecommendationTitle(string outputfile, string title)
{
	ofstream outfile;

	outfile.open(outputfile, ofstream::app);
	if(!outfile.is_open())
	{
		cout << "Could not open output file" << endl;
		exit(EXIT_FAILURE);
	}

	outfile << title << endl;

	outfile.close();
	
}

void printRecommendationTimer(string outputfile, double time)
{
	ofstream outfile;

	outfile.open(outputfile, ofstream::app);
	if(!outfile.is_open())
	{
		cout << "Could not open output file" << endl;
		exit(EXIT_FAILURE);
	}

	outfile << "Execution time: " << time << endl << endl;

	outfile.close();
}

void resetOutput(string outputfile)
{
	
	/*== open outputfile for writing*/
	ofstream outfile;

	outfile.open(outputfile, ofstream::out | ofstream::trunc);
	if(!outfile.is_open())
	{
		cout << "Could not open output file" << endl;
		exit(EXIT_FAILURE);
	}

	outfile.close();
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

vector<double> eliminateUnknown(vector<double> user)
{
	for(unsigned int i=0; i<100; i++)
	{
		if(user.at(i) == INT_MAX)
			user.at(i) = 0;
	}

	return user;
}
