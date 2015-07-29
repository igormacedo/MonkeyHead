#include <IL/il.h>
#include <GL/glut.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <png++/png.hpp>
#include "DecisionTree.h"

using namespace std;
using namespace png;


#define DEFAULT_WIDTH  1280
#define DEFAULT_HEIGHT 720

int width  = DEFAULT_WIDTH;
int height = DEFAULT_HEIGHT;

GLuint texid[4];
ILuint img[4];
set<int> usedImages;

bool fullscreen = false;

DecisionTree myTree1, myTree2, myTree3;

string cPath("/home/igormacedo/Blender/images/color");
string dPath("/home/igormacedo/Blender/images/depth");


PixelList* createPixelList(int numberOfImages, int maxNumber);
void display();
void init(int width, int height);
int LoadImage(char* filename);
void specialKeyboard(int key, int x, int y);
void evaluateImagewithSingleTree();
void evaluateImagewithForest();

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glBindTexture(GL_TEXTURE_2D, texid[0]);
	ilBindImage(img[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
    glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
	 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	glBegin(GL_QUADS);
		glTexCoord2i(0,0); glVertex2i(50,50);
	    glTexCoord2i(0,1); glVertex2i(50,   height/2);
	    glTexCoord2i(1, 1); glVertex2i(width/2-150, height/2);
	    glTexCoord2i(1, 0); glVertex2i(width/2-150, 50);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texid[1]);
	ilBindImage(img[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
	 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2i(width/2+150,50);
		    glTexCoord2i(0, 1); glVertex2i(width/2+150,   height/2);
		    glTexCoord2i(1, 1); glVertex2i(width-50, height/2);
		    glTexCoord2i(1, 0); glVertex2i(width-50, 50);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texid[2]);
	ilBindImage(img[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
	 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	glBegin(GL_QUADS);
			glTexCoord2i(0,0); glVertex2i(50,height/2 + 30);
		    glTexCoord2i(0,1); glVertex2i(50,   height-50);
		    glTexCoord2i(1, 1); glVertex2i(width/2-150, height-50);
		    glTexCoord2i(1, 0); glVertex2i(width/2-150, height/2 + 30);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texid[3]);
	ilBindImage(img[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); /* We will use linear interpolation for magnification filter */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); /* We will use linear interpolation for minifying filter */
	glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP), ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
	 0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE, ilGetData());

	glBegin(GL_QUADS);
			glTexCoord2i(0, 0); glVertex2i(width/2+150,height/2 + 30);
		    glTexCoord2i(0, 1); glVertex2i(width/2+150,   height-50);
		    glTexCoord2i(1, 1); glVertex2i(width-50, height-50);
		    glTexCoord2i(1, 0); glVertex2i(width-50, height/2 + 30);
	glEnd();

	glutSwapBuffers();

}

void init(int width, int height)
{
	glViewport(0, 0, width, height);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, height, 0.0, 0.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
	glClearDepth(0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cout << "texture 1 : " << texid[0] << "|| texture 2: " << texid[1] << endl;
	glGenTextures(4, texid);
	cout << "texture 1 : " << texid[0] << "|| texture 2: " << texid[1] << endl;
	//glGenTextures(2, texid[1]);
}

int LoadImage(char* filename)
{
	ILboolean success;
	ILuint im;

	cout << "image 1 : " << im <<  endl;
	ilGenImages(1, &im);
	cout << "image 1 : " << im <<  endl;
	ilBindImage(im);
	cout << "Loading: " << filename << endl;
	success = ilLoadImage(filename);

	if(success)
	{
		success = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		if(!success)
		{
			return -1;
		}
		else
		{
			cout << "Loaded " << filename << " successfully!" << endl;
		}
	}
	else
	{
		return -1;
	}

	return im;
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(DEFAULT_WIDTH, DEFAULT_HEIGHT);
			glutPositionWindow(50, 50);
		}
	}
	if (key == GLUT_KEY_F2)
	{
		clock_t start = clock();
		usedImages.clear();

		PixelList* p1 = createPixelList(8,25);
		PixelList* p2 = createPixelList(8,25);
		PixelList* p3 = createPixelList(9,25);
		cout << "Time to create pixel list: " << ((clock() - start) / (float)CLOCKS_PER_SEC) << endl;
		cout << "PixelList size: " << p1->size() << endl;

		if(!p1->isEmpty() && !p2->isEmpty() && !p3->isEmpty())
		{
			time_t start,end, startTotal, endTotal;
			time (&start);
			time (&startTotal);
			//FeatureDefinition myBestFit = myTree.MaximizeInfoGain(*p);
			cout << "================================    Creating TREE 1 ======================================================================" << endl;
			myTree1.CreateTree(p1);
			time (&end);
			double dif = difftime (end,start);
			cout << "Time to create Decision Tree: " << dif << " seconds" << endl;

			time (&start);
			cout << "================================    Creating TREE 2 ======================================================================" << endl;
			myTree2.CreateTree(p2);
			time (&end);
			dif = difftime (end,start);
			cout << "Time to create Decision Tree: " << dif << " seconds" << endl;

			time (&start);
			cout << "================================    Creating TREE 3 ======================================================================" << endl;
			myTree3.CreateTree(p3);
			time (&end);
			time (&endTotal);
			dif = difftime (end,start);
			cout << "Time to create Decision Tree: " << dif << " seconds" << endl;
			dif = difftime (endTotal,startTotal);
			cout << "Time to create complete Decision Forest: " << dif << " seconds" << endl;
		}
		else
		{
			cout << "Error creating PixelList!";
		}
	}

	if(key == GLUT_KEY_F3)
	{
		evaluateImagewithSingleTree();
	}

	if(key == GLUT_KEY_F4)
	{
		evaluateImagewithForest();
	}
}

PixelList* createPixelList(int numberOfImages, int maxNumber)
{
	PixelList* pList = new PixelList();
	int imageCounter = 0;
	ILuint imge[2];
	//for(int im = 0; im < 24; im++)
	//{
	while(imageCounter < numberOfImages){

		int im;
		do{
			im = rand() % maxNumber;
		}while(usedImages.find(im) != usedImages.end());
		imageCounter++;


		ostringstream st;
		st << im;

		char* path;
		path = &(cPath + st.str() + string(".png"))[0];
		imge[0] = LoadImage(path);

		path = &(dPath + st.str() + string(".png"))[0];
		imge[1] = LoadImage(path);

		if(imge[0] == -1 || imge[1] == -1)
		{
			PixelList* p;
			return p;
		}

		int imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		int imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);

		cout << "Creating pixels... " << endl;
		for(int y = 0; y <= imageHeight; y = y+1)
		{
			for(int x = 0; x <= imageWidth; x = x+2)
			{

				ilBindImage(imge[1]);
				ILuint depthPixel;
				ilCopyPixels(x,y,0,1,1,1,IL_RGB,IL_UNSIGNED_BYTE,&depthPixel);

				Pixel* pixel = new Pixel;
				pixel->depth = ((depthPixel) & 0xFF);
				pixel->x = x;
				pixel->y = y;
				pixel->image = im;

				ILuint colorPixel;
				ilBindImage(imge[0]);
				ilCopyPixels(x,y,0,1,1,1,IL_RGB,IL_UNSIGNED_BYTE,&colorPixel);


				if(((colorPixel) & 0xFF) == 255)
				{
					pixel->color = RED;
				}
				else if(((colorPixel >> 8) & 0xFF) == 255)
				{
					pixel->color = GREEN;
				}
				else if(((colorPixel >> 16) & 0xFF) == 255)
				{
					pixel->color = BLUE;
				}
				else
				{
					pixel->color = UNDEFINED;
					x = x+30; //Jump pixels
				}

				//if (pixel->color != UNDEFINED)
				//{
					pList->push_back(pixel);
					//cout << "Pixel ("<<pixel->x <<":"<<pixel->y<<"): depth=" << pixel->depth << ", color=" << pixel->color << " image:" << pixel->image << endl;
				//}
			}
		}

		ilDeleteImages(2, imge);
	}

	return pList;
}

void evaluateImagewithSingleTree()
{
	int quantity;
	cout << "Classifying images until:  ";
	cin >> quantity;

	for(int im = 0; im < quantity; im++)
	{
		ostringstream st;
		st << im;
		myTree1.classifyImage(dPath + st.str() + string(".png"), string("/home/igormacedo/Blender/newImage") + st.str() + string(".png"));
	}

}

Color getColor(Color c1, Color c2, Color c3)
{
	if((c1 == RED && c2 == RED) || (c1 == RED && c3 == RED) || (c3 == RED && c2 == RED))
	{
		return RED;
	}
	else if((c1 == GREEN && c2 == GREEN) || (c1 == GREEN && c3 == GREEN) || (c3 == GREEN && c2 == GREEN))
	{
		return GREEN;
	}
	else if((c1 == BLUE && c2 == BLUE) || (c1 == BLUE && c3 == BLUE) || (c3 == BLUE && c2 == BLUE))
	{
		return BLUE;
	}
	else
	{
		return UNDEFINED;
	}
}

void evaluateImagewithForest()
{
	int quantity;
	cout << "Classifying images until: ";
	cin >> quantity;

	for(int im = 0; im < quantity; im++)
	{
		ostringstream st;
		st << im;

		image<rgb_pixel> depthImage(dPath + st.str() + string(".png"));
		image<rgb_pixel> newImage(160,120);

		for(int row = 0; row < (int)depthImage.get_height(); row++){
			for(int col = 0; col < (int)depthImage.get_width(); col++){

				Color color1 = myTree1.classifyPixel(col, row, depthImage);
				Color color2 = myTree2.classifyPixel(col, row, depthImage);
				Color color3 = myTree3.classifyPixel(col, row, depthImage);

				Color color = getColor(color1,color2,color3);

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

		string outputPath = string("/home/igormacedo/Blender/newImage") + st.str() + string(".png");
		cout << "writing image to" << outputPath << endl;
		newImage.write(outputPath);

	}

}

int main(int argc, char **argv)
{

	if( argc < 1)
	{
		return -1;
	}

	glutInit(&argc,argv);
		glutInitDisplayMode(GLUT_DOUBLE);
		glutInitWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
		glutCreateWindow(argv[0]);
		glutDisplayFunc(display);
		glutSpecialFunc(specialKeyboard);

	init(DEFAULT_WIDTH, DEFAULT_HEIGHT);


	if(ilGetInteger(IL_VERSION_NUM) < IL_VERSION)
	{
		cout << "Wrong DevIL version \n" << endl;
		return -1;
	}
	ilInit();

	img[0] = LoadImage(argv[1]);
	if(img[0] == -1)
	{
		cout << "Could not load image" << argv[1] << " by DevIL" << endl;
		return -1;
	}

	void* data1 = ilGetData();
	cout << "data1: " << data1 << endl;


    img[1] = LoadImage(argv[2]);
	if(img[1] == -1)
	{
		cout << "Could not load image" << argv[2] << " by DevIL" << endl;
		return -1;
	}

	display();

    glutMainLoop();

    //ILuint i = (ILuint) image;
    ilDeleteImages(2, img);
    //glDeleteTextures(1, &texid[0]);
    //glDeleteTextures(1, &texid[1]);
    return 0;
}


















