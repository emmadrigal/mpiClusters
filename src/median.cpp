#include <string>
#include <iostream>
#include <algorithm>    // std::partial_sort

//Used for time measurement
#include <chrono>

//Support for messages in the cluster
#include <mpi.h>

//Used for interaction with the user
#include <boost/program_options.hpp>

//Used for image manipulation
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


namespace po = boost::program_options;

/*
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
*/

void display(std::string imgAdd, int windowSize){
	double start, end;
	
	cv::Mat src, dst;
	
	unsigned char* inData;
	unsigned char* outData;
	unsigned char* displayData;
	
	int imgSize;
	int imgRows;
	int imgCols;
	
	MPI_Init(NULL, NULL);
	
	int rank;
	int size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	
	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
	start = MPI_Wtime();
	
	//Broadcast image dimenstions to all nodes so that they can reserve memory
	if (rank == 0) {
		src = cv::imread(imgAdd, CV_LOAD_IMAGE_GRAYSCALE);
		dst = cv::Mat(src.rows, src.cols, CV_8U);
		
		if( !src.data ){
			printf("Image not found\n");
			return;
		}
		
		imgSize = src.cols*src.rows;
		imgRows = src.rows;
		imgCols = src.cols;
		
		//Third argument indicates that 0 sends the data
		MPI_Bcast(&imgSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&imgRows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&imgCols, 1, MPI_INT, 0, MPI_COMM_WORLD);
	}
	else {
		MPI_Bcast(&imgSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&imgRows, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&imgCols, 1, MPI_INT, 0, MPI_COMM_WORLD);
		
		inData =  (unsigned char*)malloc(imgSize*sizeof(unsigned char));
	}
	
	
	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);
	
	
	//Broadcast image to all elements and process their data
	if (rank == 0) {
		//Third argument indicates that 0 sends the data
		MPI_Bcast(src.data, src.cols*src.rows, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
		
		outData = (unsigned char*) malloc((imgSize/size) * sizeof(unsigned char));
		
		int windowElements = (windowSize*2 + 1)*(windowSize*2 + 1);
		
		int* window = (int*) malloc(windowElements*sizeof(int));
		
		int j;
		for(int i = 0; i < (imgSize/size); i++){
			j = i + (imgSize/size)*rank;
			
			int cnt = 0;
			// Pick up window element
			for(int u = -windowSize; u <= windowSize; u++){
				for(int v = -windowSize; v <= windowSize; v++){
					int row = (j + v + u*src.rows) / imgRows;
					int col = (j + v + u*src.rows) % imgRows;
					
					if( (row > 0) && (row < imgRows) && (col > 0) && (col < imgCols) )
						window[cnt] = src.data[j + v + u*src.rows];
					else
						window[cnt] = 128;
					cnt++;
				}
			}
			
			// sort the window to find median
			std::partial_sort (window, window + windowSize/2 + 1, window + windowSize);

			// assign the median to centered element of the matrix
			outData[i] = window[windowSize/2];
		}
	}
	else {
		MPI_Bcast(inData, imgSize, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
		
		outData = (unsigned char*) malloc((imgSize/size) * sizeof(unsigned char));
		
		int windowElements = (windowSize*2 + 1)*(windowSize*2 + 1);
		
		int* window = (int*) malloc(windowElements*sizeof(int));
		
		int j;
		for(int i = 0; i < (imgSize/size); i++){
			j = i + (imgSize/size)*rank;
			
			int cnt = 0;
			// Pick up window element
			for(int u = -windowSize; u <= windowSize; u++){
				for(int v = -windowSize; v <= windowSize; v++){
					int row = (j + v + u*src.rows) / imgRows;
					int col = (j + v + u*src.rows) % imgRows;
					
					if( (row > 0) && (row < imgRows) && (col > 0) && (col < imgCols) )
						window[cnt] = inData[j + v + u*src.rows];
					else
						window[cnt] = 128;
					cnt++;
				}
			}

			// sort the window to find median
			std::partial_sort (window, window + windowSize/2 + 1, window + windowSize);

			// assign the median to centered element of the matrix
			outData[i] = window[windowSize/2];
		}
	}
	
	//Waits for all procs to reach this point
	MPI_Barrier(MPI_COMM_WORLD);

	if(rank == 0){
		displayData = (unsigned char*) malloc(imgSize*sizeof(unsigned char));
		
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
	MPI_Barrier(MPI_COMM_WORLD); /* IMPORTANT */
	end = MPI_Wtime();
	
	if(rank == 0){
			cv::Mat dst = cv::Mat(cvSize(src.cols, src.rows), CV_8U, displayData);
			
			cv::namedWindow("final");
			cv::imshow("final", dst);

			cv::namedWindow("initial");
			cv::imshow("initial", src);
			
			printf("time: %.6fs\n", end -start);
			
			cv::waitKey();
	}
	MPI_Finalize();
}


int main(int ac, char* av[]){
	try{
		std::string address;
		int kernelSize;
		
		po::options_description desc("Allowed Options");
		desc.add_options()
			("help,h", "Produce help message")
			("file,f", po::value<std::string>(&address), "File to be processed")
			("kernelSize,k", po::value<int>(&kernelSize)->default_value(1), "Size of the kernel to apply")
			;
		
		po::variables_map vm;
		po::store(po::parse_command_line(ac, av, desc), vm);
		po::notify(vm);
		
		if(vm.count("file")){
			display(address, kernelSize);
		}
		else {//Includes help message
			std::cout << "This program applies a median filter to an image, and distributes the job between several processors and machines\n" << desc << "\n";
			return 0;
		}
	}
	catch(std::exception& e) {
		std::cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch(...) {
		std::cerr << "Exception of unknown type!\n";
	}
	return 0;
}