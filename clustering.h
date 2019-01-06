#ifndef __CLUSTERING_H__
#define __CLUSTERING_H__

#include "metric.h"

#include <vector>

std::vector<int> k_meanspp(std::vector<std::vector<double>> users, std::vector<std::vector<double>> prenormalised_users, int clusters);

std::vector<std::vector<double>> initialisation(std::vector<std::vector<double>> data, int data_size, int clusters, std::vector<int> labels);

std::vector<int> assignment(std::vector<std::vector<double>> data, std::vector<std::vector<double>> centroids, int data_size, Metric<double>* metric_ptr, std::vector<int> labels);

std::vector<std::vector<double>> update(std::vector<std::vector<double>> data, std::vector<int> labels, std::vector<std::vector<double>> centroids, long double &objective_function, Metric<double>* metric_ptr);

#endif
