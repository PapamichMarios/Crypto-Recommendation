#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>

#include "utilities.h"
#include "recommendationLSH.h"
#include "recommendationClustering.h"
#include "hash_table.h"
#include "clustering.h"

#define CLUSTER_TWEETS 50
#define ARGS 7

using namespace std;

int main(int argc, char** argv)
{
	short int inputFileIndex, outputFileIndex, tweetsFileIndex;
	bool validateFlag = false;
	int k = 4;
	int L = 3;

	/*== check the #args*/	
	rerunCheck(argc, ARGS);

	/*== get inline arguments*/
	getInlineArguments(argc, argv, inputFileIndex, outputFileIndex, validateFlag, tweetsFileIndex);

	/*== clear outputfile */
	resetOutput(argv[outputFileIndex]);

	/*== create a map with the sentimental value of every word*/
	map<string, float> vaderLexicon = vaderLexiconMap();
	
	/*== create a map crypto : index*/
	map<string, int> cryptos = cryptosMap();
	
	/*== create a map index : crypto*/
	map<int, string> cryptosIndex = cryptosIndexMap();

	/*== construct vector "user : crypto"*/
	vector<vector<double>> users = createUserVector(argv[inputFileIndex], vaderLexicon, cryptos);

	/*== normalize user vectors*/
	vector<vector<double>> normalisedUsers = users;
	normalisation(normalisedUsers);

	/*==== 1. Recommend cryptos based on user clustering*/
	recommendationLSH(users, normalisedUsers, cryptosIndex, k, L, argv[inputFileIndex], argv[outputFileIndex]);
	recommendationClustering(users, normalisedUsers, cryptosIndex, argv[outputFileIndex]);
	/*====*/

	/*== construct map "twitterid : vector<double> crypto sentiments"*/
	map<int, vector<double>> tweets_sentiments = createTweetMap(argv[inputFileIndex], vaderLexicon, cryptos);
	map<int, vector<double>> tweets_sentiments_normalised = tweets_sentiments;
	normalisation(tweets_sentiments_normalised);
	
	/*== construct vector with the tweets from the previous assignment*/
	vector<vector<double>> tweets = createTweetVector(argv[tweetsFileIndex]);

	/*== cluster tweets*/
	vector<int> tweets_clusters = k_meanspp(tweets, CLUSTER_TWEETS);

	/*== find virtual user for every cluster*/
	vector<vector<double>> virtual_users = createVirtualUsers(tweets_sentiments, tweets_clusters, CLUSTER_TWEETS, 100);
	vector<vector<double>> normalised_virtual_users = createVirtualUsersNormalised(tweets_sentiments_normalised, tweets_clusters, CLUSTER_TWEETS, 100);

	/*==== 2. Recommend cryptos based on twitter clustering*/
	recommendationLSH(users, normalisedUsers, virtual_users, normalised_virtual_users, cryptosIndex, k, L, argv[inputFileIndex], argv[outputFileIndex]);
	recommendationClustering(users, normalisedUsers, virtual_users, normalised_virtual_users, cryptosIndex, argv[outputFileIndex]);
	/*===*/

	exit(EXIT_SUCCESS);	
}
