#include "runedetector.h"

#include <iostream>
#include <word.h>


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

	bool result = word.decode_image(word_image);

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

cv::Mat RuneDetector::cropBlackBorders(const cv::Mat& image) {
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


			if (debug_mode) {
				displayMatProperties(pattern_image, "Pattern: " + word.get_hash());
				//displayMatProperties(image, "Image");

				std::cout << "Try to find word: " << word.get_hash() << std::endl;
				cv::imshow("Try to find word : " + word.get_hash(), pattern_image);
			}

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
							cv::rectangle(original_img, bounding_box, cv::Scalar(255), 1);
						}
					}
				}
				std::cout << std::endl;
			}

			if (debug_mode) {
				cv::imshow("Detected Runes", original_img);
				cv::waitKey(300); // Wait for a key press to close the window
				cv::destroyAllWindows();
			}
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
