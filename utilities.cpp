#include <iostream>
#include <getopt.h>
#include <map>
#include <fstream>
#include <sstream>

#include "utilities.h"

#define VADER_LEXICON "./vader_lexicon.csv"
#define CRYPTOS	"coins_queries.csv"

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
	map<string, float> vaderLexicon;
    ifstream infile;

    infile.open(VADER_LEXICON);
    if(!infile.is_open())
    {
        cout << "Could not open input data file" << endl;
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
	map<string, int> cryptos;
    ifstream infile;
	int i=0;

    infile.open(CRYPTOS);
    if(!infile.is_open())
    {
        cout << "Could not open input data file" << endl;
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
