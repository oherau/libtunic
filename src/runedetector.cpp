#include "runedetector.h"

#include <iostream>
#include <word.h>
#include <toolbox.h>


RuneDetector::RuneDetector(RuneDictionary* dictionary) : m_dictionary(dictionary)
{
}

bool RuneDetector::load_rune_folder(const fs::path& dict_folder)
{
	// Check if the folder exists
	if (!fs::exists(dict_folder) || !fs::is_directory(dict_folder)) {
		std::cerr << "Error: Rune folder does not exist or is not a directory: " << dict_folder << std::endl;
		return false;
	}

	for(const auto& entry : fs::directory_iterator(dict_folder)) {
		if (entry.is_regular_file()) {
			if (!register_word_image(entry.path())) {
				std::cerr << "Error: Could not load rune image for " << entry.path() << std::endl;
				return false;
			}
		} else {
			std::cerr << "Skipping non-file entry: " << entry.path() << std::endl;
		}
	}

	return false;
}

bool RuneDetector::register_word_image(const fs::path& word_image)
{
	auto filename = word_image.stem().string();

	auto pos = filename.find(RUNE_WORD_TRANSLATION_SEPARATOR);

	std::string rune_part = "";
	std::string translation = "";
	Word word;
	if(pos != std::string::npos) {
		rune_part = filename.substr(0, pos);
		word = Word(rune_part);
		translation = filename.substr(pos + 1);
	}
	else {
		rune_part = filename;
		word = Word(rune_part);
		translation = word.to_pseudophonetic();
	}

	
	if(word.size() == 0) {
		std::cerr << "Error: Invalid word name from image path: " << word_image << std::endl;
		return false;
	}

	// Load the rune image from the specified path
	auto image = cv::imread(word_image.string(), cv::IMREAD_GRAYSCALE);
	if (image.empty()) {
		std::cerr << "Error: Could not load rune image from " << word_image << std::endl;
		return false;
	}
	// Store the rune bin value
	image.convertTo(image, CV_8U);
	
	// Store the rune image in the map with the word's hash as the key
	m_rune_images[word.get_hash()] = image;

	std::cout << "Loaded image for: " << word.get_hash() << std::endl;

	if (translation.size() > 0 && m_dictionary != nullptr) {
		m_dictionary->add_word(word.get_hash(), translation);
	}

	return true;
}

bool RuneDetector::decode_word_image(const fs::path& file_path, Word& word)
{
	auto word_image = cv::imread(file_path.string(), cv::IMREAD_COLOR_BGR);
	if (word_image.empty()) {
		std::cerr << "Error: Could not load rune image from " << word_image << std::endl;
		return false;
	}

	//cv::imshow("word_image before", word_image);


	make_white_rune_black_background(word_image);
	//cv::imshow("word_image after", word_image);
	//cv::waitKey(1000);
	//cv::destroyAllWindows();


	int line_center_y = 0;
	int tickness = 0;
	if (!find_horizontal_separator(word_image, line_center_y, tickness)) {
		std::cerr << "No horizontal separator found in the rune image." << std::endl;
		return false; // No separator line found
	}

	int line_center_x_min = 0;
	int line_center_x_max = word_image.cols;
	if (!find_horizontal_separator_bounds(word_image, line_center_y, line_center_x_min, line_center_x_max)) {
		std::cerr << "Error when trying to find word width" << std::endl;
		return false;
	}

	// crop size - we only crop the left size of the word image
	int height = word_image.rows;
	int width = word_image.cols - line_center_x_min; 
	cv::Rect roi(line_center_x_min, 0, width, height);
	cv::Mat cropped_image = word_image(roi);

	bool word_decoding_result = word.decode_image(cropped_image);

	// last step: an image of the decoded word is generated and a detection is done on original image
	// TODO: launch a detection on originial image to confirm crrectness of the decoding
	bool result = word_decoding_result;

	return result;
}

// Custom comparison function for sorting character zones
bool compareCharacterZones(const RuneZone& a, const RuneZone& b) {
	// Define a small tolerance for vertical alignment to consider characters on the same line
	// You might need to adjust this value based on your font sizes and image resolution.
	const int verticalTolerance = 10; // Example tolerance in pixels

	// First, compare by vertical position (top to bottom)
	// If they are on roughly the same line, then compare by horizontal position.
	if (std::abs(a.rect.y - b.rect.y) < verticalTolerance) {
		// If on the same "line" (within tolerance), sort by horizontal position (left to right)
		return a.rect.x < b.rect.x;
	}
	else {
		// Sort by vertical position
		return a.rect.y < b.rect.y;
	}
}

cv::Mat RuneDetector::get_image_lines(const cv::Mat& src) {

	// Declare the output variables
	cv::Mat dst, cdst, cdstP;

	// Edge detection
	cv::Canny(src, dst, 50, 200, 3);
	// Copy edges to the images that will display the results in BGR
	cvtColor(dst, cdst, cv::COLOR_GRAY2BGR);
	cdstP = cdst.clone();
	// Standard Hough Line Transform
	std::vector<cv::Vec2f> lines; // will hold the results of the detection
	HoughLines(dst, lines, 1, CV_PI / 180, 150, 0, 0); // runs the actual detection
	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++)
	{
		float rho = lines[i][0], theta = lines[i][1];
		cv::Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		cv::line(cdst, pt1, pt2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}

	// Probabilistic Line Transform
	std::vector<cv::Vec4i> linesP; // will hold the results of the detection
	HoughLinesP(dst, linesP, 1, CV_PI / 180, 50, 50, 10); // runs the actual detection
	// Draw the lines
	for (size_t i = 0; i < linesP.size(); i++)
	{
		cv::Vec4i l = linesP[i];
		line(cdstP, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
	}

	return cv::Mat(cdstP);
}

cv::Mat RuneDetector::crop_black_borders(const cv::Mat& image) {
	if (image.empty()) {
		std::cerr << "Input image is empty. Cannot crop." << std::endl;
		return cv::Mat(); // Return empty Mat
	}

	// 1. Convert to grayscale (if not already)
	cv::Mat grayImage;
	if (image.channels() == 3) {
		cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
	}
	else {
		grayImage = image.clone(); // If already grayscale, just make a copy
	}

	// 2. Threshold the image to create a binary mask
	// Pixels brighter than 'threshold_value' become 255 (white), others become 0 (black)
	// We want to identify non-black pixels as content.
	// A low threshold (e.g., 5 or 10) is good to catch near-black pixels as part of the border.
	cv::Mat binaryImage;
	int threshold_value = 10; // Adjust this value if your "black" border isn't pure black
	cv::threshold(grayImage, binaryImage, threshold_value, 255, cv::THRESH_BINARY);

	// Optional: Morphological operations to clean up small noise or gaps
	// cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	// cv::morphologyEx(binaryImage, binaryImage, cv::MORPH_OPEN, kernel, cv::Point(-1,-1), 2); // Remove small black dots
	// cv::morphologyEx(binaryImage, binaryImage, cv::MORPH_CLOSE, kernel, cv::Point(-1,-1), 2); // Fill small white gaps

	// 3. Find contours in the binary image
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(binaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// 4. Find the bounding rectangle of the largest contour (assumed to be the content)
	cv::Rect boundingBox;
	if (!contours.empty()) {
		double maxArea = 0;
		int largestContourIdx = -1;

		for (size_t i = 0; i < contours.size(); i++) {
			double area = cv::contourArea(contours[i]);
			if (area > maxArea) {
				maxArea = area;
				largestContourIdx = i;
			}
		}

		if (largestContourIdx != -1) {
			boundingBox = cv::boundingRect(contours[largestContourIdx]);
		}
		else {
			// This case should ideally not be hit if contours is not empty
			std::cerr << "No valid largest contour found. Returning original image." << std::endl;
			return image.clone();
		}
	}
	else {
		std::cerr << "No contours found in the image. It might be all black or all white. Returning original image." << std::endl;
		return image.clone(); // Or return an empty Mat if an empty image is desired
	}

	// 5. Crop the original image using the detected bounding box
	// Ensure the bounding box is valid before cropping
	if (boundingBox.width <= 0 || boundingBox.height <= 0) {
		std::cerr << "Calculated bounding box has zero or negative dimensions. Returning original image." << std::endl;
		return image.clone();
	}

	cv::Mat croppedImage = image(boundingBox);

	return croppedImage;
}

// Take an image, partition it into potential rune words zones, try to parse and store them in the dictionary if not already present
bool RuneDetector::dictionarize(const fs::path& image_path, bool debug_mode)
{
	// Load the image
	auto original_img = cv::imread(image_path.string(), cv::IMREAD_COLOR_BGR);

	// prepare the image
	make_white_rune_black_background(original_img);

	std::vector<cv::Rect> partition;
	if (!partition_image(original_img, partition)) {
		std::cerr << "Error: Could not partition the image into potential rune words zones." << std::endl;
		return false;
	}

	int i = 1;
	for(const auto& part : partition) {
		
		//// Draw the bounding rectangle on the output image
		//cv::rectangle(original_img, part.tl(), part.br(), cv::Scalar(0, 255, 0), 2); // Green rectangle

		//// List the detected block's coordinates
		//std::cout << "Block " << ++block_count << ": (x=" << bounding_rect.x
		//	<< ", y=" << bounding_rect.y
		//	<< ", width=" << bounding_rect.width
		//	<< ", height=" << bounding_rect.height << ")" << std::endl;

		// You can also extract the individual block as a separate image
		cv::Mat extracted_block = original_img(part).clone();
		//cv::imshow("Extracted Block " + std::to_string(block_count), extracted_block); 
		//cv::imwrite("extracted_block_" + std::to_string(block_count) + ".jpg", extracted_block); // Save to file

		if (debug_mode) {
			// for debugging purposes, display the image
			cv::imshow("Part " + i, extracted_block);
			cv::waitKey(1000); // Wait for a key press to close the window
			cv::destroyAllWindows();
		}

		// Decode the word image
		Word word;
		if (!word.decode_image(extracted_block)) {
			std::cerr << "Error: Could not decode word image." << std::endl;
			continue; // Skip to the next image if decoding fails
		}
		// Check if the word is already in the dictionary
		std::string translation;
		if (m_dictionary->has_hash(word.get_hash())) {
			std::cout << "Word '" << word.get_hash() << "' already exists in the dictionary with translation: " << translation << std::endl;
			continue; // Skip if the word is already in the dictionary
		}
		// Add the new word to the dictionary
		m_dictionary->add_word(word.get_hash(), word.to_pseudophonetic());
		std::cout << "Added new word '" << word.get_hash() << "' to the dictionary." << std::endl;

		i++;
	}

	return false;
}

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // For std::max
#include <optional>  // Required for std::optional

// --- Helper Definitions (from previous explanations) ---

// Define an enum for text alignment within a rectangle
enum class TextAlignment {
	LEFT_TOP,
	LEFT_CENTER,
	LEFT_BOTTOM,
	CENTER_TOP,
	CENTER_CENTER,
	CENTER_BOTTOM,
	RIGHT_TOP,
	RIGHT_CENTER,
	RIGHT_BOTTOM
};

// Helper to split a string by delimiter (used for word wrapping)
std::vector<std::string> splitString(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

/**
*@brief Draws text within a specified rectangle on an OpenCV image, scaling the font
* size to fit the text within the rectangle's dimensions directly.
*
*@param image The input image(cv::Mat).
* @param text The text string to be drawn.
* @param rect A cv::Rect representing the rectangle(x, y, width, height).
* @param font_face Font type, e.g., cv::FONT_HERSHEY_SIMPLEX.
* @param font_scale_multiplier A multiplier for the calculated font scale.
* Use to fine - tune font size.
* @param thickness Thickness of the text lines.
* @param color Color of the text in BGR format(e.g., cv::Scalar(255, 255, 255) for white).
* @param padding Padding in pixels to apply around the text within the rectangle.
* @param h_align Horizontal alignment of the text(Left, Center, Right).
* @param v_align Vertical alignment of the text(Top, Middle, Bottom).
* @param background_color Optional background color to fill the rectangle.If not provided(e.g., cv::Scalar()),
* the rectangle will not be filled.
* @return cv::Mat The image with the text drawn on it.
*/
// TODO: MOVE THIS FUNCTION IN TOOLBOX
cv::Mat draw_text_in_rect(
	cv::Mat & image,
	const std::string & text,
	const cv::Rect & rect,
	int font_face = cv::FONT_HERSHEY_SIMPLEX,
	double font_scale_multiplier = 1.0,
	int thickness = 2,
	cv::Scalar color = cv::Scalar(255, 255, 255), // White color in BGR
	cv::Scalar background_color = cv::Scalar(-1, -1, -1), // Default to an invalid scalar to indicate no fill
	int padding = 5,
	HorizontalAlignment h_align = HorizontalAlignment::Center,
	VerticalAlignment v_align = VerticalAlignment::Middle
) {
	int x = rect.x;
	int y = rect.y;
	int w = rect.width;
	int h = rect.height;

	// Calculate available space for text considering padding
	int available_width = w - 2 * padding;
	int available_height = h - 2 * padding;

	if (available_width <= 0 || available_height <= 0) {
		std::cerr << "Warning: Rectangle dimensions (" << w << "x" << h
			<< ") too small for padding (" << padding << "). Text might not be visible." << std::endl;
		return image; // Return original image if no space
	}

	// Fill the rectangle with the background color if provided
	// Check if any component of the scalar is non-negative (valid color)
	if (background_color[0] >= 0 && background_color[1] >= 0 && background_color[2] >= 0) {
		cv::rectangle(image, rect, background_color, cv::FILLED);
	}

	// Calculate font scale directly
	double font_scale = 1.0; // Start with a reference font scale
	int baseline = 0;
	cv::Size text_size_ref = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

	// Prevent division by zero if text_size_ref.width or height is 0 (e.g., empty string)
	if (text_size_ref.width == 0 || text_size_ref.height == 0) {
		return image;
	}

	// Calculate scale factor needed for width
	double scale_factor_width = (double)available_width / text_size_ref.width;
	// Calculate scale factor needed for height
	double scale_factor_height = (double)available_height / text_size_ref.height;

	// Choose the minimum scale factor to ensure text fits both dimensions
	font_scale = std::min(scale_factor_width, scale_factor_height);

	// Apply the font scale multiplier
	font_scale *= font_scale_multiplier;

	// Recalculate text size with the determined font_scale
	baseline = 0;
	cv::Size final_text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

	// Calculate the position to align the text within the rectangle
	int text_x;
	switch (h_align) {
	case HorizontalAlignment::Left:
		text_x = x + padding;
		break;
	case HorizontalAlignment::Center:
		text_x = x + padding + (available_width - final_text_size.width) / 2;
		break;
	case HorizontalAlignment::Right:
		text_x = x + padding + available_width - final_text_size.width;
		break;
	}

	int text_y;
	// The distance from the top of the text bounding box to the baseline
	int text_height_above_baseline = final_text_size.height - baseline;

	switch (v_align) {
	case VerticalAlignment::Top:
		// Baseline position = top of rect + padding + text_height_above_baseline
		text_y = y + padding + text_height_above_baseline;
		break;
	case VerticalAlignment::Middle:
		// Baseline position = center of available height + (half text height above baseline)
		// (available_height / 2.0) is the center of the available space
		// (final_text_size.height / 2.0 - baseline) is the offset from text center to baseline
		text_y = y + padding + static_cast<int>(std::round(available_height / 2.0 + final_text_size.height / 2.0 - baseline));
		break;
	case VerticalAlignment::Bottom:
		// Baseline position = bottom of rect - padding - baseline
		text_y = y + padding + available_height - baseline;
		break;
	}

	// Round coordinates to nearest integer pixel
	text_x = static_cast<int>(std::round(text_x));
	text_y = static_cast<int>(std::round(text_y));

	// Draw the text on the image
	cv::putText(image, text, cv::Point(text_x, text_y), font_face, font_scale, color, thickness, cv::LINE_AA);

	return image;
}

bool RuneDetector::detect_words(cv::Mat& original_img, std::vector<Word>& detected_words, int adaptative_cycles, bool debug_mode)
{
	//const auto ADAPTATIVE_DETECTIONS_THRESHOLD = 5;
	std::vector<double> scale_factors, adapt_scale_factors_confirmed;
	int adapt_detections = 0;
	cv::Mat image;
	cv::cvtColor(original_img, image, cv::COLOR_BGR2GRAY);
	image.convertTo(image, CV_8U);
	std::vector<RuneZone> detected_runes_zones;

	// TODO: add itermediate function to determine if the runes are:
	//   - white on dark backround (nothing to do)
	//   - black on bright background (invert color)



	// detect word in image
	for (const auto& [key, pattern_image_original] : m_rune_images) {

		auto word = Word(key);

		double best_scale_factor = 0;
		double best_scale_corr = 0;

		if (adaptative_cycles > 0 && adapt_detections > adaptative_cycles) {
			// once enough runes are found we use the few factors that gave sucessful detections (list should be much smaller)
			scale_factors = adapt_scale_factors_confirmed;
		}
		else {
			generate_scale_factors(image, pattern_image_original, scale_factors);
		}


		for (const auto& scale_factor : scale_factors) {
			// Resize the rune image to the current scale factor
			cv::Mat pattern_image;
			cv::resize(pattern_image_original, pattern_image, cv::Size(), scale_factor, scale_factor, (scale_factor > 1.0f ? cv::INTER_LINEAR : cv::INTER_AREA));
			if (pattern_image.empty()) {
				std::cerr << "Error: Resized rune image is empty for word: " << word.get_hash() << std::endl;
				continue;
			}
			// Check if the resized rune image is larger than the original image
			if (pattern_image.rows > image.rows || pattern_image.cols > image.cols) {
				std::cerr << "Error: Resized rune image is larger than the original image for word: " << word.get_hash() << std::endl;
				continue;
			}


			//if (debug_mode) {
			//	displayMatProperties(pattern_image, "Pattern: " + word.get_hash());
			//	//displayMatProperties(image, "Image");

			//	std::cout << "Try to find word: " << word.get_hash() << std::endl;
			//	cv::imshow("Try to find word : " + word.get_hash(), pattern_image);
			//}

			double pattern_area = pattern_image.rows * pattern_image.cols;

			// Create the result matrix
			int result_cols = image.cols - pattern_image.cols + 1;
			int result_rows = image.rows - pattern_image.rows + 1;
			cv::Mat result;
			result.create(result_rows, result_cols, CV_32FC1); // Result is float type

			cv::matchTemplate(image, pattern_image, result, cv::TM_CCOEFF_NORMED);

			for (int i = 2; i < result_cols - 2; i++) {
				for (int j = 2; j < result_rows - 2; j++) {

					// keep correlation even is not good enough
					if (result.at<float>(j, i) > best_scale_corr) {
						best_scale_factor = scale_factor;
						best_scale_corr = result.at<float>(j, i);
					}

					if (result.at<float>(j, i) > 0.8) {
						if (adaptative_cycles > 0) {
							adapt_detections++;
							if (std::find(adapt_scale_factors_confirmed.begin(), adapt_scale_factors_confirmed.end(), scale_factor) == adapt_scale_factors_confirmed.end()) {
								adapt_scale_factors_confirmed.push_back(scale_factor);
							}
						}

						cv::Rect bounding_box = cv::Rect(i, j, pattern_image.cols, pattern_image.rows);
						detected_runes_zones.push_back({ word, bounding_box });
						if (debug_mode) {


							cv::Rect text_zone = cv::Rect(
								bounding_box.x + 0.05* bounding_box.width,
								bounding_box.y + 0.05* bounding_box.height, // Position below the rune
								bounding_box.width - 0.1 * bounding_box.width,
								bounding_box.height - 0.1 * bounding_box.height // Fixed height for the text zone
							);
							std::string translation = m_dictionary->translate(word);

							int fontFace = 0;
							double tickness = 1;
							int padding = 0;
							auto fontColor = cv::Scalar(255, 255, 255);
							auto bgColor = cv::Scalar(0, 0, 0);
							draw_text_in_rect(original_img, translation, text_zone, fontFace, 1.0, tickness, fontColor, bgColor, padding);
						}
					}
				}
				std::cout << std::endl;
			}
		}
		if (debug_mode) {
			//cv::destroyAllWindows();
			cv::imshow("Detected Runes", original_img);
			cv::imshow("Pattern to find", pattern_image_original);
			std::cout << "Best scale factor: " << best_scale_factor << std::endl
				<< "Best scale correlation: " << best_scale_corr << std::endl;
			cv::waitKey(500); // Wait for a key press to close the window
			cv::destroyAllWindows();
		}
	}

	// Sort the character zones
	std::sort(detected_runes_zones.begin(), detected_runes_zones.end(), compareCharacterZones);

	std::vector<RuneZone> processed_rune_zones = detected_runes_zones;

	//// --- Process runes at the same position ---

	//std::vector<RuneZone> processed_rune_zones; // Renamed vector
	//if (!detected_runes_zones.empty()) {
	//	processed_rune_zones.push_back(detected_runes_zones[0]); // Add the first rune

	//	for (size_t i = 1; i < detected_runes_zones.size(); ++i) {
	//		// Check if the current rune is at the exact same position as the last processed one
	//		if (detected_runes_zones[i] == processed_rune_zones.back()) {
	//			std::cout << "Detected rune '" << detected_runes_zones[i].word.get_hash()
	//				<< "' at the same position as '" << processed_rune_zones.back().word.get_hash()
	//				<< "' (Rect: " << detected_runes_zones[i].rect.x << "," << detected_runes_zones[i].rect.y << "). ";

	//			std::cout << "Skipping duplicate, keeping '" << processed_rune_zones.back().word.get_hash() << "'.\n";

	//		}
	//		else {
	//			// If not at the same position, add it to the processed list
	//			processed_rune_zones.push_back(detected_runes_zones[i]);
	//		}
	//	}
	//}

	// Print the sorted characters to demonstrate the order
	std::cout << "Runes in occidental reading order:\n";
	if (!processed_rune_zones.empty()) {
		int current_line_y = processed_rune_zones[0].rect.y;
		const int verticalTolerance = 10; // Same tolerance as in comparison function

		for (size_t i = 0; i < processed_rune_zones.size(); ++i) {
			// Check if we're starting a new line (with tolerance)
			if (i > 0 && std::abs(processed_rune_zones[i].rect.y - current_line_y) >= verticalTolerance) {
				current_line_y = processed_rune_zones[i].rect.y;
			}
			std::cout << processed_rune_zones[i].word.get_hash();
			detected_words.push_back(Word(processed_rune_zones[i].word));
		}
		std::cout << "\n";
	}

	return true;
}



// Function to display cv::Mat properties
void RuneDetector::displayMatProperties(const cv::Mat& mat, const std::string& name) {
	std::cout << "--- " << name << " Properties ---" << std::endl;

	if (mat.empty()) {
		std::cout << "  (Matrix is empty)" << std::endl;
		std::cout << "--------------------------" << std::endl;
		return;
	}

	// Dimensions
	std::cout << "  Dimensions: " << mat.dims << std::endl;
	std::cout << "  Rows (Height): " << mat.rows << std::endl;
	std::cout << "  Cols (Width): " << mat.cols << std::endl;

	// Size
	std::cout << "  Size: " << mat.size() << std::endl; // Prints [width x height]

	// Channels
	std::cout << "  Channels: " << mat.channels() << std::endl;

	// Depth (Data Type)
	std::cout << "  Depth (OpenCV Type): " << mat.depth() << std::endl;
	std::cout << "  Type (Full Type Code): " << mat.type() << std::endl;

	// Convert depth code to human-readable string
	std::string depth_str;
	switch (mat.depth()) {
	case CV_8U:  depth_str = "CV_8U (unsigned char/byte)"; break;
	case CV_8S:  depth_str = "CV_8S (signed char)"; break;
	case CV_16U: depth_str = "CV_16U (unsigned short)"; break;
	case CV_16S: depth_str = "CV_16S (signed short)"; break;
	case CV_32S: depth_str = "CV_32S (signed int)"; break;
	case CV_32F: depth_str = "CV_32F (float)"; break;
	case CV_64F: depth_str = "CV_64F (double)"; break;
	case CV_16F: depth_str = "CV_16F (half float - only in certain modules)"; break; // C++11 required for this
	default:     depth_str = "UNKNOWN_DEPTH"; break;
	}
	std::cout << "  Depth (Human Readable): " << depth_str << std::endl;

	// Element Size (size of one pixel element in bytes)
	std::cout << "  Element Size (bytes): " << mat.elemSize() << std::endl;

	// Element Size (size of one channel element in bytes)
	std::cout << "  Single Channel Element Size (bytes): " << mat.elemSize1() << std::endl;

	// Total number of elements (pixels)
	std::cout << "  Total Elements (Pixels): " << mat.total() << std::endl;

	// Is Continuous? (important for direct memory access)
	std::cout << "  Is Continuous: " << (mat.isContinuous() ? "Yes" : "No") << std::endl;

	// Step (bytes per row)
	std::cout << "  Step (bytes per row): " << mat.step[0] << std::endl;

	// Data pointer (address of the first byte of the matrix data)
	std::cout << "  Data Pointer: " << static_cast<void*>(mat.data) << std::endl;

	// Min/Max values (Note: This is computationally intensive for large images)
	// For single-channel images, you can easily get min/max.
	// For multi-channel images, you'd typically split channels first.
	if (mat.channels() == 1 && !mat.empty()) {
		double minVal, maxVal;
		cv::minMaxLoc(mat, &minVal, &maxVal);
		std::cout << std::fixed << std::setprecision(2); // Set precision for float/double
		std::cout << "  Min Value (1-ch): " << minVal << std::endl;
		std::cout << "  Max Value (1-ch): " << maxVal << std::endl;
		std::cout << std::defaultfloat << std::setprecision(6); // Reset to default
	}
	else if (mat.channels() > 1 && !mat.empty()) {
		std::cout << "  Min/Max values not shown for multi-channel directly (split channels first)." << std::endl;
	}

	std::cout << "--------------------------" << std::endl;
}

bool RuneDetector::generate_scale_factors(const cv::Mat& image, const cv::Mat& pattern, std::vector<double>& scale_factors)
{
	// TODO: improve this method to use the size of the rune in the image to determine the scale factors
	int nb_values = 20;

	// ex: MIN SIZE rune 20x42 pix on a 1680x1280 screenshot of a page of the manual (horizontal: 0.011904761 vertical: 0.0328125 )
	// ex: MAX SIZE rune 16x27 pix on a 342x255   screenshot of a page of the manual (horizontal: 0.046783626 vertical: 0.10588235)
	double MIN_FACTOR_BOUND = 0.2; // LOWER FACTOR BOUND
	double MAX_FACTOR_BOUND = 0.8;  // UPPER FACTOR BOUND

	double min_h_ratio = 0.011904761;
	double min_w_ratio = 0.0328125;
	double max_h_ratio = 0.046783626;
	double max_w_ratio = 0.10588235;

	double min_h_rune = min_h_ratio * image.rows;
	double min_w_rune = min_w_ratio * image.cols;
	double max_h_rune = max_h_ratio * image.rows;
	double max_w_rune = max_w_ratio * image.cols;

	double min_h_factor = min_h_rune / RUNE_DEFAULT_SIZE.height;
	double min_w_factor = min_w_rune / RUNE_DEFAULT_SIZE.width;
	double max_h_factor = max_h_rune / RUNE_DEFAULT_SIZE.height;
	double max_w_factor = max_w_rune / RUNE_DEFAULT_SIZE.width;

	double min_factor = std::max(MIN_FACTOR_BOUND, std::min(min_h_factor, min_w_factor));
	double max_factor = std::min(MAX_FACTOR_BOUND, std::max(max_h_factor, max_w_factor));

	// Calculate the base for the exponential growth
		// We want min_factor * (ratio^S) = max_factor
		// So, ratio^S = max_factor / min_factor
		// ratio = (max_factor / min_factor)^(1/nb_values)
	double ratio = std::pow(max_factor / min_factor, 1.0 / nb_values);

	// Generate the sequence
	for (int i = 0; i <= nb_values; ++i) {
		double scale_factor = min_factor * std::pow(ratio, i);
		scale_factors.push_back(scale_factor);
	}

	return true;
}


int RuneDetector::image_detection(const fs::path& image_file, const fs::path& output_file) {

	const auto RUNES_FOLDER = fs::path("../../../data/runes");

	auto original_img = cv::imread(image_file.string(), cv::IMREAD_COLOR_BGR);
	if (original_img.empty()) {
		std::cerr << "Error: Could not load rune image from " << image_file << std::endl;
		return false;
	}

	resize_to_fit_max_bounds(original_img, MAX_IMAGE_DETECTION_DIMENSIONS);

    std::vector<Word> detected_words;
	int adaptative_cycles = 7;
	bool debug_mode = true;
    this->detect_words(original_img, detected_words, adaptative_cycles, debug_mode);

    // Afficher la séquence de runes detectees
    std::cout << "==== DETECTED RUNES ====" << std::endl;
    for(const auto& word : detected_words) {
        std::cout << word.get_hash() << " ";
    }
    std::cout << std::endl << std::endl;

    
    std::cout << "==== TRANSLATION ==== " << std::endl;
    for(const auto& word : detected_words) {
        std::cout << word.get_hash() << " -> " << m_dictionary->translate(word) << std::endl;
	}

	// save image
	bool save_result = cv::imwrite(output_file.string(), original_img);

	//return -1; // Placeholder for image detection logic
	return true;
}

template <typename T>
int test_check(const T& expected, const T& result) {
    bool testOK = (result == expected);
    auto resultStr = testOK ? "[ OK ] " : "[ KO ] ";
    std::cout << resultStr << "expected: [" << expected << "]  result: [" << result << "]" << std::endl;
    return testOK ? 0 : 1;
}
