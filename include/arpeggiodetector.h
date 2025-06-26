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
#include <dictionary.h>
#include <arpeggio.h>

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

// Hash table (std::map) to store note frequencies
const std::map<Note, double> target_notes = {
    // Octave 0
    {Note::C0, 16.35}, {Note::C_SHARP0_D_FLAT0, 17.32}, {Note::D0, 18.35}, {Note::D_SHARP0_E_FLAT0, 19.45},
    {Note::E0, 20.60}, {Note::F0, 21.83}, {Note::F_SHARP0_G_FLAT0, 23.12}, {Note::G0, 24.50},
    {Note::G_SHARP0_A_FLAT0, 25.96}, {Note::A0, 27.50}, {Note::A_SHARP0_B_FLAT0, 29.14}, {Note::B0, 30.87},
    // Octave 1
    {Note::C1, 32.70}, {Note::C_SHARP1_D_FLAT1, 34.65}, {Note::D1, 36.71}, {Note::D_SHARP1_E_FLAT1, 38.89},
    {Note::E1, 41.20}, {Note::F1, 43.65}, {Note::F_SHARP1_G_FLAT1, 46.25}, {Note::G1, 49.00},
    {Note::G_SHARP1_A_FLAT1, 51.91}, {Note::A1, 55.00}, {Note::A_SHARP1_B_FLAT1, 58.27}, {Note::B1, 61.74},
    // Octave 2
    {Note::C2, 65.41}, {Note::C_SHARP2_D_FLAT2, 69.30}, {Note::D2, 73.42}, {Note::D_SHARP2_E_FLAT2, 77.78},
    {Note::E2, 82.41}, {Note::F2, 87.31}, {Note::F_SHARP2_G_FLAT2, 92.50}, {Note::G2, 98.00},
    {Note::G_SHARP2_A_FLAT2, 103.83}, {Note::A2, 110.00}, {Note::A_SHARP2_B_FLAT2, 116.54}, {Note::B2, 123.47},
    // Octave 3
    {Note::C3, 130.81}, {Note::C_SHARP3_D_FLAT3, 138.59}, {Note::D3, 146.83}, {Note::D_SHARP3_E_FLAT3, 155.56},
    {Note::E3, 164.81}, {Note::F3, 174.61}, {Note::F_SHARP3_G_FLAT3, 185.00}, {Note::G3, 196.00},
    {Note::G_SHARP3_A_FLAT3, 207.65}, {Note::A3, 220.00}, {Note::A_SHARP3_B_FLAT3, 233.08}, {Note::B3, 246.94},
    // Octave 4
    {Note::C4, 261.63}, {Note::C_SHARP4_D_FLAT4, 277.18}, {Note::D4, 293.66}, {Note::D_SHARP4_E_FLAT4, 311.13},
    {Note::E4, 329.63}, {Note::F4, 349.23}, {Note::F_SHARP4_G_FLAT4, 369.99}, {Note::G4, 392.00},
    {Note::G_SHARP4_A_FLAT4, 415.30}, {Note::A4, 440.00}, {Note::A_SHARP4_B_FLAT4, 466.16}, {Note::B4, 493.88},
    // Octave 5
    {Note::C5, 523.25}, {Note::C_SHARP5_D_FLAT5, 554.37}, {Note::D5, 587.33}, {Note::D_SHARP5_E_FLAT5, 622.25},
    {Note::E5, 659.26}, {Note::F5, 698.46}, {Note::F_SHARP5_G_FLAT5, 739.99}, {Note::G5, 783.99},
    {Note::G_SHARP5_A_FLAT5, 830.61}, {Note::A5, 880.00}, {Note::A_SHARP5_B_FLAT5, 932.33}, {Note::B5, 987.77}
};

// "Enum to string" function
std::string note_to_string(Note note, bool no_octave = false);
std::string notes_to_string(const std::deque<Note>& notes, bool no_octave = false);
std::string remove_octave(const std::string& str);

// Detector main class
class ArpeggioDetector {
public:
    ArpeggioDetector() =  default;
    bool load_dictionary(const Dictionary& dictionary);
    void processFile(const std::filesystem::path& filePath, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH);
    std::vector<Note> get_clean_sequence(const std::vector<Note>& sequence);
    bool detect_runes(std::vector<int>& arpeggio_sequence, std::vector<Rune>& detected_sequence);
private:
    Note findClosestNote(double frequency);
    void detectNoteSequence(const std::vector<float>& samples, uint32_t sampleRate, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH);
    std::map<std::string, Arpeggio> m_arpeggio_dictionary;
    int m_arpeggio_min_length = INT_MAX;
	int m_arpeggio_max_length = 0;
};

#endif // __NOTEDETECTOR_H__