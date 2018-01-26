#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "common/types.h"
using namespace std;
using cv::resize;
using cv::cvtColor;
using cv::findContours;
using cv::approxPolyDP;
using cv::imread;
using cv::Mat;
using cv::adaptiveThreshold;
using cv::COLOR_BGR2GRAY;
using cv::RETR_EXTERNAL;
using cv::CHAIN_APPROX_SIMPLE;
using cv::ADAPTIVE_THRESH_GAUSSIAN_C;
using cv::THRESH_BINARY_INV;
using cv::arcLength;
using cv::contourArea;
using cv::Size;

void find_coordinates(const char* input, ListOfShapes* const list){

  Mat src, src_gray;
  //const char* source_window = "Source image";
  //const char* corners_window = "Corners detected";

  src = imread(input);
  if (src.empty()) {
    cout << "Could not open or find the image!\n" << endl;
    return;
  }
  Mat src_processed;

  double scale = 800.0 / src.size().width;
  resize(src, src_processed, Size(scale * src.size().width, scale * src.size().height));
  cvtColor(src_processed, src_gray, COLOR_BGR2GRAY);
  adaptiveThreshold(src_gray, src_gray, 100, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, 11, 2);
  //GaussianBlur(src_gray, src_gray, Size(5, 5), 0, 0);
  //Mat cannised;
  //Canny(src_gray, cannised, 0, 250, 5);

  vector< vector<cv::Point> > contours;
  findContours(src_gray, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  vector<cv::Point> approx;
  //Mat src_gray_display = src_gray.clone();



  for (unsigned int i = 0; i < contours.size(); ++i) {
    approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true) * 0.02, true);
    if (fabs(contourArea(contours[i])) < 50 || approx.size() % 2 == 1) {
      continue;
    }

    std::cout << "New Contour: " << std::endl;
    ListOfPoints* shapeList = new ListOfPoints();

    for (unsigned int j = 0; j < approx.size(); ++j) {
      std::cout << approx[j] << std::endl;
      shapeList->push_back(Point(approx[j].x,approx[j].y));

		}
    list->push_back(shapeList);
	}
  for (uint i = 0; i < list->size(); i++) {
    std::cout << "Shape " << i << " : ";
    ListOfPoints* shapeList = (*list)[i];
    for (uint j = 0; j < shapeList->size(); j++) {
      std::cout << "(" << (*shapeList)[j].x << ", " << (*shapeList)[j].y << ")   ";
    }
      std::cout << std::endl;
  }

}

void debug_coordinates(const char* filename, ListOfShapes* const list){}
