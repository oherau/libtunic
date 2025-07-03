#include "rune.h"
#include "toolbox.h"
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

Rune Rune::vowel_part() const
{
	return Rune(m_rune & RUNE_VOWEL);
}

Rune Rune::consonant_part() const
{
	return Rune(m_rune & RUNE_CONSONANT);
}

bool Rune::is_reverse() const
{
	return (m_rune & RUNE_REVERSE);
}

std::string Rune::to_string(unsigned long bin) const {

    switch (bin) {
        // VOWELS
	case RUNE_VOWEL_A: return "A";
	case RUNE_VOWEL_AR: return "AR";
	case RUNE_VOWEL_AH: return "AH";
	case RUNE_VOWEL_AY: return "AY";
	case RUNE_VOWEL_UH: return "UH";
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

	std::string pseudophonetic;
    if(reverse)
		pseudophonetic = std::string(Rune::to_string(vowel) + Rune::to_string(consonant));
	else
		pseudophonetic = std::string(Rune::to_string(consonant) + Rune::to_string(vowel));

	if (pseudophonetic.empty()) {
		return "???";
	}

	// return corrected pseudophonetic string
	return pseudophonetic;
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

bool Rune::generate_image(int x, int y, cv::Size2i size, int tickness, cv::Mat& output_image, bool draw_separator) const {


	if(draw_separator)
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
	const auto RUNE_SEGMENT_DETECTION_THRESHOLD = 0.50; // Threshold for segment detection
	bool result = true;
	auto height = rune_image.rows;
	auto width = rune_image.cols;
	m_rune = 0;

	cv::Mat binary_image;
	cv::threshold(rune_image, binary_image, 100, 255, cv::THRESH_BINARY);
	cv::imshow("Binary Image (Line Isolated)", binary_image);

	cv::waitKey(1000); // Wait for a key press to close the window
	cv::destroyAllWindows();

	int line_center_y = 0;
	int tickness = 0;
	if(!find_horizontal_separator(binary_image, line_center_y, tickness)) {
		std::cerr << "No horizontal separator found in the rune image." << std::endl;
		return false; // No separator line found
	}

	// 2. Calculate the origin point for the rune segments based on separator position
	// point E is the left point of the horizontal separator
	auto dx = 0 ;
	auto dy = line_center_y - (0.01 * RUNE_POINT_E.y * height) - (RUNE_SEGMENT_DRAW_DEFAULT_TICKNESS * width);

	// put original image to a bigger one with black background and borders
	// this is needed to avoid problems with matching the rune segments
	cv::Mat rune_image_with_borders(2*height, 2*width, CV_8UC1, cv::Scalar(0));
	cv::Rect paste_zone(0.5 * width + dx, 0.5 * height + dy, width, height);

	rune_image.copyTo(rune_image_with_borders(paste_zone)); // Copy source image into the ROI of the destination

	//cv::imshow("Result using copyTo()", rune_image_with_borders);
	//cv::waitKey(1000); // Wait for a key press to close the window


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
		rune_part.generate_image(0.5 * width, 0.5 * height, cv::Size2i(width, height), RUNE_SEGMENT_DETECTION_FILTER_MASK_TICKNESS* height, rune_filter_mask, false);
		rune_part.generate_image(0.5 * width, 0.5 * height, cv::Size2i(width, height), RUNE_SEGMENT_DETECTION_DETECTION_MASK_TICKNESS * height, rune_detection_mask, false);

		cv::Mat filtered_result, detection_result;
		cv::bitwise_and(rune_image_with_borders, rune_filter_mask, filtered_result, rune_filter_mask);
		cv::bitwise_and(filtered_result, rune_detection_mask, detection_result, rune_detection_mask);

		//int rune_filter_mask_white_pixel_count = cv::countNonZero(rune_filter_mask);
		int detection_mask_white_pixel_count = cv::countNonZero(rune_detection_mask);
		int match_white_pixel_count = cv::countNonZero(detection_result);
		double match = static_cast<double>(match_white_pixel_count) / static_cast<double>(detection_mask_white_pixel_count);

		if (match >= RUNE_SEGMENT_DETECTION_THRESHOLD) {
			std::cout << "Pattern detected!" << std::endl;
			m_rune |= rune_bit; // Set the corresponding bit in m_rune
		}

		//cv::imshow("rune_image", rune_image);
		cv::imshow("1-rune_image_with_borders", rune_image_with_borders);
		cv::imshow("2-rune_filter_mask", rune_filter_mask);
		cv::imshow("3-rune_detection_mask", rune_detection_mask);
		cv::imshow("4-filtered_result", filtered_result);
		cv::imshow("5-detection_result", detection_result);

		cv::waitKey(300); // Wait for a key press to close the window
		cv::destroyAllWindows();
	}
	 
	return (m_rune != 0);
}
