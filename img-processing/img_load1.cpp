#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include "stb_image.h"
#include "stb_image_write.h"
#include<omp.h>
#include <unistd.h>

using namespace std;

float BilinearlyInterpolation (float**, int, int, float, float);
void sequentialColorInversion(float**, unsigned char*, int, int, int);
void parallelColorInversion(float**, unsigned char*, int, int, int);
float nearestNeighbor(float**, int, int, float, float);


int main()
{
	int width, height, channels;
    unsigned char* data = stbi_load("shrek.jpg", &width, &height, &channels, 0);
  
    
    if(data == NULL){
    	cout << "Failed to load image" << endl;
    	exit(1);
	}
	
	int size =  width*height*channels;
	float **pixelArr = new float*[height];
	float **outputArr = new float*[size];
 
    for (int i = 0; i < height; i++) {
 
        pixelArr[i] = new float[width*channels];
        outputArr[i] = new float[width*channels];
    }
 
	for(int i=0; i<height; i++){
		for(int j=0; j<width*channels; j++){
			pixelArr[i][j] = float(data[i*width*channels + j]);
		}
	}
	
    
    cout << "Width: " << width << endl;
    cout << "Height: " << height << endl;
    cout << "Channels: " << channels << endl;
    cout << "Size: " << size << endl << endl;
    float max = -9999;
    float min = 9999;

//	for(int i=0; i<height; i++){
//		for(int j=0; j<width*channels; j++){
//			if(pixelArr[i][j] < min){
//    		min = pixelArr[i][j];
//			}
//			if(pixelArr[i][j] > max){
//				max = pixelArr[i][j];
//			}
//		}
//	}
	
	cout << "MIN: " << min << endl << "MAX: " << max << endl;

	//color inversion
	
//	double startClrInvSeq = omp_get_wtime(); 
//	sequentialColorInversion(pixelArr, data, height, width, channels);
//	double endClrInvSeq = omp_get_wtime();
//	printf("Color Inversion [ sequential ] took %f seconds\n", endClrInvSeq - startClrInvSeq);
//	
//	double startClrInvParallel = omp_get_wtime();
//	parallelColorInversion(pixelArr, data, height, width, channels);
//	double endClrInvParallel = omp_get_wtime();
//	printf("Color Inversion [ parallel ] took %f seconds\n", endClrInvParallel - startClrInvParallel);
//	
//	stbi_write_png("output.png", width, height, channels, data, channels * width * sizeof(unsigned char));
	
	

	// image warping
	float minX = 9999, minY = 9999, maxX = -9999, maxY=-9999;
	
	int index, xp, yp, tx = width*channels / 2, ty = height / 2;
	float x, y, radius, theta, PI = 3.141527f, DRAD = 180.0f / PI;
	int a,b,c,d;
//	#pragma omp parallel for shared(pixelArr, outputArr, width, height) private(x, y, index, radius, theta, xp, yp)
		for (yp = 0; yp < height; yp++) {
			for (xp = 0; xp < width*channels; xp++) {
				radius = sqrtf((xp - tx) * (xp - tx) + (yp - ty) * (yp - ty));
				theta = (radius / 2) * DRAD;
				x = cos(theta) * (xp - tx) - sin(theta) * (yp - ty) + tx;
				y = sin(theta) * (xp - tx) + cos(theta) * (yp - ty) + ty;
//				x = xp - tx;
//				y = yp - ty;
				if(x < 0){
					x *= -1;
				}
				if(y < 0){
					y *= -1;
				}
				outputArr[yp][xp] = nearestNeighbor(pixelArr, width*channels, height, x, y);
				
//				cout << pixelArr[349][241] << endl;
//				#pragma omp critical

//				outputArr[yp*width*channels + xp] = BilinearlyInterpolation(pixelArr, width*channels, height, x, y);
//				cout << outputArr[yp*width*channels + xp] << endl;
			}
		}
	for(int i=0; i<height; i++){
		for(int j=0; j<width*channels; j++){
			data[i*width*channels + j] = (unsigned char)outputArr[i][j];
		}
	}
	
//  int stbi_write_jpg(char const *filename, int w, int h, int comp, const void *data, int quality);
	stbi_write_bmp("Warpoutput.bmp", width, height, channels, data);
 }


/*
	<========== IMAGE PROCESSING FUNCTIONS ============>
*/

void sequentialColorInversion(float **input, unsigned char *output, int height, int width, int channels){
	for(int i=0; i<height; i++){
		for(int j=0; j<width*channels; j++){
			output[i*width*channels + j] = (unsigned char)(255 - input[i][j]);
		}
	}
}

void parallelColorInversion(float **input, unsigned char *output, int height, int width, int channels){
	#pragma omp parallel for num_threads(1)
	for(int i=0; i<height; i++){
		for(int j=0; j<width*channels; j++){
			output[i*width*channels + j] = (unsigned char)(255 - input[i][j]);
		}
	}
}

float nearestNeighbor(float **pixelArr, int width, int height, float x, float y){
	int A = int(x);
	int B = int(y);
	int indX, indY;
	
	if(y-B < (B+1)-y){
		indY = B+1;
	}
	else{
		indY = B;
	}
	if(x-A < (A+1)-x){
		indX = A;
	}
	else{
		indY = A+1;
	}
	
	if(indX >= width ){
		indX = width-1;
	}
	if(indX < 0){
		indX = 0;
	}
	
	if(indY >= height ){
		indY = height-1;
	}
	if(indY < 0){
		indY = 0;
	}
	return pixelArr[indY][indX];
}

float BilinearlyInterpolation (float **pixelArr,int width,int height, float x,float y)
{
	int x1,x2,y1,y2;
	x1 = int(y-1);
	y1 = int(x-1);
	x2 = int(y+1);
	y2 = int(x+1);
//	x1 = 0;
//	y1 = 0;
//	x2 = width-1;
//	y2 = height-1;


	if(x1 >= width){
		x1 = width-1;
	}
	if(x2 >= width){
		x2 = width-1;
	}
	if(y1 >= height){
		y1 = height-1;
	}
	if(y2 >= height){
		y2 = height-1;
	}
	

	float q11,q12,q22,q21;

	q11 = pixelArr[y1][x1];
	q12 = pixelArr[y2][x1];
	q21 = pixelArr[y1][x2];
	q22 = pixelArr[y2][x2];
//	cout << "Q1 : " << q11;
//	cout << "  Q12 : " << q12;
//	cout << "  Q21 : " << q21;
//	cout << "  Q22 : " << q22;
//	cout << "\n";
//	sleep(1);
	float x2x1, y2y1, x2x, y2y, yy1, xx1;
    x2x1 = x2 - x1;
    y2y1 = y2 - y1;
    x2x = x2 - x;
    y2y = y2 - y;
    yy1 = y - y1;
    xx1 = x - x1;
    return 1.0 / (x2x1 * y2y1) * (
        q11 * x2x * y2y +
        q21 * xx1 * y2y +
        q12 * x2x * yy1 +
        q22 * xx1 * yy1
    );
}
