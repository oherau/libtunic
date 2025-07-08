#ifndef __TOOLBOX_H__
#define __TOOLBOX_H__

#include <string>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <filesystem>
#include "note.h"
namespace fs = std::filesystem;

const int RUNE_IMAGE_BINARY_FILTER_THRESOLD = 128; // Threshold for binary filter on rune images
const auto MAX_IMAGE_DETECTION_DIMENSIONS = cv::Size(1280, 900);

// Define the constant vector of 3-character language codes
const std::vector<std::string> language_codes_3_char = {
    "bul", // Bulgarian
    "hrv", // Croatian
    "ces", // Czech
    "dan", // Danish
    "nld", // Dutch
    "eng", // English
    "est", // Estonian
    "fin", // Finnish
    "fra", // French
    "deu", // German
    "ell", // Greek
    "hun", // Hungarian
    "gle", // Irish
    "ita", // Italian
    "lav", // Latvian
    "lit", // Lithuanian
    "mlt", // Maltese
    "pol", // Polish
    "por", // Portuguese
    "ron", // Romanian
    "slk", // Slovak
    "slv", // Slovenian
    "spa", // Spanish
    "swe"  // Swedish
};


bool find_horizontal_separator(const cv::Mat& binary_image, int& line_center_y, int& separator_tickness);
bool find_horizontal_separator_bounds(const cv::Mat& binary_image, int line_center_y, int& line_center_x_min, int& line_center_x_max);
bool crop_borders(const cv::Mat& image, int line_center_y, int line_center_x_min, int line_center_x_max, cv::Mat& cropped_image);
bool make_white_rune_black_background(cv::Mat& image);

cv::Mat applyErosion(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyDilation(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyOpening(const cv::Mat& input_image, int kernel_size, int iterations = 1);
cv::Mat applyClosing(const cv::Mat& input_image, int kernel_size, int iterations = 1);

std::vector<std::string> loadLinesFromFile(const fs::path& file);
bool partition_image(cv::Mat& image, std::vector<cv::Rect>& partition);

double frequencyToMidiNote(double frequency_hz);

//int generate_test_wav(const fs::path& wav_file);
int generate_wav(const std::vector<std::pair<Note, float>>& notes, const fs::path& wav_file);

std::string translate(const std::string& textToTranslate,
    const std::string& sourceLanguageCode,
    const std::string& targetLanguageCode);
bool translate_dictionary(const fs::path& rune_dictionary_file, const fs::path& lang_dictionnary_file);


void toLowerFast(std::string& s);
std::string toLowerFastCopy(const std::string& s);

void resize_to_fit_max_bounds(cv::Mat& image, const cv::Size& max_bounds);

#endif // __TOOLBOX_H__