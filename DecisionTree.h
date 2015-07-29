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
using namespace png;

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

typedef struct FeatureDefinition
{
	int x1,y1,x2,y2;
	//float angle;
	int diff;
	double infoGain;
	PixelList* rightList = NULL;
	PixelList* leftList = NULL;
	double entropy = 0.0;

	FeatureDefinition()
	{
		this->x1 = 0; this->y1 = 0;
		this->x2 = 0; this->y2 = 0;
		this->diff = 0;
		this->infoGain = 0.0;
	}

	FeatureDefinition(int x1, int y1, int x2, int y2, int diff)
	{
		this->x1 = x1; this->y1 = y1;
		this->x2 = x2; this->y2 = y2;
		this->diff = diff;
		this->infoGain = 0;
	}

	void clear()
	{
		this->x1 = 0; this->y1 = 0;
		this->x2 = 0; this->y2 = 0;
		this->diff = 0;
		this->infoGain = DBL_MAX;
		this->entropy = 0.0;
		this->rightList = NULL;
		this->leftList = NULL;
	}

}FeatureDefinition;

typedef struct TreeNode
{
	TreeNode* leftNode;
	TreeNode* rightNode;
	TreeNode* parent;
	int level;

	FeatureDefinition feature;

	PixelList* pList = NULL;

	TreeNode()
	{
		this->leftNode = NULL;
		this->rightNode = NULL;
		this->parent = NULL;
		this->level = 1;
	}

	TreeNode(int level, TreeNode* parent)
	{
		this->rightNode = NULL;
		this->leftNode = NULL;
		this->level = level;
		this->parent = parent;
	}

}TreeNode;

class DecisionTree{
public:
	DecisionTree();
	virtual ~DecisionTree();
	FeatureDefinition MaximizeInfoGain(PixelList);
	void CreateTree(PixelList*);
	void classifyImage(string, string);
	Color classifyPixel(int, int, image<rgb_pixel>);

private:
	TreeNode* head;
	list<TreeNode> myTree;

	double H(double, double);
	int LoadImage(string);
	double CalculateInformationGain(PixelList, PixelList);
	void threadInfoGainMiximizer(PixelList, int, int);
	bool updateMainFeature(FeatureDefinition);
	double totalEntropy(PixelList);
	Color defineColor(TreeNode*);

};

#endif /* DECISIONTREE_H_ */
