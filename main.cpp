#include <IL/il.h>
#include <GL/glut.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>
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

bool fullscreen = false;

DecisionTree myTree;

string cPath("/home/igormacedo/Blender/images/color");
string dPath("/home/igormacedo/Blender/images/depth");


PixelList* createPixelList();
void display();
void init(int width, int height);
int LoadImage(char* filename);
void specialKeyboard(int key, int x, int y);
void evaluateImage();

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
		//createDecisionTree();
		clock_t start = clock();
		PixelList* p = createPixelList();
		cout << "Time to create pixel list: " << ((clock() - start) / (float)CLOCKS_PER_SEC) << endl;
		cout << "PixelList size: " << p->size() << endl;

		if(!p->isEmpty())
		{
			time_t start,end;
			time (&start);
			//FeatureDefinition myBestFit = myTree.MaximizeInfoGain(*p);
			myTree.CreateTree(p);
			time (&end);
			double dif = difftime (end,start);
			//cout << "Time to MaximizeInfoGain: " << dif << " seconds" << endl;

			//cout << "Defined feature > ";
			//cout << "Radius = " << myBestFit.radius << ", angle = " <<  myBestFit.angle << ", ratio = " <<  myBestFit.ratio << endl;
		}
		else
		{
			cout << "Error creating PixelList!";
		}
	}

	if (key == GLUT_KEY_F12)
	{
		cout << "Opening Image" << endl;
		png::image<png::rgb_pixel> img("/home/igormacedo/Blender/images/color0.png");

		rgb_pixel pixel = img[119][159];
		cout << "RED:" << (int)pixel.red << " GREEN:" << (int)pixel.green << " BLUE:" << (int) pixel.blue << endl;

		img.read("/home/igormacedo/Blender/images/color12.png");
		pixel = img[60][60];
		cout << "RED:" << (int)pixel.red << " GREEN:" << (int)pixel.green << " BLUE:" << (int) pixel.blue << endl;

	}

	if(key == GLUT_KEY_F3)
	{

	}

	if(key == GLUT_KEY_F4)
	{
		evaluateImage();
	}
}

PixelList* createPixelList()
{
	PixelList* pList = new PixelList();

	for(int im = 0; im < 25; im++)
	{
		ostringstream st;
		st << im;

		char* path;
		path = &(cPath + st.str() + string(".png"))[0];
		img[0] = LoadImage(path);

		path = &(dPath + st.str() + string(".png"))[0];
		img[1] = LoadImage(path);

		if(img[0] == -1 || img[1] == -1)
		{
			PixelList* p;
			return p;
		}

		int imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		int imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);

		cout << "Creating pixels... " << endl;
		for(int x = 0; x <= imageWidth; x++)
		{
			for(int y = 0; y <= imageHeight; y++)
			{

				ilBindImage(img[1]);
				ILuint depthPixel;
				ilCopyPixels(x,y,0,1,1,1,IL_RGB,IL_UNSIGNED_BYTE,&depthPixel);

				//Pixel* pixel = (Pixel*)malloc(sizeof(Pixel));
				Pixel* pixel = new Pixel;
				pixel->depth = ((depthPixel) & 0xFF);
				pixel->x = x;
				pixel->y = y;
				pixel->image = im;

				ILuint colorPixel;
				ilBindImage(img[0]);
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
				}

				//if (pixel->color != UNDEFINED)
				//{
					pList->push_back(pixel);
					//cout << "Pixel ("<<pixel->x <<":"<<pixel->y<<"): depth=" << pixel->depth << ", color=" << pixel->color << " image:" << pixel->image << endl;
				//}
			}
		}

		ilDeleteImages(2, img);
	}

	return pList;
}

void evaluateImage()
{
	string path;
	cout << "Type depth image path: ";
	cin >> path;
	cout << "Loading: " << path << endl;

	png::image<rgb_pixel> image(path);


	char* pathChar;
	pathChar = &(path)[0];
	//ILuint newImage;
	img[2] = LoadImage(pathChar);
	display();

	myTree.classifyImage(image);

}

int main(int argc, char **argv)
{
	//GLuint texid;
	//int image;

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

//	ilBindImage(image[0]);
//	ILuint Data;
//	ilCopyPixels(81,80,0,1,1,1,IL_RGB,IL_UNSIGNED_BYTE,&Data);
//
//
//
//	cout << "R: > " << ((Data) & 0xFF) << endl;
//	cout << "G: > " << ((Data >> 8 ) & 0xFF) << endl;
//	cout << "B: > " << ((Data >> 16 ) & 0xFF) << endl;

    glutMainLoop();

    //ILuint i = (ILuint) image;
    ilDeleteImages(2, img);
    //glDeleteTextures(1, &texid[0]);
    //glDeleteTextures(1, &texid[1]);
    return 0;
}


















