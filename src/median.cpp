#include <stdio.h>
#include <mpi.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


void insertionSort(int window[], int windowsSize)
{
	int temp, i , j;
	for(i = 0; i < windowsSize; i++){
		temp = window[i];
		for(j = i-1; j >= 0 && temp < window[j]; j--){
			window[j+1] = window[j];
		}
		window[j+1] = temp;
	}
}

int main(int argc, char** argv) {
	int windowsSize = 1;
	
	cv::Mat src;
	unsigned char* inData;
	unsigned char* outData;
	
	unsigned char* displayData;
	
	int imgSize;
	int imgRows;

	MPI_Init(NULL, NULL);

	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	//Load image and broadcast size so that every element can reserve the necessary memory
	if (rank == 0) {
			src = cv::imread("/home/emadrigal/Pictures/Lenna.png", CV_LOAD_IMAGE_GRAYSCALE);

			
			imgSize = src.cols*src.rows;
			imgRows = src.rows;
			//Third argument indicates that 0 sends the data
			MPI_Bcast(&imgSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&imgRows, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}

	else {
			
			MPI_Bcast(&imgSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&imgRows, 1, MPI_INT, 0, MPI_COMM_WORLD);

			inData =  (unsigned char*)malloc(imgSize*sizeof(unsigned char));
	}

	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	
	
	//Broadcast image to all elements
	if (rank == 0) {
			//Third argument indicates that 0 sends the data
			MPI_Bcast(src.data, src.cols*src.rows, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
			
			outData = (unsigned char*) malloc(imgSize/size);
			int* window = (int*) malloc((windowsSize*2 + 1)*(windowsSize*2 + 1));
			int j;
			for(int i = 0; i < (imgSize/size); i++){
					//Index to adress actual image data
					j = i + (imgSize/size)*rank;
			
					if( ((j + imgRows + windowsSize) < imgSize) && ((j - imgRows - windowsSize) > 0)){
							int cnt = 0;
							// Pick up window element
							for(int u = -windowsSize; u < windowsSize; u++){
								for(int v = -windowsSize; v < windowsSize; v++){
									window[cnt] = src.data[j + u + v*windowsSize];
								}
							}
					}
					
					// sort the window to find median
					insertionSort(window, 9);
					
					// assign the median to centered element of the matrix
					outData[i] = window[4];
			}
	}
	else {
			MPI_Bcast(inData, imgSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
			
			
			outData = (unsigned char*) malloc(imgSize/size);
			int window[9];
			int i = 0;
			int j;
			for(; i < (imgSize/size); i++){
					//Index to adress actual image data
					j = i + (imgSize/size)*rank;
			
					if( ((j + imgRows + 1) < imgSize) && ((j - imgRows - 1) > 0)){
							// Pick up window element
							window[0] = inData[j];
							window[1] = inData[j + 1];
							window[2] = inData[j - 1];
							window[3] = inData[j + imgRows];
							window[4] = inData[j - imgRows];
							window[5] = inData[j - imgRows + 1];
							window[6] = inData[j - imgRows - 1];
							window[7] = inData[j + imgRows + 1];
							window[8] = inData[j + imgRows  -1];
					}
					
					// sort the window to find median
					insertionSort(window, 9);
					
					// assign the median to centered element of the matrix
					outData[i] = window[4];
			}
	}
	
	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == 0){
		displayData = (unsigned char*) malloc(imgSize);
		
		MPI_Gather(
			outData,
			(imgSize/size),
			MPI_UNSIGNED_CHAR,
			
			displayData,
			(imgSize/size),//This is per process
			MPI_UNSIGNED_CHAR,
			
			0,
			MPI_COMM_WORLD);
	}
	else{
		MPI_Gather(
			outData,
			(imgSize/size),
			MPI_UNSIGNED_CHAR,
			
			NULL,
			0,
			MPI_UNSIGNED_CHAR,
			
			0,
			MPI_COMM_WORLD);
	}
	
	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == 0){
	
			cv::Mat dst = cv::Mat(cvSize(src.cols, src.rows), CV_8U, outData);
			
			cv::namedWindow("final");
			cv::imshow("final", dst);

			cv::namedWindow("initial");
			cv::imshow("initial", src);
			
			cv::waitKey();
	}
	MPI_Finalize();
}
