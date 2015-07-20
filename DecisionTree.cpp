/*
 * DecisionTree.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: igormacedo
 */

#include "DecisionTree.h"

//int div = (225-45)/30;

FeatureDefinition *mainFeature = new FeatureDefinition(0,0.0);

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

//int DecisionTree::LoadImage(string filename)
//{
//	ILboolean success;
//	ILuint im;
//
//	//cout << "image 1 : " << im <<  endl;
//	ilGenImages(1, &im);
//	//cout << "image 1 : " << im <<  endl;
//	ilBindImage(im);
//	//cout << "Loading: " << filename << endl;
//	string s(filename);
//	char* path = &(s)[0];
//
//	success = ilLoadImage(path);
//
//	if(success)
//	{
//		success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
//		if(!success)
//		{
//			cout << "Loading " << filename << " was unsuccessful!" << endl;
//			cout << "Problem with ConvertImage" << endl;
//			return -1;
//		}
//		else
//		{
//			//cout << "Loaded " << filename << " successfully!" << endl;
//		}
//	}
//	else
//	{
//		cout << "Loading " << filename << " was unsuccessful!" << endl;
//		cout << "Problem with ilLoadImage" << endl;
//
//		if(success == IL_COULD_NOT_OPEN_FILE)
//		{
//			cout << "Could not open file" << endl;
//		}
//		else if(success == IL_ILLEGAL_OPERATION)
//		{
//			cout << "Illegal operation" << endl;
//		}
//		else if(success == IL_INVALID_EXTENSION)
//		{
//			cout << "Invalid Extention" << endl;
//		}
//		else if(success == IL_INVALID_PARAM)
//		{
//			cout << "INvalid Parameter" << endl;
//		}
//		else if(success == IL_FALSE)
//		{
//			cout << "Simply False" << endl;
//		}
//		else
//		{
//			cout << "Unknown problem" << endl;
//			cout << success << endl;
//		}
//		return -1;
//	}
//
//	return im;
//}

FeatureDefinition DecisionTree::MaximizeInfoGain(PixelList pixelList){

	mainFeature->clear();
	int numberOfThreads = boost::thread::hardware_concurrency() - 1;
	cout << "CoreNumber: " << numberOfThreads << endl;

	boost::thread_group thread_group;

	for(int counter = 1; counter <= numberOfThreads; counter++){
		thread_group.add_thread(new boost::thread(&DecisionTree::threadInfoGainMiximizer, this, pixelList, numberOfThreads, counter));
	}

	thread_group.join_all();

	mainFeature->entropy = totalEntropy(pixelList);

	return *mainFeature;
}

void DecisionTree::threadInfoGainMiximizer(PixelList pixelList, int coreNumber, int threadID)
{

	string colorPath("/home/igormacedo/Blender/images/color");
	string depthPath("/home/igormacedo/Blender/images/depth");
	double mainInfoGain = DBL_MAX;

	int myconst = (int) 255/coreNumber;

	cout << "Thread " << threadID << " started" << endl;

	for(int ang = 0; ang <= (int) (2*M_PI*100); ang += (int) (M_PI_4*100))
	{
		float a = ang/100.0;

		cout << "Thread " << threadID << ": Trying new angle : " << a << endl;
		for(int r = 0; r <= 80; r+= 2)
		{

			//expected minimum and maximum value for ratios of pixels in the image sample
			for(int rat = (threadID-1)*myconst; rat < (threadID*myconst); rat += 5)
			{
				FeatureDefinition newFeature(r,a, rat/100.0);

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

					float ratio;

					int x = (int) (cos(a)*r + (*it)->x);
					int y = (int) (sin(a)*r + (*it)->y);

					if((x < 160 && y < 120) && (x >= 0 && y >= 0))
					{
						rgb_pixel depthPixel = img[y][x];
						ratio = ((int) depthPixel.red)/(float) ((*it)->depth);
					}
					else
					{
						ratio = 0;
					}

					if(ratio >= newFeature.ratio)
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

		cout << "Maximizing information gain for node in level " << node->level << endl;
		FeatureDefinition feature = MaximizeInfoGain(*node->pList);

		cout << "Feature definition:: << infoGain: " << feature.infoGain;
		cout << "Node level" << node->level << endl;

		if(feature.entropy > 0 && node->level < 8)
		{
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

void DecisionTree::classifyImage(image<rgb_pixel> depthImage)
{
	image<rgb_pixel> newImage(160,120);

	for(int row = 0; row < (int)depthImage.get_height(); row++){
		for(int col = 0; col < (int)depthImage.get_width(); col++){

			//cout << "here1" << endl;
			TreeNode* node = this->head;

			while(node->leftNode != NULL){

				//cout << "here2" << endl;
				FeatureDefinition feature = node->feature;
				float a = feature.angle;
				int r = feature.radius;

				float ratio;

				int x = (int) (cos(a)*r + col);
				int y = (int) (sin(a)*r + row);

				if((x < 160 && y < 120) && (x >= 0 && y >= 0))
				{
					//cout << "here inside if" << endl;
					ratio = ((int) depthImage[y][x].red)/(float) depthImage[row][col].red;
				}
				else
				{
					ratio = 0;
				}

				//cout << "comparing ration" << endl;
				//Compare with feature ration
				if(ratio >= feature.ratio)
				{
					//rightList->push_back(*it);
					node = node->rightNode;
				}
				else
				{
					//leftList->push_back(*it);
					node = node->leftNode;
				}
			}

			//cout << "defining color of the pixel" << endl;
			switch(defineColor(node))
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

	cout << "writing image to /home/igormacedo/Blender/newImage.png" << endl;
	newImage.write(string("/home/igormacedo/Blender/newImage.png"));
}
