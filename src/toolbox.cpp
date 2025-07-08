#include <opencv2/imgproc.hpp>
#include "toolbox.h"
#include "wavgenerator.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>   // For sin and M_PI
#include <numeric> // For std::iota (C++11+)
#include <map>     // For std::map
#include <string>  // For std::string in main's debug output
#include <cstdint> // For uint16_t, uint32_t, int16_t etc.
#include <sstream> // Required for std::ostringstream and std::istringstream
#include <opencv2/opencv.hpp> // For cv::Mat, cv::Size, cv::resize
#include <algorithm>          // For std::min

// Define PI if not available (M_PI is a common GNU extension)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool find_horizontal_separator(const cv::Mat& binary_image, int& line_center_y, int& separator_tickness)
{

	// Find the Row with the Most White Pixels
	long long max_white_pixels_in_row = 0;
	int best_y_pos_top = -1; // Top-most y-coordinate of the thick line
	int best_y_pos_bottom = -1; // Bottom-most y-coordinate of the thick line
	double WHITE_SEP_PERCENT_THRESOLD = 0.75;

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
		if (separator_tickness > (0.2 * binary_image.rows))
		{
			return false;
		}

		// if line center is not in the middle of the image, it is probably not a separator line
		if (line_center_y < (0.3 * binary_image.rows) || line_center_y >(0.7 * binary_image.rows))
		{
			return false;
		}

		// separator line found
		return true;
	}

	return false;
}

bool find_horizontal_separator_bounds(const cv::Mat& binary_image, int line_center_y, int& line_center_x_min, int& line_center_x_max)
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

bool make_white_rune_black_background(cv::Mat& image) {
	if (image.empty()) {
		std::cerr << "Error: Input image is empty." << std::endl;
		return false;
	}
	// Convert the image to grayscale if it is not already
	if (image.channels() == 3) {
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
	}
	// Threshold the image to create a binary mask
	cv::Mat binary_image;
	cv::threshold(image, binary_image, 128, 255, cv::THRESH_BINARY);

	auto white_pixels = cv::countNonZero(binary_image);
	auto black_pixels = binary_image.total() - white_pixels;

	if (white_pixels > black_pixels) {
		// If there are more white pixels than black, apply threshold and invert the image
		cv::threshold(image, binary_image, RUNE_IMAGE_BINARY_FILTER_THRESOLD, 255, cv::THRESH_BINARY);
		cv::bitwise_not(binary_image, binary_image);
	}
	else {
		// If there are more black pixels than white, apply threshold without inversion
		cv::threshold(image, binary_image, 255 - RUNE_IMAGE_BINARY_FILTER_THRESOLD, 255, cv::THRESH_BINARY);
	}

	image = binary_image; // Update the original image with the binary mask

	return true;
}

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string> // For std::to_string
#include <rune.h>
#include <note.h>
#include "runedictionary.h"
#include <dictionary.h>

// Function to apply Erosion
// input_image: The source image (should be grayscale or binary for typical use)
// kernel_size: The size of the structuring element (e.g., 3 for 3x3, 5 for 5x5)
// iterations: The number of times the erosion operation is applied
cv::Mat applyErosion(const cv::Mat& input_image, int kernel_size, int iterations) {
	if (input_image.empty()) {
		std::cerr << "Error in applyErosion: Input image is empty." << std::endl;
		return cv::Mat(); // Return an empty Mat
	}

	// Ensure kernel_size is positive and odd for a centered kernel
	if (kernel_size <= 0) {
		std::cerr << "Warning: kernel_size must be positive. Using 3." << std::endl;
		kernel_size = 3;
	}
	if (kernel_size % 2 == 0) {
		std::cerr << "Warning: kernel_size should ideally be odd for symmetric kernels. Increasing by 1." << std::endl;
		kernel_size += 1;
	}

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(kernel_size, kernel_size),
		cv::Point(kernel_size / 2, kernel_size / 2));
	cv::Mat eroded_image;
	cv::erode(input_image, eroded_image, kernel, cv::Point(-1, -1), iterations);
	return eroded_image;
}

// Function to apply Dilation
// input_image: The source image
// kernel_size: The size of the structuring element
// iterations: The number of times the dilation operation is applied
cv::Mat applyDilation(const cv::Mat& input_image, int kernel_size, int iterations) {
	if (input_image.empty()) {
		std::cerr << "Error in applyDilation: Input image is empty." << std::endl;
		return cv::Mat();
	}

	if (kernel_size <= 0) {
		std::cerr << "Warning: kernel_size must be positive. Using 3." << std::endl;
		kernel_size = 3;
	}
	if (kernel_size % 2 == 0) {
		std::cerr << "Warning: kernel_size should ideally be odd for symmetric kernels. Increasing by 1." << std::endl;
		kernel_size += 1;
	}

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(kernel_size, kernel_size),
		cv::Point(kernel_size / 2, kernel_size / 2));
	cv::Mat dilated_image;
	cv::dilate(input_image, dilated_image, kernel, cv::Point(-1, -1), iterations);
	return dilated_image;
}

// Function to apply Opening (Erosion then Dilation)
cv::Mat applyOpening(const cv::Mat& input_image, int kernel_size, int iterations) {
	if (input_image.empty()) {
		std::cerr << "Error in applyOpening: Input image is empty." << std::endl;
		return cv::Mat();
	}

	if (kernel_size <= 0) {
		std::cerr << "Warning: kernel_size must be positive. Using 3." << std::endl;
		kernel_size = 3;
	}
	if (kernel_size % 2 == 0) {
		std::cerr << "Warning: kernel_size should ideally be odd for symmetric kernels. Increasing by 1." << std::endl;
		kernel_size += 1;
	}

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(kernel_size, kernel_size),
		cv::Point(kernel_size / 2, kernel_size / 2));
	cv::Mat opened_image;
	cv::morphologyEx(input_image, opened_image, cv::MORPH_OPEN, kernel, cv::Point(-1, -1), iterations);
	return opened_image;
}

// Function to apply Closing (Dilation then Erosion)
cv::Mat applyClosing(const cv::Mat& input_image, int kernel_size, int iterations) {
	if (input_image.empty()) {
		std::cerr << "Error in applyClosing: Input image is empty." << std::endl;
		return cv::Mat();
	}

	if (kernel_size <= 0) {
		std::cerr << "Warning: kernel_size must be positive. Using 3." << std::endl;
		kernel_size = 3;
	}
	if (kernel_size % 2 == 0) {
		std::cerr << "Warning: kernel_size should ideally be odd for symmetric kernels. Increasing by 1." << std::endl;
		kernel_size += 1;
	}

	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(kernel_size, kernel_size),
		cv::Point(kernel_size / 2, kernel_size / 2));
	cv::Mat closed_image;
	cv::morphologyEx(input_image, closed_image, cv::MORPH_CLOSE, kernel, cv::Point(-1, -1), iterations);
	return closed_image;
}

std::vector<std::string> loadLinesFromFile(const fs::path& filename) {
	std::vector<std::string> lines; // Vector to store the lines
	std::ifstream inputFile(filename); // Open the file for reading

	// Check if the file was opened successfully
	if (!inputFile.is_open()) {
		std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
		return lines; // Return an empty vector on failure
	}

	std::string line;
	// Read lines one by one until the end of the file is reached
	while (std::getline(inputFile, line)) {
		lines.push_back(line); // Add the read line to the vector
	}

	inputFile.close(); // Close the file
	return lines;      // Return the vector of lines
}

bool partition_image(cv::Mat& image, std::vector<cv::Rect>& partition)
{
	partition.clear();

	if (image.empty()) {
		std::cerr << "Error: Could not open or find the image." << std::endl;
		return false;
	}

	// 1. Convert to grayscale
	cv::Mat gray_image;
	if (image.channels() == 3) {
		// If the image is in color (3 channels), convert it to grayscale
		cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
	}
	else if (image.channels() == 4) {
		// If the image has an alpha channel (4 channels), convert it to grayscale
		cv::cvtColor(image, gray_image, cv::COLOR_BGRA2GRAY);
	}
	else {
		// If the image is already grayscale, just use it as is
		gray_image = image;
	}


	//// 2. Invert the image (so black lines become white, and blocks become black)
	//// This is useful for finding contours or connected components where objects are white on a black background
	//cv::Mat inverted_image;
	//cv::bitwise_not(gray_image, inverted_image);

	// 3. Threshold the image to make the black lines very clear (white) and blocks black
	// Adjust the threshold value (e.g., 50) based on how dark your "black" lines are.
	// If the lines are perfectly black, a low threshold like 10-20 might work.
	// If they are dark gray, you might need a higher value.
	cv::Mat binary_lines;
	cv::threshold(gray_image, binary_lines, 50, 255, cv::THRESH_BINARY);

	// Optional: Dilate the lines to ensure they are connected and form closed boundaries
	// This helps in finding complete rectangular contours.
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	const auto nb_filtering = 1;
	//cv::dilate(binary_lines, binary_lines, kernel, cv::Point(-1, -1), nb_filtering); // Dilate a couple of times
	//cv::erode(binary_lines, binary_lines, kernel, cv::Point(-1, -1), nb_filtering); // Erode a couple of times

	//cv::imshow("gray_image", gray_image);
	//cv::imshow("binary_lines", binary_lines);
	//cv::waitKey(0); // Wait for a key press to close the window


	// 4. Find contours in the binary image
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(binary_lines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// Create a copy of the original image to draw on
	cv::Mat output_image = image.clone();

	std::cout << "image before partition:" << std::endl;
	int block_count = 0;

	// 5. Iterate through the found contours
	std::vector<cv::Point> approx_poly;
	for (const auto& contour : contours) {
		// Approximate the contour to a polygon
		approx_poly.clear();
		cv::approxPolyDP(contour, approx_poly, cv::arcLength(contour, true) * 0.02, true);

		// Check if the approximated polygon has 4 vertices (a rectangle) and is sufficiently large
		// You might need to adjust the contour area threshold based on your image and expected block sizes

		if (approx_poly.size() == 4 && cv::isContourConvex(approx_poly)) {
			auto area = cv::contourArea(contour);
			if (area > (RUNE_DEFAULT_SIZE.area() / 2.0)) {
				// Get the bounding rectangle for the current contour
				cv::Rect bounding_rect = cv::boundingRect(contour);
				partition.push_back(cv::Rect(bounding_rect));
			}
		}
	}

	return true;
}

double frequencyToMidiNote(double frequency_hz) {
	if (frequency_hz <= 0.0) {
		// Frequencies must be positive. Return a sentinel value or throw an error.
		// MIDI notes are typically 0-127, so -1.0 is a good indicator of invalid input.
		return -1.0;
	}
	const double A4_FREQUENCY = 440.0; // Reference A4 frequency
	const int A4_MIDI_NOTE = 69;       // MIDI note number for A4

	// Calculate MIDI note number using the formula
	return 12.0 * std::log2(frequency_hz / A4_FREQUENCY) + A4_MIDI_NOTE;
}

int generate_wav(const std::vector<std::pair<Note, double>>& notes, const fs::path& wav_file) {
	WaveGenerator generator(44100, 16, 1, 1.0);

	for (const auto& [note, duration] : notes) {
		generator.addNote(note, duration);
	}

	if (generator.save(wav_file.string().c_str())) {
		std::cout << "Successfully created " << wav_file.stem().string() << std::endl;
	}
	else {
		std::cerr << "Failed to create " << wav_file.stem().string() << std::endl;
	}

	return 0;
}

bool translate_dictionary(const fs::path& rune_dictionary_file, const fs::path& lang_dictionnary_file) {

	RuneDictionary rune_dict_src, rune_dict_dst;
	rune_dict_src.load(rune_dictionary_file);

	Dictionary lang_dict_src(lang_dictionnary_file);

	auto filename = lang_dictionnary_file.stem().string();
	auto sep_pos = filename.find('-');
	auto target_language = filename.substr(sep_pos + 1);

	std::string word;
	std::vector<std::string> hash_list;
	rune_dict_src.get_hash_list(hash_list);
	for (const auto& hash : hash_list) {
		rune_dict_src.get_translation(hash, word);
		std::string translation = "";
		try {
			auto entry = lang_dict_src.find(word);
			if (entry.has_value()) {
				translation = entry.value().to_string();
			}
			translation += " "; // space befor following suffix
		}
		catch (...)
		{
		}
		// always suffix translation witho original word
		translation += "{" + word + "}";
		printf(" %s (eng) => %s (%s)\n", word.c_str(), translation.c_str(), target_language.c_str());
		rune_dict_dst.add_word(hash, translation);
	}

	auto parent_folder = rune_dictionary_file.parent_path();
	auto target_file = parent_folder / fs::path("dictionary." + target_language + ".txt");
	rune_dict_dst.save(target_file);

	return true;
}

bool translate_dictionary(const fs::path& rune_dictionary_file, const std::string& target_language) {
	RuneDictionary dict_src, dict_dst;
	dict_src.load(rune_dictionary_file);

	std::string word;
	std::vector<std::string> hash_list;
	dict_src.get_hash_list(hash_list);
	for (const auto& hash : hash_list) {
		dict_src.get_translation(hash, word);
		std::string translation = "";
		try {
			translation = translate(word, "eng", target_language);
			translation += " "; // space befor following suffix
		}
		catch (...) 
		{}
		// always suffix translation witho original word
		translation += "{" + word + "}";
		printf(" %s (eng) => %s (%s)\n", word.c_str(), translation.c_str(), target_language.c_str());
		dict_dst.add_word(hash, translation);
	}

	auto parent_folder = rune_dictionary_file.parent_path();
	auto target_file = parent_folder / fs::path("dictionary." + target_language + ".txt");
	dict_dst.save(target_file);

	return true;
}

std::string translate(const std::string& textToTranslate,
	const std::string& sourceLanguageCode,
	const std::string& targetLanguageCode) {

	//const auto DICTIONARY_ROOT_PATH = fs::path("../../../dict");
	const auto dictionary_file = "../../../dict/" + std::string(sourceLanguageCode) + "-" + std::string(targetLanguageCode) + ".tei";

	//fs::path dict_filepath = DICTIONARY_ROOT_PATH / dictionary_file;

	Dictionary dict(dictionary_file);
	auto dict_entry = dict.find(textToTranslate);

	if (dict_entry.has_value()) {
		return dict_entry.value().to_string();
	}

	return "";
}

// Using a lambda for the transformation, ensuring it's locale-safe for basic ASCII.
// The static_cast<int>(c) is important to prevent undefined behavior with char types
// that might be signed and have negative values (like some extended ASCII chars)
// when passed to functions like std::tolower that expect an int.
void toLowerFast(std::string& s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); });
}

// If you need a version that returns a new string and leaves the original untouched:
std::string toLowerFastCopy(const std::string& s) {
	std::string lower_s = s; // Make a copy
	std::transform(lower_s.begin(), lower_s.end(), lower_s.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return lower_s;
}

void resize_to_fit_max_bounds(cv::Mat& image, const cv::Size& max_bounds) {

	// 1. Handle empty image case
	if (image.empty()) {
		std::cerr << "Warning: Cannot resize empty image." << std::endl;
		return;
	}

	// 2. Get current image dimensions
	int current_width = image.cols;
	int current_height = image.rows;

	// 3. Get maximum allowed dimensions
	int max_width = max_bounds.width;
	int max_height = max_bounds.height;

	// 4. Check if the image already fits within the bounds
	if (current_width <= max_width && current_height <= max_height) {
		// Image already fits or is smaller, no upscaling required.
		// Do nothing.
		return;
	}

	// 5. Calculate scaling factors for both width and height
	//    Use double for precise ratio calculation to avoid integer truncation.
	double scale_factor_w = static_cast<double>(max_width) / current_width;
	double scale_factor_h = static_cast<double>(max_height) / current_height;

	// 6. Choose the smaller scale factor to ensure the image fits fully within the bounds
	//    (e.g., if image is very wide but not tall, it scales by width ratio)
	double scale_factor = (std::min)(scale_factor_w, scale_factor_h);

	// 7. Calculate new dimensions
	//    Using cv::round to ensure integer values for new dimensions
	int new_width = cvRound(current_width * scale_factor);
	int new_height = cvRound(current_height * scale_factor);

	// 8. Perform the resize operation
	//    cv::INTER_AREA is generally recommended for shrinking images as it avoids aliasing.
	cv::resize(image, image, cv::Size(new_width, new_height), 0, 0, cv::INTER_AREA);

	std::cout << "Resized image from " << current_width << "x" << current_height
		<< " to " << image.cols << "x" << image.rows << " (target max: "
		<< max_width << "x" << max_height << ")" << std::endl;
}