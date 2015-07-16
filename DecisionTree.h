/*
 * DecisionTree.h
 *
 *  Created on: Jun 30, 2015
 *      Author: igormacedo
 */

#include <list>
#include <math.h>
#include <IL/il.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <cfloat>
#include <boost/thread.hpp>
#include <png++/png.hpp>

using namespace std;

#ifndef DECISIONTREE_H_
#define DECISIONTREE_H_


enum Color {RED, GREEN, BLUE, UNDEFINED};

typedef struct Pixel
{
	Color color;
	int depth;
	int image;
	int x;
	int y;

}Pixel;

typedef struct FeatureDefinition
{
	int radius;
	float angle;
	float ratio;
	double infoGain;

	FeatureDefinition()
	{
		this->radius = 0;
		this->angle = 0.0;
		this->ratio = 0.0;
		this->infoGain = 0.0;
	}

	FeatureDefinition(int radius, float angle)
	{
		this->radius = radius;
		this->angle = angle;
		this->ratio = 0.0;
		this->infoGain = 0.0;
	}

	FeatureDefinition(int radius, float angle, float ratio)
	{
		this->radius = radius;
		this->angle = angle;
		this->ratio = ratio;
		this->infoGain = 0.0;
	}

	void clear()
	{
		this->radius = 0;
		this->angle = 0.0;
		this->ratio = 0.0;
		this->infoGain = DBL_MAX;
	}

}FeatureDefinition;

typedef struct PixelList
{
	list<Pixel*> pList;
	int red;
	int green;
	int blue;
	int undedefined;

	PixelList()
	{
		this->red = 0;
		this->green = 0;
		this->blue = 0;
		this->undedefined = 0;
	}

	~PixelList()
	{
		//delete(pList);
	}

	void push_back(Pixel* it)
	{
		if(it->color == RED)
		{
			red++;
		}
		else if(it->color == GREEN)
		{
			green++;
		}
		else if(it->color == BLUE)
		{
			blue++;
		}
		else
		{
			undedefined++;
		}

		pList.push_back(it);
	}

	int size()
	{
		return this->pList.size();
	}

	bool isEmpty()
	{
		if(pList.size() <= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

}PixelList;

typedef struct TreeNode
{
	TreeNode* leftNode;
	TreeNode* rightNode;

	FeatureDefinition feature;

	PixelList pList;

}TreeNode;

class DecisionTree{
public:
	DecisionTree();
	virtual ~DecisionTree();
	FeatureDefinition MaximizeInfoGain(PixelList);

private:
	double H(double, double);
	int LoadImage(string);
	double CalculateInformationGain(PixelList, PixelList);
	void threadInfoGainMiximizer(PixelList, int, int);
	bool updateMainFeature(FeatureDefinition);

};

#endif /* DECISIONTREE_H_ */
