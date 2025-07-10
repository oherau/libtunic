// Sample code
#include <opencv2/opencv.hpp> // Includes most common OpenCV functionalities
#include <iostream>
#include <vector>
#include <queue>
#include <filesystem>
#include <string>
#include "arpeggiodetector.h"
#include "runedetector.h"
#include "runedictionary.h"
//#include "libtuneic.h"
namespace fs = std::filesystem;

// catch2 mocking
#define CHECK( x ) { if(!(x)) { std::cout << ANSI::RED << " [ KO ] " << #x << ANSI::RESET << std::endl;} else { std::cout << ANSI::GREEN << " [ OK ] " << #x << ANSI::RESET << std::endl;}  }

void PRINT_TEST_HEADER(const std::string& title) { 
    const std::string sep =  std::string(80, '='); 
    int offset = (sep.size() - std::string(title).size()) / 2 ; 
    std::cout << std::endl << std::endl << sep << std::endl << std::string(offset, ' ') << title << std::endl << sep << std::endl << std::endl;
}

// Platform-specific headers
#ifdef _WIN32
#include <windows.h> // For GetModuleFileNameW
#elif defined(__linux__)
#include <unistd.h>  // For readlink
#include <limits.h>  // For PATH_MAX
#elif defined(__APPLE__)
#include <mach-o/dyld.h> // For _NSGetExecutablePath
#include <limits.h>  // For PATH_MAX
#endif
#include <toolbox.h>
#include <wavgenerator.h>
#include "toolbox.cpp"

namespace fs = std::filesystem;

fs::path getExecutablePath() {
#ifdef _WIN32
    // On Windows, use GetModuleFileNameW for wide characters
    wchar_t path[MAX_PATH];
    DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        // Handle error or buffer too small
        return fs::path(); // Return an empty path
    }
    return fs::path(path);
#elif defined(__linux__)
    // On Linux, use /proc/self/exe symlink
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count == -1) {
        // Handle error
        return fs::path(); // Return an empty path
    }
    return fs::path(std::string(path, count));
#elif defined(__APPLE__)
    // On macOS, use _NSGetExecutablePath
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        // Handle buffer too small or other error
        // You might need to reallocate and try again if _NSGetExecutablePath returns non-zero
        return fs::path(); // Return an empty path
    }
    return fs::path(path);
#else
    // Fallback for other/unknown platforms or if you don't need this functionality everywhere
    std::cerr << "Warning: getExecutablePath() not implemented for this platform." << std::endl;
    return fs::current_path(); // Return current working directory as a fallback
#endif
}


const auto RUNES_FOLDER = fs::path("../../../data/runes");
const auto DICTIONARY_ENG = fs::path("../../../lang/dictionary.eng.txt");
const auto DICTIONARY_FRA = fs::path("../../../lang/dictionary.fra.txt");

int main(int argc, char* argv[]) {

    bool yin_algo = true;

    if (argc != 2 && argc != 3) {
        std::cerr << "Usage: "
            << argv[0] << " <file.wav> [note_length_millisec=75]>" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];

    int detection_result = -1;

    if (fs::path(input_file).extension() == ".wav") {
        double note_length = 75;
        if (argc >= 3) {
            note_length = std::stod(argv[2]);
        }
        ArpeggioDetector audio_detector;
		std::string result;
        int detection_result = audio_detector.audio_detection(DICTIONARY_ENG, fs::path(input_file), note_length, yin_algo, result);
    }
    if (fs::path(input_file).extension() == ".jpg") {

        RuneDictionary rune_dictionary(DICTIONARY_ENG);
        rune_dictionary.save(DICTIONARY_ENG);
        rune_dictionary.generate_images(RUNES_FOLDER);
        RuneDetector rune_detector(&rune_dictionary);
        rune_detector.load_rune_folder(RUNES_FOLDER);

        auto output_file = fs::path(input_file).parent_path() / (fs::path(input_file).stem().string() + std::string("_decrypted") + fs::path(input_file).extension().string());

        int detection_result = rune_detector.image_detection(input_file, output_file, 7, false, false);
    }

    return detection_result;
}
