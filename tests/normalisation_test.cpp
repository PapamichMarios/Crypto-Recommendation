#include <gtest/gtest.h>

#include "../utilities.cpp"

using namespace std;

TEST(NormalisationTest, ValuesReturned)
{
	vector<vector<double>> user(1, vector<double>(100, INT_MAX));
	
	user[0][0] = 1;
	user[0][1] = -1;

	normalisation(user);
	
	int sum = user[0][0] + user[0][1];
	ASSERT_EQ(0, sum);
}
