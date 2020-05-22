#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <dirent.h>
#include <opencv2/opencv.hpp>

#define IMG_PATH "second"

using namespace cv;
using namespace std;


vector<Mat> images;


/* Get all the images in the directory */
void getFileNames(const char* path)
{
  DIR *dir = opendir(path);
  dirent *p = NULL;
  while((p = readdir(dir)) != NULL) {
    if (p->d_name[0] != '.') {
      string name = path + string("/") + string(p->d_name);
      cout << name << endl;
      Mat tmp = imread(name);
      images.push_back(tmp.clone());
    }
  }
  closedir(dir);
}

bool sortByArea(Mat a, Mat b) {
  return contourArea(a) < contourArea(b);
}

void fusion(vector<Mat> files)
{
  Mat dst, gray, thresh, mask, cnt, minRect, sub;
  vector<Mat> cnts;
  Rect rect;
  Stitcher stitcher = Stitcher::createDefault(false);
  Stitcher::Status status = stitcher.stitch(files, dst);
  if (status != Stitcher::Status::OK) {
    cout << "Failed to stitch image" << endl;
    return;
  }

  // Make borders
  copyMakeBorder(dst, dst, 10, 10, 10, 10, BORDER_CONSTANT, 0);
  cvtColor(dst, gray, COLOR_BGR2GRAY);
  threshold(gray, thresh, 0, 255, THRESH_BINARY);
  
  findContours(thresh.clone(), cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  std::sort(cnts.begin(), cnts.end(), sortByArea);
  cnt = cnts.back();

  mask = Mat(thresh.size(), thresh.type());
  rect = boundingRect(cnt);
  rectangle(mask, rect, 255, -1);
  
  minRect = mask.clone(), sub = mask.clone();
  while (countNonZero(sub) > 0) {
    erode(minRect, minRect, noArray());
    subtract(minRect, thresh, sub);
  }

  findContours(minRect.clone(), cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  std::sort(cnts.begin(), cnts.end(), sortByArea);
  cnt = cnts.back();
  rect = boundingRect(cnt);
  dst = dst(rect);

  imshow("final.jpg", dst);
  waitKey(0);
}

int main()
{
  getFileNames(IMG_PATH);
  fusion(images);
  return 0;
}