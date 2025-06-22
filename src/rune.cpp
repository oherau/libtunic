#include "rune.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <runedetector.h>

Rune::Rune(unsigned long bin) : m_rune(bin)
{}

Rune::Rune(const Rune& rune) : m_rune(rune.m_rune)
{}

std::string Rune::to_string(unsigned long bin) const {

    switch (bin) {
        // VOWELS
	case RUNE_VOWEL_A: return "A";
	case RUNE_VOWEL_AR: return "AR";
	case RUNE_VOWEL_AH: return "AH";
	case RUNE_VOWEL_AY: return "AY";
	case RUNE_VOWEL_E: return "E";
	case RUNE_VOWEL_EE: return "EE";
	case RUNE_VOWEL_EER: return "EER";
	case RUNE_VOWEL_EH: return "EH";
    case RUNE_VOWEL_ERE: return "ERE";
	case RUNE_VOWEL_I: return "I";
	case RUNE_VOWEL_IE: return "IE";
	case RUNE_VOWEL_IR: return "IR";
	case RUNE_VOWEL_OH: return "OH";
	case RUNE_VOWEL_OI: return "OI";
	case RUNE_VOWEL_OO: return "OO";
	case RUNE_VOWEL_OU: return "OU";
	case RUNE_VOWEL_OW: return "OW";
	case RUNE_VOWEL_ORE: return "ORE";
	case RUNE_CONSONANT_B: return "B";
	case RUNE_CONSONANT_CH: return "CH";
	case RUNE_CONSONANT_D: return "D";
	case RUNE_CONSONANT_F: return "F";
	case RUNE_CONSONANT_G: return "G";
	case RUNE_CONSONANT_H: return "H";
	case RUNE_CONSONANT_J: return "J";
	case RUNE_CONSONANT_K: return "K";
	case RUNE_CONSONANT_L: return "L";
	case RUNE_CONSONANT_M: return "M";
	case RUNE_CONSONANT_N: return "N";
	case RUNE_CONSONANT_NG: return "NG";
	case RUNE_CONSONANT_P: return "P";
	case RUNE_CONSONANT_R: return "R";
	case RUNE_CONSONANT_S: return "S";
	case RUNE_CONSONANT_SH: return "SH";
	case RUNE_CONSONANT_T: return "T";
	case RUNE_CONSONANT_THS: return "THS";
	case RUNE_CONSONANT_THZ: return "THZ";
	case RUNE_CONSONANT_V: return "V";
	case RUNE_CONSONANT_W: return "W";
	case RUNE_CONSONANT_Y: return "Y";
	case RUNE_CONSONANT_Z: return "Z";
	case RUNE_CONSONANT_ZH: return "ZH";
    }
    return "";
}

void Rune::draw_segment(const std::initializer_list<cv::Point2d>& segment, int tickness, int x, int y, cv::Size2i size, cv::Mat& output_image) const
{
	if (segment.size() < 2) return; // Ensure there are at least two points to draw a line
	// Create a vector of points from the initializer list
	std::vector<cv::Point2d> points(segment);
	// Scale the points based on the size provided
	for (auto& point : points) {
		point.x = x + point.x * size.width / 100; // Assuming points are in percentage
		point.y = y + point.y * size.height / 100; // Assuming points are in percentage
	}
	// Draw the line segment on the output image
	cv::line(output_image, points[0], points[1], cv::Scalar(255, 255, 255), tickness); // White color
}

void Rune::draw_circle(const std::initializer_list<cv::Point2d>& radius, int tickness, int x, int y, cv::Size2i size, cv::Mat& output_image) const
{
	if(radius.size() < 1) return; // Ensure there is at least one point to draw a circle
	// Create a vector of points from the initializer list
	std::vector<cv::Point2d> points(radius);
	// Scale the points based on the size provided
	for (auto& point : points) {
		point.x = x + point.x * size.width / 100; // Assuming points are in percentage
		point.y = y + point.y * size.height / 100; // Assuming points are in percentage
	}
	// Scale the radius based on the size provided
	double dx = points[0].x - points[1].x;
	double dy = points[0].y - points[1].y;
	auto radius_length = std::sqrt(dx * dx + dy * dy) - 0.5 * tickness;

	// Draw the circle on the output image
	cv::circle(output_image, points[0], radius_length, cv::Scalar(255, 255, 255), tickness); // White color with thickness of 2
}


std::string Rune::to_pseudophonetic() const {
    if(m_rune == RUNE_NULL)
        return " ";

    unsigned long consonant = RUNE_CONSONANT & m_rune;
    unsigned long vowel = RUNE_VOWEL & m_rune;
    bool reverse = RUNE_REVERSE & m_rune;

    if(reverse)
        return std::string(Rune::to_string(vowel) + Rune::to_string(consonant));
    return std::string(Rune::to_string(consonant) + Rune::to_string(vowel));
}

std::string Rune::to_hexa() const
{
    std::stringstream ss;
    ss << std::hex // Set hexadecimal base
        << std::setw(4) // Set width to 4 characters
        << std::setfill('0') // Pad with leading zeros if less than 4 chars
        << (m_rune & 0xFFFFUL); // Mask to ensure only the lower 16 bits are considered
    // This effectively truncates to 4 hex digits if value is larger
    return ss.str();
}

bool Rune::from_hexa(const std::string& hexString)
{
    std::stringstream ss(hexString);
    unsigned long value;

    // Attempt to read as hexadecimal
    ss >> std::hex >> value;

    // Check for errors:
    // 1. ss.fail(): If the conversion failed (e.g., non-hex characters found)
    // 2. !ss.eof(): If there are characters remaining in the stream after parsing (partial parse)
    if (ss.fail() || !ss.eof()) {
        return false; // Return empty optional on failure
    }

    m_rune = value; // Set the rune value
    return true;
}

bool Rune::generate_image(int x, int y, cv::Size2i size, int tickness, cv::Mat& output_image) const {


	// alway draw the separator segment
	draw_segment(RUNE_SEGMENT_SEP, tickness, x, y, size, output_image);

	if (m_rune & (0x1 << 0)) draw_segment(RUNE_SEGMENT_01, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 1)) draw_segment(RUNE_SEGMENT_02, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 2)) draw_segment(RUNE_SEGMENT_03, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 3)) draw_segment(RUNE_SEGMENT_04, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 4)) draw_segment(RUNE_SEGMENT_05, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 5)) draw_segment(RUNE_SEGMENT_06, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 7)) draw_segment(RUNE_SEGMENT_08, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 8)) draw_segment(RUNE_SEGMENT_09, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 9)) draw_segment(RUNE_SEGMENT_10, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 10)) draw_segment(RUNE_SEGMENT_11, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 11)) draw_segment(RUNE_SEGMENT_12, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 12)) draw_segment(RUNE_SEGMENT_13, tickness, x, y, size, output_image);
	if (m_rune & (0x1 << 13)) draw_segment(RUNE_SEGMENT_14, tickness, x, y, size, output_image);

	if (m_rune & (0x1 << 15)) draw_circle(RUNE_RADIUS_16, tickness, x, y, size, output_image);

	return true;
}


bool Rune::decode_image(const cv::Mat& rune_image)
{
	bool result = true;
	auto height = rune_image.rows;
	auto width = rune_image.cols;
	m_rune = 0;

	cv::Mat binary_image;
	cv::threshold(rune_image, binary_image, 100, 255, cv::THRESH_BINARY);
	cv::imshow("Binary Image (Line Isolated)", binary_image);

	//cv::waitKey(00); // Wait for a key press to close the window
	//cv::destroyAllWindows();

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

	int line_height = 0;
	int line_center_y = 0;
	if (best_y_pos_top != -1 && best_y_pos_bottom != -1) {
		// Top-left Y of the line region
		int line_top_y = best_y_pos_top;
		// Height of the detected line region
		line_height = best_y_pos_bottom - best_y_pos_top + 1;
		// Center Y position of the line
		line_center_y = line_top_y + line_height / 2;
	}


	// 2. Calculate the origin point for the rune segments based on separator position
	// point E is the left point of the horizontal separator
	auto dx = 0 ;
	auto dy = line_center_y - (0.01 * RUNE_POINT_E.y * height) - (RUNE_SEGMENT_DEFAULT_TICKNESS * width);

	// put original image to a bigger one with black background and borders
	// this is needed to avoid problems with matching the rune segments
	cv::Mat rune_image_with_borders(2*height, 2*width, CV_8UC1, cv::Scalar(0));
	cv::Rect paste_zone(0.5 * width + dx, 0.5 * height + dy, width, height);

	rune_image.copyTo(rune_image_with_borders(paste_zone)); // Copy source image into the ROI of the destination

	//cv::imshow("Result using copyTo()", rune_image_with_borders);
	//cv::waitKey(0); // Wait for a key press to close the window


 	for(int shift = 0; shift < 16; ++shift) {

		if(shift == 6 || shift == 14) {
			// no segments for these bits
			continue;
		}

		// generate a segment mask (greater tickeness for better detection)
		cv::Mat rune_filter_mask(2*height, 2*width, CV_8UC1, cv::Scalar(0));
		cv::Mat rune_detection_mask(2*height, 2*width, CV_8UC1, cv::Scalar(0));
		unsigned long rune_bit = (0x1 << shift);
		Rune rune_part = Rune(rune_bit);
		rune_part.generate_image(0.5 * width, 0.5 * height, cv::Size2i(width, height), RUNE_SEGMENT_DEFAULT_TICKNESS * height, rune_filter_mask);
		rune_part.generate_image(0.5 * width, 0.5 * height, cv::Size2i(width, height), RUNE_SEGMENT_DEFAULT_TICKNESS * height, rune_detection_mask);

		//cv::imshow("rune_filter_mask", rune_filter_mask);
		//cv::imshow("rune_detection_mask", rune_detection_mask);
		//cv::waitKey(300); // Wait for a key press to close the window
		//cv::destroyAllWindows();

		cv::Mat filtered_result;
		cv::bitwise_and(rune_image_with_borders, rune_image_with_borders, filtered_result, rune_filter_mask);
		//cv::imshow("filtered_result", filtered_result);

		cv::Mat result_match;
		cv::matchTemplate(filtered_result, rune_detection_mask, result_match, cv::TM_CCOEFF_NORMED);

		double minVal, maxVal;
		cv::Point minLoc, maxLoc;
		cv::minMaxLoc(result_match, &minVal, &maxVal, &minLoc, &maxLoc);

		// The top-left corner of the best match
		cv::Point matchLoc = maxLoc; // For TM_CCOEFF_NORMED, maxVal indicates best match

		std::cout << "Best match correlation: " << maxVal << std::endl;
		std::cout << "Best match location (top-left): " << matchLoc << std::endl;


		if(maxVal >= RUNE_DETECTION_THRESHOLD) {
			std::cout << "Pattern detected!" << std::endl;
			m_rune |= rune_bit; // Set the corresponding bit in m_rune
		}


		cv::imshow("rune_filter_mask", rune_filter_mask);
		cv::imshow("rune_detection_mask", rune_detection_mask);
		cv::imshow("rune_image", rune_image);
		cv::imshow("rune_image_with_borders", rune_image_with_borders);
		cv::imshow("rune_filter_mask", rune_filter_mask);
		cv::imshow("rune_detection_mask", rune_detection_mask);

		cv::imshow("filtered_result", filtered_result);

		cv::waitKey(300); // Wait for a key press to close the window
		cv::destroyAllWindows();
	}

	return (m_rune != 0);
}
