#ifndef YOLOV5_MODEL_GEN_COLOR_H_
#define YOLOV5_MODEL_GEN_COLOR_H_

#include <opencv4/opencv2/opencv.hpp>
#include <vector>

//将HSV颜色空间值转换成RGB值，参考这里
cv::Scalar HSV2RGB(const float h, const float s, const float v) {
  const int h_i = static_cast<int>(h * 6);
  const float f = h * 6 - h_i;
  const float p = v * (1 - s);
  const float q = v * (1 - f * s);
  const float t = v * (1 - (1 - f) * s);
  float r, g, b;
  switch (h_i) {
    case 0:r = v;
      g = t;
      b = p;
      break;
    case 1:r = q;
      g = v;
      b = p;
      break;
    case 2:r = p;
      g = v;
      b = t;
      break;
    case 3:r = p;
      g = q;
      b = v;
      break;
    case 4:r = t;
      g = p;
      b = v;
      break;
    case 5:r = v;
      g = p;
      b = q;
      break;
    default:r = 1;
      g = 1;
      b = 1;
      break;
  }
  return
      cv::Scalar(r * 255, g * 255, b * 255);
}
//固定s和v值，改变h值，并通过黄金分割得到随机均匀的h值，从而得到较理想的随机颜色
std::vector<cv::Scalar> GetColors(const int n) {
  std::vector<cv::Scalar> colors;
  cv::RNG rng(0);
  const float golden_ratio_conjugate = 0.618033988749895;
  const float s = 0.3;
  const float v = 0.99;
  for (int i = 0; i < n; ++i) {
    const float h = std::fmod(rng.uniform(0.f, 1.f) + golden_ratio_conjugate,
                              1.f);//始终返回（0，1）区间的小数，取余fmod(a,b)=a-n*b(n为最大整除得到的整数商)商的符号取决于a
    colors.push_back(HSV2RGB(h, s, v));
  }
  return colors;
}

#endif //YOLOV5_MODEL_GEN_COLOR_H_
