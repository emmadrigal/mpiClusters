#include <stdio.h>
#include <mpi.h>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


void insertionSort(int window[], int windowsSize){
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
	int windowsSize = 2;
	
	cv::Mat src;
	unsigned char* inData;
	unsigned char* outData;
	
	unsigned char* displayData;
	
	int imgSize = 0;
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
		
		printf("%d, %d", imgSize, imgRows);
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
		int windowElements = (windowsSize*2 + 1)*(windowsSize*2 + 1);
		int* window = (int*) malloc(windowElements*sizeof(int));
		int j;
		for(int i = 0; i < (imgSize/size); i++){
			//Index to adress actual image data
			j = i + (imgSize/size)*rank;
	
			
			int cnt = 0;
			// Pick up window element
			for(int u = -windowsSize; u <= windowsSize; u++){
				for(int v = -windowsSize; v <= windowsSize; v++){
					if( ((j + v + u*imgRows) < imgSize) && ((j + v + u*imgRows) > 0)){
						window[cnt] = src.data[j + v + u*imgRows];
					}
					else
						window[cnt] = 0;
					cnt++;
				}
			}
			
			
			// sort the window to find median
			insertionSort(window, windowElements);
			
			// assign the median to centered element of the matrix
			outData[i] = window[windowElements/2];
		}
	}
	else {
		MPI_Bcast(inData, imgSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
		
		int windowElements = (windowsSize*2 + 1)*(windowsSize*2 + 1);
		
		outData = (unsigned char*) malloc(imgSize/size);
		int* window = (int*) malloc(windowElements*sizeof(int));
		int i = 0;
		int j;
		for(; i < (imgSize/size); i++){
				//Index to adress actual image data
				j = i + (imgSize/size)*rank;	
				
				int cnt = 0;
				// Pick up window element
				for(int u = -windowsSize; u <= windowsSize; u++){
					for(int v = -windowsSize; v <= windowsSize; v++){
						if( ((j + v + u*imgRows) < imgSize) && ((j + v + u*imgRows) > 0)){
							window[cnt] = src.data[j + v + u*imgRows];
						}
						else
							window[cnt] = 0;
						cnt++;
					}
				}
				
				// sort the window to find median
				insertionSort(window, windowElements);
				
				// assign the median to centered element of the matrix
				outData[i] = window[windowElements/2];
		}
	}
	
	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	
	if(rank == 0){
		//displayData = (unsigned char*) malloc(imgSize*sizeof(unsigned char)/4);
		displayData = (unsigned char*) malloc(124800*sizeof(unsigned char));
		
		
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
	
			cv::Mat dst = cv::Mat(cvSize(src.cols, src.rows), CV_8U, displayData);
			
			cv::namedWindow("final");
			cv::imshow("final", dst);

			cv::namedWindow("initial");
			cv::imshow("initial", src);
			
			cv::waitKey();
	}
	MPI_Finalize();
}
