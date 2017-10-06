/******************************************************************************
* Author:	jaron.ho
* Date:		2016-04-10
* Brief:	calculate probability by weight list
******************************************************************************/
#include "Probability.h"
//--------------------------------------------------------------------------
Probability::Probability(const std::vector<std::vector<int>>& weightList)
{
	mWeightListInit = weightList;
	mWeightList = weightList;
	mThreshold = 1;
	parseWeightList();
}
//--------------------------------------------------------------------------
void Probability::parseWeightList(void)
{
	mThreshold = 1;
	size_t count = mWeightList.size();
	for (size_t i=0; i<count; ++i)
	{
		size_t r = rand()%count;
		if (r != i)
		{
			std::vector<int> temp = mWeightList[r];
			mWeightList[r] = mWeightList[i];
			mWeightList[i] = temp;
		}
	}
	for (size_t i=0; i<count; ++i)
	{
		std::vector<int> factor = mWeightList[i];
		int value = factor[0];
		int weight = factor[1];
		size_t begin = mThreshold;
		size_t end = mThreshold + weight - 1;
		std::vector<int> range;
		range[0] = value;
		range[1] = begin;
		range[2] = end;
		mWeightRange.push_back(range);
		mThreshold = mThreshold + weight;
	}
}
//--------------------------------------------------------------------------
int Probability::getValue(void)
{
	if (1 == mThreshold)
	{
		return 0;
	}
	size_t index = rand()%(mThreshold - 1);
	size_t count = mWeightRange.size();
	for (size_t i=0; i<count; ++i)
	{
		int value = mWeightRange[i][0];
		size_t begin = mWeightRange[i][1];
		size_t end = mWeightRange[i][2];
		if (index >= begin && index <= end)
		{
			return value;
		}
	}
	return 0;
}
//--------------------------------------------------------------------------
int Probability::getWeight(int value)
{
	size_t count = mWeightList.size();
	for (size_t i=0; i<count; ++i)
	{
		if (value == mWeightRange[i][0])
		{
			return mWeightRange[i][1];
		}
	}
	return 0;
}
//--------------------------------------------------------------------------
void Probability::setWeight(int value, int weight)
{
	bool isFind = false;
	size_t count = mWeightList.size();
	for (size_t i=0; i<count; ++i)
	{
		if (value == mWeightRange[i][0])
		{
			mWeightRange[i][1] = weight;
			isFind = true;
			break;
		}
	}
	if (!isFind)
	{
		std::vector<int> info;
		info[0] = value;
		info[1] = weight;
		mWeightList.push_back(info);
	}
	parseWeightList();
}
//--------------------------------------------------------------------------
void Probability::reset(void)
{
	mWeightList.clear();
	size_t count = mWeightListInit.size();
	for (size_t i=0; i<count; ++i)
	{
		std::vector<int> info;
		info[0] = mWeightListInit[i][0];
		info[1] = mWeightListInit[i][1];
		mWeightList.push_back(info);
	}
	parseWeightList();
}
//--------------------------------------------------------------------------