#include <string>
#include<iostream>
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



//sort the window using insertion sort
//insertion sort is best for this sorting
void insertionSort(int window[], int len)
{
    int temp, i , j;
    for(i = 0; i < len; i++){
        temp = window[i];
        for(j = i-1; j >= 0 && temp < window[j]; j--){
            window[j+1] = window[j];
        }
        window[j+1] = temp;
    }
}

void display(std::string imgAdd, int windowSize=2){
	int windowElements = (windowSize*2 + 1)*(windowSize*2 + 1);
	cv::Mat src, dst;
	
	// Load an image
	src = cv::imread(imgAdd, CV_LOAD_IMAGE_GRAYSCALE);
	int imgSize = src.rows*src.cols;
	int imgRows = src.rows;
	int imgCols = src.cols;

	if( !src.data )
	{ return; }

	dst = src.clone();

	int* window = (int*) malloc(windowElements*sizeof(int));
	for(int i = 0; i < src.rows*src.cols ; i++){

		int cnt = 0;
		// Pick up window element
		for(int u = -windowSize; u <= windowSize; u++){
			for(int v = -windowSize; v <= windowSize; v++){
				
				int row = (i + v + u*src.rows) % imgRows;
				int col = (i + v + u*src.rows) / imgRows;
				
				
				if( (row > 0) && (row < imgRows) && (col > 0) && (col < imgCols) ){
					window[cnt] = src.data[i + v + u*src.rows];
				}
				else{
					assert(cnt < windowsElements);
					window[cnt] = 128;
				}
				cnt++;
			}
		}

		// sort the window to find median
		std::partial_sort (window, window + windowSize/2 + 1, window + windowSize);

		// assign the median to centered element of the matrix
		dst.data[i] = window[windowSize/2];
	}

	cv::namedWindow("final");
	cv::imshow("final", dst);

	cv::namedWindow("initial");
	cv::imshow("initial", src);

	cv::waitKey();
}


int main(int ac, char* av[]){    
    try{
        po::options_description desc("Allowed Options");
        desc.add_options()
                ("help,h", "Produce help message")
                ("file,f", po::value<std::string>(), "File to be processed")
		("kernelSize,k", po::value<int>()->default_value(1), "Size of the kernel to apply")
        ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if(vm.count("file")){
            display(vm["file"].as<std::string>(), vm["kernelSize"].as<int>());
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