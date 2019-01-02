#include <vector>
#include <climits>

#include "clustering_utilities.h"

#define RECOMMENDATION_USERS 5

using namespace std;

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

Metric<double>* getMetric(std::string type)
{
	Metric<double> * metric_ptr;

	if(type == "cosine")
		metric_ptr = new Metric_COS<double>();
	else if(type == "euclidean")
		metric_ptr = new Metric_EUC<double>();

	return metric_ptr;
}

int binarySearch(vector<double> arr, int a, int z, double x)
{
	int mid = a + (z - a)/2;

	/*== If the element is present at the middle itself*/
	if(z-a == 1)
		return z;

	/*== continue with left subarray*/
	if (arr[mid] > x)
		return binarySearch(arr, a, mid, x);

	/*== continue with right subarray*/
	return binarySearch(arr, mid, z, x);
}
