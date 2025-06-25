#ifndef __TOOLBOX_H__
#define __TOOLBOX_H__

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <filesystem>
namespace fs = std::filesystem;

const int RUNE_IMAGE_BINARY_FILTER_THRESOLD = 128; // Threshold for binary filter on rune images

bool find_horizontal_separator(const cv::Mat& binary_image, int& line_center_y, int& separator_tickness);
bool find_horizontal_separator_bounds(const cv::Mat& binary_image, int line_center_y, int& line_center_x_min, int& line_center_x_max);
bool crop_borders(const cv::Mat& image, int line_center_y, int line_center_x_min, int line_center_x_max, cv::Mat& cropped_image);
bool make_white_rune_black_background(cv::Mat& image);

cv::Mat applyErosion(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyDilation(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyOpening(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyClosing(const cv::Mat& input_image, int kernel_size, int iterations = 1);

std::vector<std::string> loadLinesFromFile(const fs::path& file);
bool partition_image(cv::Mat& image, std::vector<cv::Mat>& partition);

#endif // __TOOLBOX_H__