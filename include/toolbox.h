#ifndef __TOOLBOX_H__
#define __TOOLBOX_H__

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>

bool find_horizontal_separator(const cv::Mat& binary_image, int& line_center_y, int& separator_tickness);
bool find_horizontal_separator_extremities(const cv::Mat& binary_image, int line_center_y, int& line_center_x_min, int& line_center_x_max);
bool crop_borders(const cv::Mat& image, int line_center_y, int line_center_x_min, int line_center_x_max, cv::Mat& cropped_image);


#endif // __TOOLBOX_H__