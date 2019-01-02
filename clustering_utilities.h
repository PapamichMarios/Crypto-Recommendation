#ifndef __CLUSTERING_UTILITIES_H__
#define __CLUSTERING_UTILITIES_H__

#include <vector>
#include <string>

#include "metric.h"

std::vector<int> cryptosRecommendedByNeighbourhood(std::vector<double> user);


Metric<double>* getMetric(std::string type);

int binarySearch(std::vector<double> arr, int a, int z, double x);

#endif
