#include <gtest/gtest.h>

#include "../clustering.cpp"

using namespace std;

TEST(ClusteringTest, ValuesReturned)
{
	/*== create users vector*/
	map<string, float> vaderLexicon = vaderLexiconMap();
	map<string, int> cryptos = cryptosMap();

	string input = "../../dataset/tweets_dataset_small.csv";
	vector<vector<double>> users = createUserVector(input, vaderLexicon, cryptos);
	vector<vector<double>> normalised_users = users;
	normalisation(normalised_users);

	int clusters = 5;
	int data_size = users.size();
	vector<int> labels(data_size);

	/*== initialisation*/
	vector<vector<double>> centroids = initialisation(normalised_users, data_size, clusters, labels);
	ASSERT_EQ(centroids.size(), clusters); 

	/*== assignment*/
	Metric<double> * metric_ptr = getMetric("euclidean");
	labels = assignment(normalised_users, centroids, data_size, metric_ptr, labels);
	ASSERT_EQ(labels.size(), data_size);

	/*== update*/
	long double objective_function;
	centroids = update(normalised_users, labels, centroids, objective_function, metric_ptr);
	ASSERT_EQ(centroids.size(), clusters);
}
