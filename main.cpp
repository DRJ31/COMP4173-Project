#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <dirent.h>
#ifdef WIN32
#include <io.h>
#endif
#include <opencv2/opencv.hpp>

#define IMG_PATH "img"

using namespace cv;
using namespace std;


/* Get all the images in the directory */
vector<Mat> getFileNames(const char* path)
{
  vector<Mat> images;
  #ifdef linux
  DIR *dir = opendir(path);
  dirent *p = NULL;
  while((p = readdir(dir)) != NULL) {
    if (p->d_name[0] != '.') {
      string name = "img/" + string(p->d_name);
      images.push_back(imread(name));
    }
  }
  closedir(dir);
  #endif

  #ifdef WIN32
  _finddata_t file;
  long lf;
  while(_findnext(lf, &file) == 0) {
    if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
      continue;
    images.push_back(imread(file.name));
  }
  _findclose(lf);
  #endif
  return images;
}

bool sortByArea(Mat a, Mat b) {
  return contourArea(a) < contourArea(b);
}

void fusion(vector<Mat> files)
{
  Mat dst, gray, thresh, mask, cnt, minRect, sub;
  vector<Mat> cnts;
  Rect rect;

  // Stitch images
  Stitcher *stitcher = createStitcher();
  Stitcher::Status status = stitcher->stitch(files, dst);
  if (status != Stitcher::Status::OK) {
    cout << "Failed to stitch image" << endl;
    return;
  }

  // Make borders
  copyMakeBorder(dst, dst, 10, 10, 10, 10, BORDER_CONSTANT, 0);
  cvtColor(dst, gray, COLOR_BGR2GRAY);
  threshold(gray, thresh, 0, 255, THRESH_BINARY);
  
  findContours(thresh, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  std::sort(cnts.begin(), cnts.end(), sortByArea);
  cnt = cnts.back();

  mask = Mat(thresh.size(), thresh.type());
  rect = boundingRect(cnt);
  rectangle(mask, rect, 255, -1);

  minRect = mask, sub = mask;
  while (countNonZero(sub) > 0) {
    erode(minRect, minRect, noArray());
    subtract(minRect, thresh, sub);
  }

  findContours(minRect, cnts, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  std::sort(cnts.begin(), cnts.end(), sortByArea);
  cnt = cnts.back();
  rect = boundingRect(cnt);
  dst = dst(rect);

  imshow("Fusion Image", dst);
  waitKey(0);
}

int main()
{
  vector<Mat> files = getFileNames(IMG_PATH);
  fusion(files);
  return 0;
}