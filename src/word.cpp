#include "word.h"
#include "dictionary.h"
#include <opencv2/opencv.hpp>

Word::Word(const std::string& str)
{
	parse_runes(str, m_runes);
}

std::string Word::get_hash() const
{
	std::stringstream ss;
	bool firstRune = true;
	for(const auto& rune : m_runes) {
		if(!firstRune) {
			ss << DICT_RUNE_SEPARATOR;
		} else {
			firstRune = false;
		}
		ss << rune.to_hexa();
	}
	return std::string(ss.str());
}

std::string Word::to_pseudophonetic() const
{
	std::stringstream ss;
	bool firstRune = true;
	for(const auto& rune : m_runes) {
		if(!firstRune) {
			ss << DICT_RUNE_SEPARATOR;
		} else {
			firstRune = false;
		}
		ss << rune.to_pseudophonetic();
	}
	return std::string(ss.str());
}


bool Word::parse_runes(const std::string& str, std::vector<Rune>& runes)
{
	std::string strUpper(str);
	std::stringstream ss;
	auto counter = 0;
	for (const char c : strUpper) {
		if (c == DICT_RUNE_SEPARATOR) {
			continue;
		}
		ss << c;
		counter++;
		if (counter >= 4) {
			// 4 characters max per rune
			std::string ssStr = ss.str();
			Rune rune;
			rune.from_hexa(ssStr);
			runes.push_back(rune);
			ss.str(""); // clear the stringstream
			counter = 0;
		}
	}

	return true;
}


bool Word::generate_image(cv::Size2i rune_size, int tickness, cv::Mat& output_image) const {

	if (m_runes.empty()) {
		return false;
	}
	int height = rune_size.height + 2 * tickness;
	int width = rune_size.width * (int)m_runes.size() + 2 * tickness; // TODO: substract extra right margin (scaled)

	cv::Mat image(height, width, CV_8U, cv::Scalar(0));

	int x = tickness;
	int y = tickness;
	for (const auto& rune : m_runes) {
 		rune.generate_image(x, y, rune_size, tickness, image);
		x += rune_size.width; // TODO: substract right margin (scaled) to make right edge of current rune connected to the left edgde of next rune 

	}
	image.copyTo(output_image);
	return true;
}

bool Word::decode_image(const cv::Mat& word_image)
{
	bool result = true;
	auto height = word_image.rows;
	auto width = word_image.cols;
	int rune_width = height * 0.5f; // Assuming rune width is 60% of the height (to be verivied)

	m_runes.clear(); // Clear previous runes

	for (int x = 0; x <= (width-rune_width); x += rune_width) {
		// Extract the rune image
		cv::Rect roi(x, 0, rune_width, height);
		cv::Mat rune_image = word_image(roi);
		// Convert to grayscale and threshold
		cv::Mat gray_rune;
		cv::cvtColor(rune_image, gray_rune, cv::COLOR_BGR2GRAY);
		cv::threshold(gray_rune, gray_rune, 128, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

		// Decode the rune from the image
		Rune rune;
		bool rune_detected = rune.decode_image(gray_rune);
		if(rune_detected) {
			m_runes.push_back(rune);
		} else {
			std::cerr << "Failed to decode rune from image at position: " << x << std::endl;
			result = false; // If any rune fails to decode, set result to false
		}
	}

	return result;
}