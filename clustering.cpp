#include <vector>
#include <climits>
#include <random>

#include "clustering.h"
#include "utilities.h"
#include "clustering_utilities.h"

#define MAX_PROCESS_LOOPS 15

using namespace std;

vector<int> k_meanspp(vector<vector<double>> users, vector<vector<double>> prenormalised_users, int clusters)
{
	vector<int> labels(users.size());
	vector<vector<double>> centroids;

	/*== create metric object, we use euclidean metric*/
	Metric<double> * metric_ptr = getMetric("euclidean");

	/*== assign -1 to users which dont have neighbours*/
	for(unsigned int i=0; i<prenormalised_users.size(); i++)
	{
		if(vectorIsZero(eliminateUnknown(prenormalised_users[i])))
			labels[i] = -1;
	}

	/*== initialisation*/
	centroids = initialisation(users, users.size(), clusters, labels);

	/*== assignment && update*/
	long double objective_function=0;
	long double last_objective_function =0;
	int loops=0;
	do
	{
		last_objective_function = objective_function;

		labels = assignment(users, centroids, users.size(), metric_ptr, labels);

		centroids = update(users, labels, centroids, objective_function, metric_ptr);
	
		loops++;

	} while( abs(objective_function - last_objective_function) > (double)1/100 && loops < MAX_PROCESS_LOOPS);

	/*== memory unallocation*/
	delete metric_ptr;

	return labels;
}

/*== random initialisation*/
vector<vector<double>> initialisation(vector<vector<double>> data, int data_size, int clusters, vector<int> labels)
{
	vector<vector<double>> centroids(clusters);
	int index;
	int centroids_assigned=0;
	for(int i=0; i<clusters; i++)
	{
		int flag=0;

		index = rand() % data_size;
		for(int j=0; j<centroids_assigned; j++)
		{
			if(centroids[j][0]-1 == index)
			{
				flag=1;
				break;
			}
		}

		if(flag || labels[index] == -1)
		{
			i-=1;
			continue;
		}

		centroids[i] = data[index];
		centroids_assigned++;
	}

	return centroids;
}

/*== loyds assignment*/
vector<int> assignment(vector<vector<double>> data, vector<vector<double>> centroids, int data_size, Metric<double>* metric_ptr, vector<int> labels)
{
	vector<int> cluster_assigned = labels;
	
	/*== calculate the distance of all the points with the centroids
		 assign them to the closest centroid
	  == */
	for(unsigned int i=0; i<data.size(); i++)
	{
		if(labels[i] == -1)
			continue;

		double min_distance = INT_MAX;
		int cluster;
		double temp_distance;
		for(unsigned int j=0; j<centroids.size(); j++)
		{
			temp_distance = metric_ptr->distance2(data[i], centroids[j]);

			if(temp_distance < min_distance)
			{
				min_distance = temp_distance;
				cluster = j;
			}
		}

		/*== assign point to cluster*/
		cluster_assigned[i] = cluster;
	}

	return cluster_assigned;
}

/*== k-means update*/
vector<vector<double>> update(vector<vector<double>> data, vector<int> labels, vector<vector<double>> centroids, long double &objective_function, Metric<double>* metric_ptr)
{
	vector<vector<double>> new_centroids(centroids.size(), vector<double>(centroids[0].size()));
	vector<int> cluster_size(centroids.size());

	/*== we count how many points each cluster has*/
	for(unsigned int i=0; i<labels.size(); i++)
	{
		if(labels[i] == -1)
			continue;

		cluster_size.at(labels[i])++;
	}

	/*== find the mean of each cluster*/
	for(unsigned int i=0; i<labels.size(); i++)
	{
		if(labels[i] == -1)
			continue;

		for(unsigned int j=0; j<data[i].size(); j++)
			new_centroids.at(labels[i]).at(j) += data.at(i).at(j)/cluster_size.at(labels[i]);
	}

	/*== calculate objective function*/
	objective_function = 0;

	for(unsigned int i=0; i<labels.size(); i++)
	{
		if(labels[i] == -1)
			continue;

		objective_function += pow(metric_ptr->distance2(data[i], new_centroids[labels[i]]),2);
	}

	return new_centroids;
}
