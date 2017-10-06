/******************************************************************************
* Author:	jaron.ho
* Date:		2016-04-10
* Brief:	calculate probability by weight list
******************************************************************************/
#ifndef _PROBABILITY_H_
#define _PROBABILITY_H_

#include <string>
#include <vector>

class Probability
{
public:
	Probability(const std::vector<std::vector<int>>& weightList);

public:
	int getValue(void);

	int getWeight(int value);

	void setWeight(int value, int weight);

	void reset(void);

private:
	void parseWeightList(void);

private:
	std::vector<std::vector<int>> mWeightListInit;
	std::vector<std::vector<int>> mWeightList;
	size_t mThreshold;
	std::vector<std::vector<int>> mWeightRange;
};

#endif	// _PROBABILITY_H_