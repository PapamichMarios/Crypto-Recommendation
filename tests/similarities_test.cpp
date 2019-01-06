#include <gtest/gtest.h>
#include <vector>

#include "../utilities.h"

using namespace std;

TEST(EuclideanSimilarityTest, Result)
{
	vector<double> u{1.0, 2.0};
	vector<double> v{1.0, 2.0};

	ASSERT_EQ(1, euclidean_similarity(u, v));
}

TEST(CosineSimilarityTest, Result)
{
	vector<double> u{1.0, 1.0};
	vector<double> v{1.0, 1.0};

	ASSERT_EQ(1, cosine_similarity(u, v));
}
