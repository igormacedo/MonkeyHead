/*
 * DecisionTree.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: igormacedo
 */

#include "DecisionTree.h"

//int div = (225-45)/30;

FeatureDefinition *mainFeature = new FeatureDefinition();

boost::mutex mtx_;

//int div = (int) (225-45)/6;

DecisionTree::DecisionTree(){
	// TODO Auto-generated constructor stub

}

DecisionTree::~DecisionTree(){
	// TODO Auto-generated destructor stub
}

double DecisionTree::H(double p, double n)
{
	//Calculates the (partial) entropy of the variable
	if(p/n == 0 || p == n) return 0.0 ;
	else return -p/n*log2(p/n);
}

double DecisionTree::totalEntropy(PixelList l)
{
	double size = (double)l.size();
	return H(l.red, size) + H(l.green, size) + H(l.blue, size) + H(l.undedefined,size);
}

double DecisionTree::CalculateInformationGain(PixelList l1, PixelList l2)
{
			double sizel1 = (double)l1.size();
			double sizel2 = (double)l2.size();
			double size = sizel1+sizel2;

			double entropyl1 = totalEntropy(l1);
			double entropyl2 = totalEntropy(l2);
			double newIG = ((sizel1/size)*(entropyl1) + (sizel2/size)*(entropyl2));

			return newIG;
}

FeatureDefinition DecisionTree::MaximizeInfoGain(PixelList pixelList){

	mainFeature->clear();
	int numberOfThreads = boost::thread::hardware_concurrency();
	cout << "CoreNumber: " << numberOfThreads << endl;

	boost::thread_group thread_group;

	for(int counter = 1; counter <= numberOfThreads; counter++){
		thread_group.add_thread(new boost::thread(&DecisionTree::threadInfoGainMiximizer, this, pixelList, numberOfThreads, counter));
	}

	thread_group.join_all();

	//mainFeature->entropy = totalEntropy(pixelList);

	return *mainFeature;
}

void DecisionTree::threadInfoGainMiximizer(PixelList pixelList, int coreNumber, int threadID)
{

	string colorPath("/home/igormacedo/Blender/images/color");
	string depthPath("/home/igormacedo/Blender/images/depth");
	double mainInfoGain = DBL_MAX;


	int myconst = (int) 510/(coreNumber);
	//myconst = (threadID > coreNumber/2)? myconst: -myconst;
	//int multiplier = (int) threadID%(coreNumber/2);

	cout << "Thread " << threadID << " started" << endl;

//	for(int ang = 78; ang <= (int) (2*M_PI*100); ang += (int) (M_PI_4*100))
//	{
//		float a = ang/100.0;
//
//		cout << "Thread " << threadID << ": Trying new angle : " << a << endl;
//		for(int r = 0; r <= 80; r+= 10)
//		{
//
//			//expected minimum and maximum value for ratios of pixels in the image sample
//			for(unsigned int rat = (threadID-1)*myconst; rat < (threadID*myconst); rat += 10)
//			{

	for(int x1 = -80; x1 <= 80; x1 += 20){
		cout << "Thread " << threadID << ": Trying new x1: " << x1 << endl;
		for(int y1 = -60; y1 <= 60; y1 += 20){

			//Second pixel for comparison
			//for(int x2 = -80; x2 <= 80; x2 += 40){
				//for(int y2 = -60; y2 <= 60; y2 += 30){

					for(int diff = (threadID-1)*myconst; diff < (threadID*myconst); diff +=10){

						int x2 = 0, y2 = 0;
						FeatureDefinition newFeature(x1,y1,x2,y2,diff-255);

						PixelList* rightList = new PixelList;
						PixelList* leftList = new PixelList;

						png::image<png::rgb_pixel> img;
						int image = -1;

						for(list<Pixel*>::iterator it = pixelList.pList.begin(); it != pixelList.pList.end(); it++)
						{

							if((*it)->image != image)
							{
								image = (*it)->image;
								ostringstream st;
								st << image;
								string path = depthPath + st.str() + string(".png");

								img.read(path);
							}

							int difference;

							//int x = (int) (cos(a)*r + (*it)->x);
							//int y = (int) (sin(a)*r + (*it)->y);
							int xa = (*it)->x + x1;
							int ya = (*it)->y + y1;
							int xb = (*it)->x + x2;
							int yb = (*it)->y + y2;

							if((xa < 160 && ya < 120) && (xa >= 0 && ya >= 0) && (xb < 160 && yb < 120) && (xb >= 0 && yb >= 0))
							{

								rgb_pixel depthPixel1 = img[ya][xa];
								rgb_pixel depthPixel2 = img[yb][xb];
								difference = depthPixel2.red - depthPixel1.red;
							}
							else
							{
								difference = -256;
							}

							if(difference >= newFeature.diff)
							{
								rightList->push_back(*it);
							}
							else
							{
								leftList->push_back(*it);
							}

						}

						double newInfoGain = CalculateInformationGain(*leftList, *rightList);
						newFeature.infoGain = newInfoGain;
						newFeature.rightList = rightList;
						newFeature.leftList = leftList;

						bool changeFlag = false;
						if(newInfoGain < mainInfoGain){
							if(updateMainFeature(newFeature)){
								cout << "Thread " << threadID << ": new MAIN infoGAIN: " << mainInfoGain << endl;
								cout << "   Right list count: red=" << rightList->red << ", green=" << rightList->green <<", blue=" << rightList->blue <<", undef=" << rightList->undedefined<<endl;
								cout << "   Left list count: red=" << leftList->red << ", green=" << leftList->green <<", blue=" << leftList->blue <<", undef=" << leftList->undedefined<<endl;
								changeFlag = true;
							}

							mainInfoGain = newInfoGain;
						}

						if(!changeFlag){
							delete(rightList);
							delete(leftList);
						}
					}
				}
			}
		}
	//}
//}

bool DecisionTree::updateMainFeature(FeatureDefinition f)
{
	if(mainFeature->infoGain > f.infoGain){
		mtx_.lock();
			*mainFeature = f;
		mtx_.unlock();
		return true;
	}
	else{
		return false;
	}
}

void DecisionTree::CreateTree(PixelList* pList)
{
	this->head = new TreeNode();
	this->head->pList = pList;

	list<TreeNode*> myQueue;
	myQueue.push_back(head);

	while(!myQueue.empty())
	{
		cout << "Poping node from queue" << endl;
		TreeNode* node = myQueue.front();
		myQueue.pop_front();

		double entropy = totalEntropy(*node->pList);

		if(entropy > 0 && node->level <= 8)
		{
			cout << "Maximizing information gain for node in level " << node->level << endl;
			FeatureDefinition feature = MaximizeInfoGain(*node->pList);

			cout << "Feature definition:: << infoGain: " << feature.infoGain;
			cout << "Node level" << node->level << endl;

			TreeNode* left = new TreeNode(node->level + 1, node);
			TreeNode* right = new TreeNode(node->level + 1, node);

			left->pList = feature.leftList;
			right->pList = feature.rightList;

			node->rightNode = right;
			node->leftNode = left;

			node->feature = feature;

			cout << " === Adding left node and right node ===" << endl;
			myQueue.push_back(left);
			myQueue.push_back(right);
		}
	}

	cout << "TREE WAS CREATED!" << endl;
}

Color DecisionTree::defineColor(TreeNode* node)
{
	if(node->pList->red > 0.5*(node->pList->red + node->pList->green + node->pList->blue + node->pList->undedefined))
	{
		return RED;
	}
	else if(node->pList->green > 0.5*(node->pList->red + node->pList->green + node->pList->blue + node->pList->undedefined))
	{
		return GREEN;
	}
	else if(node->pList->blue > 0.5*(node->pList->red + node->pList->green + node->pList->blue + node->pList->undedefined))
	{
		return BLUE;
	}
	else
	{
		return UNDEFINED;
	}

}

Color DecisionTree::classifyPixel(int col, int row, image<rgb_pixel> depthImage)
{
	TreeNode* node = this->head;

	while(node->leftNode != NULL){

		//cout << "here2" << endl;
		FeatureDefinition feature = node->feature;
		int x1 = feature.x1; int y1 = feature.y1;
		int x2 = feature.x2; int y2 = feature.y2;

		int diff;

		int xa = col + x1;
		int ya = row + y1;
		int xb = col + x2;
		int yb = row + y2;

		if((xa < 160 && ya < 120) && (xa >= 0 && ya >= 0) && (xb < 160 && yb < 120) && (xb >= 0 && yb >= 0))
		{
			rgb_pixel depthPixel1 = depthImage[ya][xa];
			rgb_pixel depthPixel2 = depthImage[yb][xb];
			diff = depthPixel2.red - depthPixel1.red;
		}
		else
		{
			diff = -256;
		}

		if(diff >= feature.diff)
		{
			node = node->rightNode;
		}
		else
		{
			node = node->leftNode;
		}
	}

	return defineColor(node);
}

void DecisionTree::classifyImage(string path, string outputPath)
{
	image<rgb_pixel> depthImage(path);
	image<rgb_pixel> newImage(160,120);

	for(int row = 0; row < (int)depthImage.get_height(); row++){
		for(int col = 0; col < (int)depthImage.get_width(); col++){

//			//cout << "here1" << endl;
//			TreeNode* node = this->head;
//
//			while(node->leftNode != NULL){
//
//				//cout << "here2" << endl;
//				FeatureDefinition feature = node->feature;
//				int x1 = feature.x1; int y1 = feature.y1;
//				int x2 = feature.x2; int y2 = feature.y2;
//
//				int diff;
//
//				//int x = (int) (cos(a)*r + (*it)->x);
//				//int y = (int) (sin(a)*r + (*it)->y);
//				int xa = col + x1;
//				int ya = row + y1;
//				int xb = col + x2;
//				int yb = row + y2;
//
//				if((xa < 160 && ya < 120) && (xa >= 0 && ya >= 0) && (xb < 160 && yb < 120) && (xb >= 0 && yb >= 0))
//				{
//					rgb_pixel depthPixel1 = depthImage[ya][xa];
//					rgb_pixel depthPixel2 = depthImage[yb][xb];
//					diff = depthPixel2.red - depthPixel1.red;
//				}
//				else
//				{
//					diff = -256;
//				}
//
//				if(diff >= feature.diff)
//				{
//					node = node->rightNode;
//				}
//				else
//				{
//					node = node->leftNode;
//				}
//			}

			Color color = classifyPixel(col, row, depthImage);

			//cout << "defining color of the pixel" << endl;
			switch(color)
			{
				case RED:
					//cout << "it is red" << endl;
					newImage[row][col].red = 255;
					newImage[row][col].green = 0;
					newImage[row][col].blue = 0;
					break;
				case GREEN:
					//cout << "it is green" << endl;
					newImage[row][col].red = 0;
					newImage[row][col].green = 255;
					newImage[row][col].blue = 0;
					break;
				case BLUE:
					//cout << "it is blue" << endl;
					newImage[row][col].red = 0;
					newImage[row][col].green = 0;
					newImage[row][col].blue = 255;
					break;
				case UNDEFINED:
					//cout << "it is undefined" << endl;
					newImage[row][col].red = 200;
					newImage[row][col].green = 200;
					newImage[row][col].blue = 200;
					break;
			}
		}
	}

	cout << "writing image to" << outputPath << endl;
	newImage.write(outputPath);
}
