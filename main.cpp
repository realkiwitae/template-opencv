#include <opencv2/opencv.hpp>
#include <chrono>
#include <unistd.h>
#include <iostream>
#include <opencv2/tracking.hpp>
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace std::chrono;

#define FPS 10
#define SLEEP_TIME 1000000/FPS
high_resolution_clock::time_point start;
high_resolution_clock::time_point start_move;
#define NOW high_resolution_clock::now()
#define TIME duration_cast<duration<double>>(NOW - start).count()
#define TIME_MOVE duration_cast<duration<double>>(NOW - start_move).count()

cv::Point point = cv::Point(0, 0);
cv::Mat pasting_item;
cv::Mat image;

void onMouse(int event, int x, int y, int, void*){
	if ( event != cv::EVENT_LBUTTONDOWN )return;

    point = cv::Point(x, y);

    // COPY paste_item but only nonwhite pixels
    pasting_item.copyTo(image(cv::Rect(point.x, point.y, pasting_item.cols, pasting_item.rows)), pasting_item);
    
    cv::destroyWindow("pasting");

}

int main(int argc, char** argv)
{
  start = NOW;
  std::cout << "-------" << std::endl;

  // resize image width to 1280 but keep aspect ratio:
  int width = 1280;
  std::string output_file = "output.jpg";
// rewrite main function so it takes arguments -i input -o output -w width
  if (argc > 1) {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-i") == 0) {
        image = cv::imread(argv[i + 1]);
      }
      if (strcmp(argv[i], "-o") == 0) {
        output_file = argv[i + 1];
      }
      if (strcmp(argv[i], "-w") == 0) {
        width = atoi(argv[i + 1]);
      }
    }
  }
  // add usage in console if no arguments
  if (argc == 1) {
    std::cout << "usage: ./main -i input.jpg -o output.jpg -w 1280" << std::endl;
    exit(0);
  }

  int height = (int) ((double) image.rows / (double) image.cols * width);
  cv::resize(image, image, cv::Size(width, height));

  // show the image and allow click to select a region store region in rectangle
  cv::Rect roi_toclear = cv::selectROI(image);
  //cv::destroyWindow("select");

  cv::Mat mask_toclear = cv::Mat::zeros(image.size(), CV_8U);
  cv::rectangle(mask_toclear, roi_toclear, cv::Scalar(255), cv::FILLED);
  cv::inpaint(image, mask_toclear, image, 3, cv::INPAINT_TELEA);


  cv::Rect roi_topaste = cv::selectROI(image);
  //cv::destroyWindow("select");

  // the rectangle part has a background deect and make it transparent
  cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);

  cv::Mat bgdModel, fgdModel;

  // get foreground mask
  cv::grabCut(image, mask, roi_topaste, bgdModel, fgdModel, 1, cv::GC_INIT_WITH_RECT);
  cv::compare(mask, cv::GC_PR_FGD, mask, cv::CMP_EQ);
 
  // extract selected area from the image without background it should be of the ROI size
  cv::Mat foreground(image.size(), CV_8UC4, cv::Scalar(255, 255, 255,255));
  image.copyTo(foreground, mask);

  pasting_item = foreground(roi_topaste);
  
  pasting_item.copyTo(image(cv::Rect(roi_toclear.x, roi_toclear.y, pasting_item.cols, pasting_item.rows)), pasting_item);

  // reduce image contrast a little keeping colors
  cv::Mat new_image = cv::Mat::zeros( image.size(), image.type() );
  double alpha = 0.5; /*< Simple contrast control */
  int beta = 0;       /*< Simple brightness control */
  for( int y = 0; y < image.rows; y++ ) {
      for( int x = 0; x < image.cols; x++ ) {
          for( int c = 0; c < 3; c++ ) {
              new_image.at<cv::Vec3b>(y,x)[c] =
                cv::saturate_cast<uchar>( alpha*( image.at<cv::Vec3b>(y,x)[c] ) + beta );
          }
      }
  }
  
  cv::imshow("new_image", new_image);
  cv::waitKey(0);

  // save to outputfile
  cv::imwrite(output_file, new_image);

  std::cout << "-------" << std::endl;
  std::cout << TIME << std::endl;
}