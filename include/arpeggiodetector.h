#ifndef __ARPEGGIODETECTOR_H__
#define __ARPEGGIODETECTOR_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <string>
#include <cmath>
#include <complex>
#include <cstdint>
#include <map>
#include <algorithm>
#include <filesystem>
#include <numbers> // for Pi
#include "note.h"
#include "runedictionary.h"
#include "arpeggio.h"

const auto DEFAULT_NOTE_LENGTH = 75.0f; // in millisec
const auto SILENCE_TONE_THRESHOLD = 10.0f;


// --- Structures WavHeader ---
// (Simplified for standard PCM files)
struct WavHeader {
    char riff[4]; // "RIFF"
    uint32_t overall_size;
    char wave[4]; // "WAVE"
    char fmt_chunk_marker[4]; // "fmt "
    uint32_t length_of_fmt;
    uint16_t format_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_chunk_header[4]; // "data"
    uint32_t data_size;
};

namespace FFT {
    using Complex = std::complex<double>;
    void fft(std::vector<Complex>& a) ;
}

// "Enum to string" function
std::string note_to_string(Note note, bool no_octave = false);
std::string notes_to_string(const std::deque<Note>& notes, bool no_octave = false);
std::string remove_octave(const std::string& str);

// Detector main class
class ArpeggioDetector {
public:
    ArpeggioDetector() =  default;
    bool load_dictionary(const RuneDictionary& dictionary);
    std::vector<Note> detect_note_sequence(const std::filesystem::path& filePath, double note_length = DEFAULT_NOTE_LENGTH, bool yin_algo = false);
    //std::vector<Note> detect_note_sequence_aubio(const fs::path& wavFilePath, float threshold = 0.7f, int minNote = (int)Note::C0, int maxNote = (int)Note::B8);
    std::vector<Note> get_clean_sequence(const std::vector<Note>& sequence);
    Word find(const Arpeggio& arpeggio);
	//std::vector<Rune> get_runes_from_arpeggio(const Arpeggio& arpeggio);
    static std::vector<int> get_indexed_note_sequence(std::vector<Note>& note_sequence, ScaleType scale = ScaleType::Mixolydian);
    bool detect_words(const std::vector<Note>& notes, std::vector<Word>& words);
    int audio_detection(const fs::path& dictionary_file, const fs::path& audio_file, double note_length, bool yin_algo, std::string& result);
private:
    Note findClosestNote(double frequency);
    void detect_note_sequence(const std::vector<float>& samples, uint32_t sampleRate, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH);
    std::map<std::string, Arpeggio> m_arpeggio_dictionary;
    int m_arpeggio_min_length = INT_MAX;
	int m_arpeggio_max_length = 0;
};

#endif // __NOTEDETECTOR_H__