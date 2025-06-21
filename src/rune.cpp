#include "rune.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <opencv2/imgproc.hpp>

Rune::Rune(unsigned long bin) : m_rune(bin)
{}

Rune::Rune(const Rune& rune) : m_rune(rune.m_rune)
{}
//
//Rune::Rune(std::string name)
//{
//         if (name == "RUNE_VOWEL_A")     m_rune = RUNE_VOWEL_A;
//    else if (name == "RUNE_VOWEL_I")     m_rune = RUNE_VOWEL_I;
//    else if (name == "RUNE_VOWEL_E")     m_rune = RUNE_VOWEL_E;
//    else if (name == "RUNE_VOWEL_OO_O")  m_rune = RUNE_VOWEL_OO_O;
//    else if (name == "RUNE_VOWEL_ER")    m_rune = RUNE_VOWEL_ER;
//    else if (name == "RUNE_VOWEL_EE")    m_rune = RUNE_VOWEL_EE;
//    else if (name == "RUNE_VOWEL_OO_U")  m_rune = RUNE_VOWEL_OO_U;
//    else if (name == "RUNE_VOWEL_IR")    m_rune = RUNE_VOWEL_IR;
//    else if (name == "RUNE_VOWEL_OOR")   m_rune = RUNE_VOWEL_OOR;
//    else if (name == "RUNE_VOWEL_AR")    m_rune = RUNE_VOWEL_AR;
//    else if (name == "RUNE_VOWEL_ERE")   m_rune = RUNE_VOWEL_ERE;
//    else if (name == "RUNE_VOWEL_AI")    m_rune = RUNE_VOWEL_AI;
//    else if (name == "RUNE_VOWEL_Y")     m_rune = RUNE_VOWEL_Y;
//    else if (name == "RUNE_VOWEL_U")     m_rune = RUNE_VOWEL_U;
//    else if (name == "RUNE_VOWEL_OW_H")  m_rune = RUNE_VOWEL_OW_H;
//    else if (name == "RUNE_VOWEL_OW_S")  m_rune = RUNE_VOWEL_OW_S;
//    else if (name == "RUNE_VOWEL_AIR")   m_rune = RUNE_VOWEL_AIR;
//    // Consonants
//    else if (name == "RUNE_CONSONANT_B")     m_rune = RUNE_CONSONANT_B;
//    else if (name == "RUNE_CONSONANT_CH")    m_rune = RUNE_CONSONANT_CH;
//    else if (name == "RUNE_CONSONANT_D")     m_rune = RUNE_CONSONANT_D;
//    else if (name == "RUNE_CONSONANT_F")     m_rune = RUNE_CONSONANT_F;
//    else if (name == "RUNE_CONSONANT_G")     m_rune = RUNE_CONSONANT_G;
//    else if (name == "RUNE_CONSONANT_H")     m_rune = RUNE_CONSONANT_H;
//    else if (name == "RUNE_CONSONANT_J")     m_rune = RUNE_CONSONANT_J;
//    else if (name == "RUNE_CONSONANT_K")     m_rune = RUNE_CONSONANT_K;
//    else if (name == "RUNE_CONSONANT_L")     m_rune = RUNE_CONSONANT_L;
//    else if (name == "RUNE_CONSONANT_M")     m_rune = RUNE_CONSONANT_M;
//    else if (name == "RUNE_CONSONANT_N")     m_rune = RUNE_CONSONANT_N;
//    else if (name == "RUNE_CONSONANT_NG")    m_rune = RUNE_CONSONANT_NG;
//    else if (name == "RUNE_CONSONANT_P")     m_rune = RUNE_CONSONANT_P;
//    else if (name == "RUNE_CONSONANT_R")     m_rune = RUNE_CONSONANT_R;
//    else if (name == "RUNE_CONSONANT_S")     m_rune = RUNE_CONSONANT_S;
//    else if (name == "RUNE_CONSONANT_SH")    m_rune = RUNE_CONSONANT_SH;
//    else if (name == "RUNE_CONSONANT_T")     m_rune = RUNE_CONSONANT_T;
//    else if (name == "RUNE_CONSONANT_TH_S")  m_rune = RUNE_CONSONANT_TH_S;
//    else if (name == "RUNE_CONSONANT_TH_Z")  m_rune = RUNE_CONSONANT_TH_Z;
//    else if (name == "RUNE_CONSONANT_V")     m_rune = RUNE_CONSONANT_V;
//    else if (name == "RUNE_CONSONANT_W")     m_rune = RUNE_CONSONANT_W;
//    else if (name == "RUNE_CONSONANT_Y")     m_rune = RUNE_CONSONANT_Y;
//    else if (name == "RUNE_CONSONANT_Z")     m_rune = RUNE_CONSONANT_Z;
//	else if (name == "RUNE_CONSONANT_ZH")    m_rune = RUNE_CONSONANT_ZH;
//}

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

