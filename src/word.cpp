#include "word.h"
#include "dictionary.h"
#include <opencv2/opencv.hpp>
#include <toolbox.h>

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
		if (c == DICT_RUNE_SEPARATOR || c == DICT_RUNE_SEPARATOR_2) {
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
	int rune_width = std::round(height * RUNE_DEFAULT_SIZE.aspectRatio());

	// resize image so the width is a perfect multiple of rune_width
	int nb_runes = std::round(double(width) / double(rune_width));


	cv::Rect roi_word(0, 0, std::min(nb_runes*rune_width, width), height);
	//cv::Mat resized_image(height, nb_runes*rune_width, CV_8UC1, cv::Scalar(0));
	cv::Mat region_to_copy = word_image(roi_word);
	region_to_copy = word_image(roi_word);

	cv::Mat resized_image;
	if(width < nb_runes * rune_width) {
		int dx = nb_runes * rune_width - width; // Calculate the right border width
		// If the image is smaller than the expected width, resize it
		cv::copyMakeBorder(word_image,
			resized_image,      // Output image
			0,                   // top border
			0,                   // bottom border
			0,                   // left border
			dx,  // right border
			cv::BORDER_CONSTANT, // Border type
			cv::Scalar(0));       // Border value
	} else {
		// If the image is larger or equal, just copy the region
		resized_image = word_image(roi_word).clone();
	}

	// Dilation
	int current_kernel_size = 5;
	int current_iterations = 1;
	cv::Mat processed_result = applyClosing(resized_image, current_kernel_size, current_iterations);
	cv::imshow("Cloased Image (dil the erosion) (Kernel: " + std::to_string(current_kernel_size) + ", Iterations: " + std::to_string(current_iterations) + ")", processed_result);



	m_runes.clear(); // Clear previous runes

	for (int i = 0; i < nb_runes; i++) {
		int x = i * rune_width;
		// Extract the rune image
		cv::Rect roi_rune(x, 0, rune_width, height);
		cv::Mat rune_image = processed_result(roi_rune);

		// Decode the rune from the image
		Rune rune;
		bool rune_detected = rune.decode_image(rune_image);

		// push back rune even if not decoded (will be display as ???)
		m_runes.push_back(rune);
		if(rune_detected) {
			//m_runes.push_back(rune);
		} else {
			std::cerr << "Failed to decode rune from image at position: " << x << std::endl;
			result = false; // If any rune fails to decode, set result to false
		}
	}

	return true;
	//return result;
}