#include<iostream>
#include <algorithm>    // std::partial_sort

#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>


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

int main(){
	int windowSize = 20;
	int windowElements = (windowSize*2 + 1)*(windowSize*2 + 1);
	cv::Mat src, dst;
	
	// Load an image
	src = cv::imread("/home/emadrigal/Pictures/Lenna.png", CV_LOAD_IMAGE_GRAYSCALE);
	int imgSize = src.rows*src.cols;

	if( !src.data )
	{ return -1; }

	dst = src.clone();
	for(int y = 0; y < src.rows; y++)
		for(int x = 0; x < src.cols; x++)
			dst.at<uchar>(y,x) = 0.0;

	int* window = (int*) malloc(windowElements*sizeof(int));
	for(int i = 0; i < src.rows*src.cols ; i++){

		int cnt = 0;
		// Pick up window element
		for(int u = -windowSize; u <= windowSize; u++){
			for(int v = -windowSize; v <= windowSize; v++){
				if( ((i + v + u*src.rows) < imgSize) && ((i + v + u*src.rows) > 0)){
					window[cnt] = src.data[i + v + u*src.rows];
				}
				else{
					assert(cnt < windowsElements);
					window[cnt] = 0;
				}
				cnt++;
			}
		}

		// sort the window to find median
		std::partial_sort (window, window + windowSize/2, window + windowSize);

		// assign the median to centered element of the matrix
		dst.data[i] = window[windowSize/2];
	}

	cv::namedWindow("final");
	cv::imshow("final", dst);

	cv::namedWindow("initial");
	cv::imshow("initial", src);

	cv::waitKey();


	return 0;
}
