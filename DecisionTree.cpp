/*
 * DecisionTree.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: igormacedo
 */

#include "DecisionTree.h"

//int div = (225-45)/30;

string colorPath("/home/igormacedo/Blender/images/color");
string depthPath("/home/igormacedo/Blender/images/depth");


FeatureDefinition *mainFeature = new FeatureDefinition(0,0.0);

boost::mutex mtx_;

//int div = (int) (225-45)/6;

DecisionTree::DecisionTree() {
	// TODO Auto-generated constructor stub

}

DecisionTree::~DecisionTree() {
	// TODO Auto-generated destructor stub
}

double DecisionTree::H(double p, double n)
{
	//Calculates the (partial) entropy of the variable
	if(p/n == 0 || p == n) return 0.0 ;
	else return -p/n*log2(p/n);
}

double DecisionTree::CalculateInformationGain(PixelList l1, PixelList l2)
{
			double sizel1 = (double)l1.size();
			double sizel2 = (double)l2.size();
			double size = sizel1+sizel2;

			//colorCounter countl1(l1);
			//colorCounter countl2(l2);

			double entropyl1 = H(l1.red, sizel1) + H(l1.green, sizel1) + H(l1.blue, sizel1) + H(l1.undedefined,sizel1);
			double entropyl2 = H(l2.red, sizel2) + H(l2.green, sizel2) + H(l2.blue, sizel2) + H(l2.undedefined,sizel2);
			double newIG = ((sizel1/size)*(entropyl1) + (sizel2/size)*(entropyl2));

			return newIG;
}

int DecisionTree::LoadImage(string filename)
{
	ILboolean success;
	ILuint im;

	//cout << "image 1 : " << im <<  endl;
	ilGenImages(1, &im);
	//cout << "image 1 : " << im <<  endl;
	ilBindImage(im);
	//cout << "Loading: " << filename << endl;
	string s(filename);
	char* path = &(s)[0];

	success = ilLoadImage(path);

	if(success)
	{
		success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		if(!success)
		{
			cout << "Loading " << filename << " was unsuccessful!" << endl;
			cout << "Problem with ConvertImage" << endl;
			return -1;
		}
		else
		{
			//cout << "Loaded " << filename << " successfully!" << endl;
		}
	}
	else
	{
		cout << "Loading " << filename << " was unsuccessful!" << endl;
		cout << "Problem with ilLoadImage" << endl;

		if(success == IL_COULD_NOT_OPEN_FILE)
		{
			cout << "Could not open file" << endl;
		}
		else if(success == IL_ILLEGAL_OPERATION)
		{
			cout << "Illegal operation" << endl;
		}
		else if(success == IL_INVALID_EXTENSION)
		{
			cout << "Invalid Extention" << endl;
		}
		else if(success == IL_INVALID_PARAM)
		{
			cout << "INvalid Parameter" << endl;
		}
		else if(success == IL_FALSE)
		{
			cout << "Simply False" << endl;
		}
		else
		{
			cout << "Unknown problem" << endl;
			cout << success << endl;
		}
		return -1;
	}

	return im;
}

FeatureDefinition DecisionTree::MaximizeInfoGain(PixelList pixelList){

	mainFeature->clear();
	//mainInfoGain = DBL_MAX;
	int numberOfThreads = 2;// boost::thread::hardware_concurrency();
	cout << "CoreNumber: " << numberOfThreads << endl;

	PixelList l2 = pixelList;
	//PixelList l3 = pixelList;

	cout << "TEST pixelList first blue" << pixelList.pList.size() << endl;
	cout << "TEST pixelList second blue " << l2.pList.size()  << endl;

	//pixelList.pList.clear();

	cout << "TEST pixelList first blue" << pixelList.pList.size() << endl;
	cout << "TEST pixelList second blue " << l2.pList.size()  << endl;

	boost::thread_group thread_group;
	//boost::thread myThread = new boost::thread(boost::bind( &threadInfoGainMiximizer));

	//for(int counter = 1; counter <= numberOfThreads; counter++){
		thread_group.add_thread(new boost::thread(&DecisionTree::threadInfoGainMiximizer, this, pixelList, numberOfThreads, 1));
		thread_group.add_thread(new boost::thread(&DecisionTree::threadInfoGainMiximizer, this, l2 , numberOfThreads, 2));
		//thread_group.add_thread(new boost::thread(&DecisionTree::threadInfoGainMiximizer, this, l3 , numberOfThreads, 3));
		//thread_group.create_thread(boost::bind(&DecisionTree::threadInfoGainMiximizer,this,pixelList,numberOfThreads,counter));
	//}

	thread_group.join_all();

	return *mainFeature;
}

void DecisionTree::threadInfoGainMiximizer(PixelList pixelList, int coreNumber, int threadID)
{
	double mainInfoGain = DBL_MAX;
	int image = -1;
	int myconst = (int) 255/coreNumber;
	ILuint imageID[2];

	cout << "Thread " << threadID << " started" << endl;

	for(int ang = 0; ang <= (int) (2*M_PI*100); ang += (int) (M_PI_4*100))
	{
		float a = ang/100.0;

		cout << "Trying new angle : " << a << endl;
		for(int r = 0; r <= 80; r+= 2)
		{

			//expected minimum and maximum value for ratios of pixels in the image sample
			//for(int rat = 0; rat < 255; rat += 5)
			for(int rat = (threadID-1)*myconst; rat < (threadID*myconst); rat += 5)
			{
				//cout << "Trying new ratio : " << rat/100.0 << endl;
				FeatureDefinition newFeature(r,a, rat/100.0);

				PixelList* rightList = new PixelList;
				PixelList* leftList = new PixelList;


				//if(threadID > 1){cout << "here 1" << endl;}
				int c = 0;
				for(list<Pixel*>::iterator it = pixelList.pList.begin(); it != pixelList.pList.end(); it++)
				{
					//if(threadID > 1){cout << "here loop " << c++ << endl;}


					if((*it)->image != image)
					{
						//if(threadID > 1)
						//cout << "Thread " << threadID << ": changing image from " << image << " to " << (*it)->image << endl;

						image = (*it)->image;
						ostringstream st;
						st << image;


						//char* path;
						string path = depthPath + st.str() + string(".png");
						ilDeleteImage(imageID[1]);
						imageID[1] = LoadImage(path);

						if(imageID[1] == -1)
						{
							cout << "Failure to open image!" << endl;
						}

						ilBindImage(imageID[1]);

					}

					float ratio;
					ILuint depthPixel;

					int x = (int) (cos(a)*r + (*it)->x);
					int y = (int) (sin(a)*r + (*it)->y);

					if(x <= 160 && y <= 120 && x >= 0 && y >= 0)
					{
						ilCopyPixels(x,y,0,1,1,1,IL_RGB,IL_UNSIGNED_BYTE,&depthPixel);
						ratio = (float)((depthPixel)&0xFF)/ (float) ((*it)->depth);
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

				//if(threadID > 1){cout << "here 3" << endl;}

				double newInfoGain = CalculateInformationGain(*leftList, *rightList);
				newFeature.infoGain = newInfoGain;

				if(newInfoGain < mainInfoGain){
					if(updateMainFeature(newFeature)){
						cout << "Thread " << threadID << ": new MAIN infoGAIN: " << mainInfoGain << endl;
						cout << "Right list count: red=" << rightList->red << ", green=" << rightList->green <<", blue=" << rightList->blue <<", undef=" << rightList->undedefined<<endl;
						cout << "Left list count: red=" << leftList->red << ", green=" << leftList->green <<", blue=" << leftList->blue <<", undef=" << leftList->undedefined<<endl;
					}

					mainInfoGain = newInfoGain;
				}

				delete(rightList);
				delete(leftList);

				//break;

				//hack
				if(rat == 0){rat = 45;}
			}

			//break;
		}

		//break;
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
