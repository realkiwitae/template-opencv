#include <opencv2/opencv.hpp>
#include <chrono>
#include <unistd.h>
#include <iostream>

using namespace std;
using namespace std::chrono;

#define FPS 10
#define SLEEP_TIME 1000000/FPS
high_resolution_clock::time_point start;
high_resolution_clock::time_point start_move;
#define NOW high_resolution_clock::now()
#define TIME duration_cast<duration<double>>(NOW - start).count()
#define TIME_MOVE duration_cast<duration<double>>(NOW - start_move).count()

cv::VideoCapture cap;
cv::Mat frame = cv::Mat::zeros(1080, 1920, CV_8UC3);

void run(){
    cap >> frame;
    cv::imshow("video", frame);
    cv::waitKey(1);
}

int main(int argc, char** argv)
{
  start = NOW;
  std::cout << "-------" << std::endl;
  std::string videopath(argv[1]);
  cap = cv::VideoCapture(videopath);

  if(!cap.isOpened()){
    std::cout << "Cannot read video at " << videopath << std::endl;
    exit(0);
  }
  int fps = cap.get(cv::CAP_PROP_FPS);
  printf("IS OPENED %d %d\n",cap.isOpened(),fps);
  while(1){
    start = NOW;
    
    run();

    int us = std::max(0.,SLEEP_TIME - TIME*1e6);
    usleep(us);

  }

  std::cout << "-------" << std::endl;
  std::cout << TIME << std::endl;
}