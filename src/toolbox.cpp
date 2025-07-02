#include "toolbox.h"
#include "wavgenerator.h"
#include <iostream>
#include <fstream>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <cmath>   // For sin and M_PI
#include <numeric> // For std::iota (C++11+)
#include <map>     // For std::map
#include <string>  // For std::string in main's debug output
#include <cstdint> // For uint16_t, uint32_t, int16_t etc.

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

//
//int main() {
//    // Create a sample binary image with a white square and some noise
//    cv::Mat src = cv::Mat::zeros(300, 300, CV_8UC1); // Black background, single channel
//    cv::rectangle(src, cv::Point(70, 70), cv::Point(230, 230), cv::Scalar(255), cv::FILLED); // White square
//    cv::circle(src, cv::Point(150, 150), 20, cv::Scalar(0), cv::FILLED); // Black hole in center
//    cv::circle(src, cv::Point(50, 50), 5, cv::Scalar(255), cv::FILLED);  // Small white "salt" noise
//    cv::rectangle(src, cv::Point(200, 20), cv::Point(210, 30), cv::Scalar(255), cv::FILLED); // Another noise
//    cv::line(src, cv::Point(10, 280), cv::Point(30, 260), cv::Scalar(255), 2); // Thin line noise
//
//    // You can also load an image:
//    // cv::Mat src = cv::imread("path_to_your_image.jpg", cv::IMREAD_GRAYSCALE); // Load as grayscale
//    // if (src.empty()) {
//    //     std::cerr << "Error: Could not load image." << std::endl;
//    //     return -1;
//    // }
//    // // Convert to binary if it's not already
//    // cv::threshold(src, src, 128, 255, cv::THRESH_BINARY);
//
//    cv::imshow("Original Image", src);
//
//    // --- Apply operations using the new functions ---
//
//    int current_kernel_size = 5;
//    int current_iterations = 1;
//
//    // Erosion
//    cv::Mat eroded_result = applyErosion(src, current_kernel_size, current_iterations);
//    cv::imshow("Eroded Image (Kernel: " + std::to_string(current_kernel_size) + ", Iterations: " + std::to_string(current_iterations) + ")", eroded_result);
//
//    // Dilation
//    cv::Mat dilated_result = applyDilation(src, current_kernel_size, current_iterations);
//    cv::imshow("Dilated Image (Kernel: " + std::to_string(current_kernel_size) + ", Iterations: " + std::to_string(current_iterations) + ")", dilated_result);
//
//    // Opening
//    cv::Mat opening_result = applyOpening(src, current_kernel_size, current_iterations);
//    cv::imshow("Opening (Kernel: " + std::to_string(current_kernel_size) + ", Iterations: " + std::to_string(current_iterations) + ")", opening_result);
//
//    // Closing
//    cv::Mat closing_result = applyClosing(src, current_kernel_size, current_iterations);
//    cv::imshow("Closing (Kernel: " + std::to_string(current_kernel_size) + ", Iterations: " + std::to_string(current_iterations) + ")", closing_result);
//
//
//    // Example with different parameters
//    int larger_kernel_size = 9;
//    int more_iterations = 2;
//
//    cv::Mat more_eroded = applyErosion(src, larger_kernel_size, more_iterations);
//    cv::imshow("More Eroded (K: " + std::to_string(larger_kernel_size) + ", I: " + std::to_string(more_iterations) + ")", more_eroded);
//
//    cv::Mat more_dilated = applyDilation(src, larger_kernel_size, more_iterations);
//    cv::imshow("More Dilated (K: " + std::to_string(larger_kernel_size) + ", I: " + std::to_string(more_iterations) + ")", more_dilated);
//
//
//    cv::waitKey(0);
//    cv::destroyAllWindows();
//
//    return 0;
//}

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

bool partition_image(cv::Mat& image, std::vector<cv::Mat>& partition)
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
	cv::dilate(binary_lines, binary_lines, kernel, cv::Point(-1, -1), 2); // Dilate a couple of times

	// 4. Find contours in the binary image
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(binary_lines, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	// Create a copy of the original image to draw on
	cv::Mat output_image = image.clone();

	std::cout << "image before partition:" << std::endl;
	int block_count = 0;

	//cv::imshow("Image before applying partition", output_image);
	//cv::waitKey(0);

	// 5. Iterate through the found contours
	for (const auto& contour : contours) {
		// Approximate the contour to a polygon
		std::vector<cv::Point> approx_poly;
		cv::approxPolyDP(contour, approx_poly, cv::arcLength(contour, true) * 0.02, true);

		// Check if the approximated polygon has 4 vertices (a rectangle) and is sufficiently large
		// You might need to adjust the contour area threshold based on your image and expected block sizes
		if (approx_poly.size() == 4 && cv::isContourConvex(approx_poly) && cv::contourArea(contour) > (RUNE_DEFAULT_SIZE.area() / 2.0)) {
			// Get the bounding rectangle for the current contour
			cv::Rect bounding_rect = cv::boundingRect(contour);

			// Draw the bounding rectangle on the output image
			cv::rectangle(output_image, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(0, 255, 0), 2); // Green rectangle

			// List the detected block's coordinates
			std::cout << "Block " << ++block_count << ": (x=" << bounding_rect.x
				<< ", y=" << bounding_rect.y
				<< ", width=" << bounding_rect.width
				<< ", height=" << bounding_rect.height << ")" << std::endl;

			// You can also extract the individual block as a separate image
			cv::Mat extracted_block = image(bounding_rect);
			//cv::imshow("Extracted Block " + std::to_string(block_count), extracted_block); 
			//cv::imwrite("extracted_block_" + std::to_string(block_count) + ".jpg", extracted_block); // Save to file

			partition.push_back(extracted_block); // Add the extracted block to the partition vector
		}
	}

	//// Display the image with detected blocks
	//cv::imshow("Image with Detected Blocks", output_image);
	//cv::waitKey(0);

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

int generate_wav(const std::vector<std::pair<Note, float>>& notes, const fs::path& wav_file) {
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

//int generate_test_wav(const fs::path& wav_file) {
//	WaveGenerator generator;
//
//	// Define your sequence of notes and durations using the Note enum
//	generator.addNote(Note::C5, 0.5); // C5 for 0.5 seconds
//	generator.addNote(Note::D5, 0.5); // D5 for 0.5 seconds
//	generator.addNote(Note::E5, 0.5); // E5 for 0.5 seconds
//	generator.addNote(Note::F5, 0.5); // F5 for 0.5 seconds
//	generator.addNote(Note::G5, 1.0); // G5 for 1.0 seconds (longer duration)
//	generator.addNote(Note::A5, 0.25); // A5 for 0.25 seconds (shorter duration)
//	generator.addNote(Note::SILENCE, 0.25); // Add a short rest
//	generator.addNote(Note::G5, 0.75); // G5 for 0.75 seconds
//	generator.addNote(Note::C5, 1.5); // Back to C5 for a longer duration
//
//	if (generator.save(wav_file.string().c_str())) {
//		std::cout << "Successfully created melody_enum.wav" << std::endl;
//	}
//	else {
//		std::cerr << "Failed to create melody_enum.wav" << std::endl;
//	}
//
//	// You can create another one with different notes
//	WaveGenerator song_generator(44100, 16, 2); // Stereo output
//	song_generator.addNote(Note::A4, 0.8);
//	song_generator.addNote(Note::C5, 0.8);
//	song_generator.addNote(Note::E5, 0.8);
//	song_generator.addNote(Note::A5, 1.2);
//	song_generator.addNote(Note::SILENCE, 0.1); // Short rest
//	song_generator.addNote(Note::G5, 0.5);
//	song_generator.addNote(Note::C5, 0.5);
//
//	if (song_generator.save("my_song_enum.wav")) {
//		std::cout << "Successfully created my_song_enum.wav (stereo)" << std::endl;
//	}
//	else {
//		std::cerr << "Failed to create my_song_enum.wav" << std::endl;
//	}
//
//	return 0;
//}