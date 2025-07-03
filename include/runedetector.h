#ifndef __RUNEDETECTOR_H__
#define __RUNEDETECTOR_H__

#include <filesystem>
#include <map>
#include "rune.h"
#include "word.h"
#include "dictionary.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
namespace fs = std::filesystem;

const double RUNE_MINIMAL_AREA = 100; // Minimum area for a rune to be considered valid. default 100.0f
const double RUNE_DETECTION_THRESHOLD = 0.8f; // Threshold the result to find matches - Adjust as needed. default 0.8
const char RUNE_WORD_TRANSLATION_SEPARATOR = '_'; // Separator for rune word translations

struct RuneZone { // Renamed from CharacterZone
    Word word;
    cv::Rect rect;

    // Helper for easy comparison of rects
    bool operator==(const RuneZone& other) const { // Renamed from CharacterZone
        return rect.x == other.rect.x &&
            rect.y == other.rect.y &&
            rect.width == other.rect.width &&
            rect.height == other.rect.height;
    }
};

class RuneDetector {
public:
    //RuneDetector() = default;
    RuneDetector(Dictionary* dictionary) ;
	bool load_rune_folder(const fs::path& dict_folder);
    bool register_word_image(const fs::path& word_image);
	//bool detect_runes(const fs::path& image_path, std::vector<Rune>& detected_runes);
    bool detect_words(const fs::path& image_path, std::vector<Word>& detected_words, bool debug_mode = false);
    void displayMatProperties(const cv::Mat& mat, const std::string& name = "Mat");
    bool generate_scale_factors(const cv::Mat& image, const cv::Mat& pattern, std::vector<double>& scale_factors);
    cv::Mat get_image_lines(const cv::Mat& src);
    bool decode_word_image(const fs::path& word_image, Word& word);
    cv::Mat crop_black_borders(const cv::Mat& image);
	bool dictionarize(const fs::path& image_path, bool debug_mode = false);
    bool draw_text(cv::Mat& image, const cv::Rect& bounding_box, const std::string& translation, const cv::Scalar& color, int thickness);
    int image_detection(const fs::path& dictionary_file, const fs::path& image_file);
private:
    Dictionary* m_dictionary = nullptr;
    std::unordered_map<std::string, cv::Mat> m_rune_images; // Map to store rune images
};






#endif // __RUNEDETECTOR_H__



