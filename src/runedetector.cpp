#include "runedetector.h"

#include <iostream>
#include <word.h>
#include <toolbox.h>


RuneDetector::RuneDetector(Dictionary* dictionary) : m_dictionary(dictionary)
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
	//cv::waitKey(0);
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

	std::vector<cv::Mat> partition;
	if (!partition_image(original_img, partition)) {
		std::cerr << "Error: Could not partition the image into potential rune words zones." << std::endl;
		return false;
	}

	for(const auto& potential_word_image : partition) {

		// Decode the word image
		Word word;
		if (!word.decode_image(potential_word_image)) {
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
 * @brief Draws text within a specified rectangle on an image, handling word wrapping and font scaling.
 * This is a complex helper function called by addTextToImage when a rect is provided.
 *
 * @param img The cv::Mat image to draw text on (modified in-place).
 * @param text The string of text to be drawn.
 * @param rect The cv::Rect bounding box for the text.
 * @param fontFace The font type (e.g., cv::FONT_HERSHEY_SIMPLEX).
 * @param initialFontScale The starting font scale to try.
 * @param color The color of the text.
 * @param thickness The thickness of the lines forming the text.
 * @param lineType The line type (e.g., cv::LINE_AA).
 * @param alignment The TextAlignment enum value for horizontal and vertical alignment.
 * @param lineSpacingFactor Factor to multiply line height for spacing (e.g., 1.2 for 20% extra space).
 * @param minFontScale Minimum font scale to try before giving up on fitting.
 */
void drawTextInRect(
	cv::Mat& img,
	const std::string& text,
	const cv::Rect& rect,
	int fontFace,
	double initialFontScale,
	cv::Scalar color,
	int thickness,
	int lineType,
	TextAlignment alignment,
	double lineSpacingFactor,
	double minFontScale
) {
	if (img.empty() || text.empty() || rect.empty()) {
		return;
	}

	double currentFontScale = initialFontScale;
	std::vector<std::string> wrappedLines;
	int totalTextHeight = 0;

	bool foundFit = false;
	while (currentFontScale >= minFontScale) {
		wrappedLines.clear();
		totalTextHeight = 0;

		std::vector<std::string> words = splitString(text, ' ');
		std::string currentLine;
		int currentLineHeight = 0;

		for (const std::string& word : words) {
			std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
			int baseline = 0;
			cv::Size testSize = cv::getTextSize(testLine, fontFace, currentFontScale, thickness, &baseline);

			if (testSize.width > rect.width && !currentLine.empty()) {
				wrappedLines.push_back(currentLine);
				totalTextHeight += currentLineHeight;
				if (!wrappedLines.empty()) {
					totalTextHeight += static_cast<int>(currentLineHeight * (lineSpacingFactor - 1));
				}
				currentLine = word;
				currentLineHeight = cv::getTextSize(currentLine, fontFace, currentFontScale, thickness, 0).height;

				if (cv::getTextSize(word, fontFace, currentFontScale, thickness, 0).width > rect.width) {
					goto reduce_font_scale;
				}
			}
			else {
				currentLine = testLine;
				currentLineHeight = std::max(currentLineHeight, testSize.height);
			}
		}
		if (!currentLine.empty()) {
			wrappedLines.push_back(currentLine);
			totalTextHeight += currentLineHeight;
			if (wrappedLines.size() > 1) {
				totalTextHeight += static_cast<int>(currentLineHeight * (lineSpacingFactor - 1));
			}
		}

		if (totalTextHeight <= rect.height) {
			foundFit = true;
			break;
		}

	reduce_font_scale:
		currentFontScale -= 0.05; // Slightly finer grain reduction
	}

	if (!foundFit && currentFontScale < minFontScale) {
		std::cerr << "Warning: Could not fit text into rectangle at minimum font scale. Text might be truncated or too small." << std::endl;
		if (wrappedLines.empty() && !text.empty()) {
			wrappedLines.push_back(text.substr(0, std::min((size_t)text.length(), (size_t)10)) + "..."); // Fallback to truncated text
		}
		if (wrappedLines.empty()) return;
	}

	int currentY = rect.y;
	if (alignment == TextAlignment::CENTER_CENTER || alignment == TextAlignment::LEFT_CENTER || alignment == TextAlignment::RIGHT_CENTER) {
		currentY = rect.y + (rect.height - totalTextHeight) / 2;
	}
	else if (alignment == TextAlignment::CENTER_BOTTOM || alignment == TextAlignment::LEFT_BOTTOM || alignment == TextAlignment::RIGHT_BOTTOM) {
		currentY = rect.y + rect.height - totalTextHeight;
	}

	int padding_x = thickness * 2;

	for (const std::string& line : wrappedLines) {
		int baseline = 0;
		cv::Size lineSize = cv::getTextSize(line, fontFace, currentFontScale, thickness, &baseline);

		int drawX = rect.x;
		if (alignment == TextAlignment::CENTER_CENTER || alignment == TextAlignment::CENTER_TOP || alignment == TextAlignment::CENTER_BOTTOM) {
			drawX = rect.x + (rect.width - lineSize.width) / 2;
		}
		else if (alignment == TextAlignment::RIGHT_CENTER || alignment == TextAlignment::RIGHT_TOP || alignment == TextAlignment::RIGHT_BOTTOM) {
			drawX = rect.x + rect.width - lineSize.width - padding_x;
		}
		else { // Left aligned
			drawX = rect.x + padding_x;
		}

		cv::Point org(drawX, currentY + lineSize.height);
		cv::putText(img, line, org, fontFace, currentFontScale, color, thickness, lineType);

		currentY += static_cast<int>(lineSize.height * lineSpacingFactor);
	}
}


// --- Modified addTextToImage Function ---

/**
 * @brief Adds text to a cv::Mat image, either at a specified point or fitting within a rectangle.
 *
 * @param image The cv::Mat image to draw text on. This image will be modified in-place.
 * @param text The string of text to be drawn.
 * @param org Optional: The bottom-left corner of the first text line if drawing at a point.
 * Must be provided if 'rect' is not.
 * @param rect Optional: The cv::Rect bounding box to fit the text into.
 * If provided, 'org' will be ignored, and text will be wrapped/scaled.
 * @param fontFace The font type (e.g., cv::FONT_HERSHEY_SIMPLEX). Default: FONT_HERSHEY_SIMPLEX.
 * @param fontScale The font scale factor. Default: 1.0.
 * @param color The color of the text (e.g., cv::Scalar(255, 0, 0) for blue in BGR). Default: Black.
 * @param thickness The thickness of the lines forming the text. Default: 1.
 * @param lineType The line type (e.g., cv::LINE_AA for anti-aliased). Default: LINE_AA.
 * @param bottomLeftOrigin When true, the image origin (0,0) is at the bottom-left corner. Default: false (top-left).
 * Only relevant if 'org' is used (i.e., 'rect' is not provided).
 * @param alignment The TextAlignment enum for horizontal/vertical alignment within 'rect'.
 * Only relevant if 'rect' is provided. Default: LEFT_TOP.
 * @param lineSpacingFactor Factor for line spacing when wrapping. Default: 1.2.
 * Only relevant if 'rect' is provided.
 * @param minFontScale Minimum font scale to try when fitting. Default: 0.3.
 * Only relevant if 'rect' is provided.
 * @param showImage If true, the image will be displayed in a window after drawing. Default: false.
 * @param windowName The name of the window to display the image in, if showImage is true.
 */
void addTextToImage(
	cv::Mat& image,
	const std::string& text,
	std::optional<cv::Point> org = std::nullopt,
	std::optional<cv::Rect> rect = std::nullopt,
	int fontFace = cv::FONT_HERSHEY_SIMPLEX,
	double fontScale = 1.0,
	cv::Scalar color = cv::Scalar(0, 0, 0),
	int thickness = 1,
	int lineType = cv::LINE_AA,
	bool bottomLeftOrigin = false,
	TextAlignment alignment = TextAlignment::LEFT_TOP,
	double lineSpacingFactor = 1.2,
	double minFontScale = 0.3,
	bool showImage = false,
	const std::string& windowName = "Image with Text"
) {
	if (image.empty() || text.empty()) {
		std::cerr << "Error: Input image or text is empty. Cannot draw text." << std::endl;
		return;
	}

	if (rect.has_value()) {
		// If a rectangle is provided, use the complex logic to fit text within it.
		// Parameters like bottomLeftOrigin are not directly applicable here as drawTextInRect handles its own placement.
		drawTextInRect(image, text, rect.value(), fontFace, fontScale, color, thickness, lineType, alignment, lineSpacingFactor, minFontScale);
	}
	else if (org.has_value()) {
		// If only an origin point is provided, use the simple cv::putText.
		cv::putText(image, text, org.value(), fontFace, fontScale, color, thickness, lineType, bottomLeftOrigin);
	}
	else {
		std::cerr << "Error: Neither 'org' point nor 'rect' box provided for text placement. No text drawn." << std::endl;
		return;
	}

	// Optional: Display the image
	if (showImage) {
		cv::imshow(windowName, image);
		cv::waitKey(0);
		cv::destroyWindow(windowName);
	}
}

bool RuneDetector::detect_words(const fs::path& image_path, std::vector<Word>& detected_words, bool debug_mode)
{
	auto original_img = cv::imread(image_path.string(), cv::IMREAD_COLOR_BGR);
	if (original_img.empty()) {
		std::cerr << "Error: Could not load rune image from " << image_path << std::endl;
		return false;
	}

	std::vector<double> scale_factors;
	cv::Mat image;
	cv::cvtColor(original_img, image, cv::COLOR_BGR2GRAY);
	image.convertTo(image, CV_8U);
	std::vector<RuneZone> detected_runes_zones;

	// detect word in image
	for (const auto& [key, pattern_image_original] : m_rune_images) {

		auto word = Word(key);

		generate_scale_factors(pattern_image_original, image, scale_factors);

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
					if (result.at<float>(j, i) > 0.8) {
						cv::Rect bounding_box = cv::Rect(i, j, pattern_image.cols, pattern_image.rows);
						detected_runes_zones.push_back({ word, bounding_box });
						if (debug_mode) {
							//std::cout << "Match found at (" << i << ", " << j << ") with value: " 
							//	<< result.at<float>(j, i) << std::endl;
							cv::rectangle(original_img, bounding_box, cv::Scalar(255, 255, 255), -1);

							cv::Rect text_zone = cv::Rect(
								bounding_box.x + 0.05* bounding_box.width,
								bounding_box.y + 0.05* bounding_box.height, // Position below the rune
								bounding_box.width - 0.1 * bounding_box.width,
								bounding_box.height - 0.1 * bounding_box.height // Fixed height for the text zone
							);
							std::string translation = m_dictionary->translate(word);
							//draw_text(original_img, bounding_box, translation, cv::Scalar(255));
							addTextToImage(
								original_img,
								translation,
								std::nullopt,
								text_zone,
								cv::FONT_HERSHEY_SIMPLEX,
								0.5, // Font scale
								cv::Scalar(0, 0, 0), // White color
								1, // Thickness
								cv::LINE_AA, // Line type,
								false,
								TextAlignment::CENTER_CENTER, // Alignment
								1.2, // Line spacing factor
								0.1 // Minimum font scale
							);
						}
					}
				}
				std::cout << std::endl;
			}
		}
		if (debug_mode) {
			cv::imshow("Detected Runes", original_img);
			cv::imshow("Pattern to find", pattern_image_original);
			cv::waitKey(2000); // Wait for a key press to close the window
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
	int nb_values = 10;
	
	// simple method
	double min_factor = 0.15; // half size of the pattern
	double max_factor = 1;   // 4 times pattern size

	// ellaborated method
	//scale_factors.clear();
	//if (image.empty() || pattern.empty()) {
	//	std::cerr << "Error: One of the images is empty." << std::endl;
	//	return false;
	//}

	//// min size: 
	//// - a page of the manual: 1680x1280 pixels 
	//// - the smallest rune on the page: 18x36 pixels
	//// -> min width factor: 18 / 1680 = 0,0107142857142857
	//// -> min height factor: 36 / 1280 = 0,028125

	//// max size:
	//// - close up of the in-game popup: 298x53 pixels
	//// - the biggest rune on the popup: 18x31 pixels
	//// -> max width ratio: 298 / 18 = 16,55555555555556
	//// -> max height ration: 53 / 31 = 0,5849056603773585


	//// pattern is always on one line, so we can use it to determine the width of a single rune
	//auto single_rune_width = static_cast<double>(pattern.rows) * 18.0f / 32.0f; // average height / width ratio of the rune

	//auto runne_image_ratio = static_cast<double>(image.cols) / single_rune_width;

	//const double min_factor = 0.01 * runne_image_ratio; // Minimum scale factor
	//const double max_factor = 0.06 * runne_image_ratio; // Maximum scale factor

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
