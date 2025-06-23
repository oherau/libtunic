#include "toolbox.h"

bool find_horizontal_separator(const cv::Mat& binary_image, int& line_center_y, int& separator_tickness)
{

	// Find the Row with the Most White Pixels
	long long max_white_pixels_in_row = 0;
	int best_y_pos_top = -1; // Top-most y-coordinate of the thick line
	int best_y_pos_bottom = -1; // Bottom-most y-coordinate of the thick line
	double WHITE_SEP_PERCENT_THRESOLD = 0.95;

	// Iterate through each row of the binary image
	for (int y = 0; y < binary_image.rows; ++y) {

		// Get a pointer to the current row's data for efficiency
		const uchar* row_ptr = binary_image.ptr<uchar>(y);
		long long current_row_white_pixels = 0;

		// Count white pixels (where value is 255) in the current row
		for (int x = 0; x < binary_image.cols; ++x) {
			if (row_ptr[x] == 255) {
				current_row_white_pixels++;
			}
		}

		if (current_row_white_pixels > max_white_pixels_in_row) {
			max_white_pixels_in_row = current_row_white_pixels;
		}

		// If this row has a significant amount of white pixels (e.g., > 90% of width)
		// AND we haven't started detecting the line yet, mark it as the top.
		// This threshold (0.1) can be adjusted.
		if (current_row_white_pixels > (binary_image.cols * WHITE_SEP_PERCENT_THRESOLD) && best_y_pos_top == -1) {
			best_y_pos_top = y;
		}
		// If this row has fewer white pixels than the threshold AND we have started detecting,
		// it means we've passed the bottom of the line.
		if (current_row_white_pixels <= (binary_image.cols * WHITE_SEP_PERCENT_THRESOLD) && best_y_pos_top != -1 && best_y_pos_bottom == -1) {
			best_y_pos_bottom = y;
		}
	}

	// Handle case where line goes to the bottom of the image
	if (best_y_pos_top != -1 && best_y_pos_bottom == -1) {
		best_y_pos_bottom = binary_image.rows - 1;

		// no separation line found
	}

	separator_tickness = 0;
	line_center_y = -1;
	if (best_y_pos_top != -1 && best_y_pos_bottom != -1) {
		// Top-left Y of the line region
		int line_top_y = best_y_pos_top;
		// Height of the detected line region
		separator_tickness = best_y_pos_bottom - best_y_pos_top + 1;
		// Center Y position of the line
		line_center_y = line_top_y + separator_tickness / 2;

		// final check on line_center_y and separator_tickness


		// if ticknckness is greater than 20% of the image height, it is probably not a separator line
		if(separator_tickness > (0.2* binary_image.rows))
		{
			return false;
		}

		// if line center is not in the middle of the image, it is probably not a separator line
		if(line_center_y < (0.3 * binary_image.rows) || line_center_y > (0.7 * binary_image.rows))
		{
			return false;
		}

		// separator line found
		return true;
	}

	return false;
}

bool find_horizontal_separator_extremities(const cv::Mat& binary_image, int line_center_y, int& line_center_x_min, int& line_center_x_max)
{
	// --- Part 2: Find horizontal extremities (line_center_x_min, line_center_x_max) ---
	line_center_x_min = -1;
	line_center_x_max = -1;

	// We'll examine the row at line_center_y in the binary image.
	// For robustness, you could average over a few rows within the line_height.

	const uchar* scan_row_ptr = binary_image.ptr<uchar>(line_center_y);

	// Find the first white pixel (from left to right)
	for (int x = 0; x < binary_image.cols; ++x) {
		if (scan_row_ptr[x] == 255) {
			line_center_x_min = x;
			break; // Found the start, exit loop
		}
	}

	// Find the last white pixel (from right to left)
	for (int x = binary_image.cols - 1; x >= 0; --x) {
		if (scan_row_ptr[x] == 255) {
			line_center_x_max = x;
			break; // Found the end, exit loop
		}
	}

	return true;
}

bool crop_borders(const cv::Mat& image, int line_center_y, int line_center_x_min, int line_center_x_max, cv::Mat& cropped_image)
{
	// Calculate width and height from the bounds
	int crop_width = line_center_x_max - line_center_x_min + 1;
	int crop_height = image.rows;

	// 3. Create a cv::Rect object for the ROI
	cv::Rect roi(line_center_x_min, 0, crop_width, crop_height);

	// 5. Crop the image using the ROI
	// This creates a new cv::Mat that points to the data within the ROI.
	// It's a "shallow copy" - no new pixel data is allocated initially.
	cropped_image = image(roi);

	//// If you need a deep copy (a completely independent image), use clone():
	//cropped_image = image(roi).clone();

	return true;
}
