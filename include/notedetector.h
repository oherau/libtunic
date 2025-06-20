#ifndef __NOTEDETECTOR_H__
#define __NOTEDETECTOR_H__

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
class NoteDetector {
public:
    NoteDetector() =  default;

    void processFile(const std::filesystem::path& filePath, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH) {
        if (!std::filesystem::exists(filePath)) {
            std::cerr << "Erreur : Le fichier '" << filePath << "' n'existe pas." << std::endl;
            return;
        }

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error : Impossible to open file." << std::endl;
            return;
        }

        WavHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
        
        // Validation simple
        if (std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
            std::cerr << "Error: Invalid or unsupported file (RIFF/WAVE header is missing)." << std::endl;
            return;
        }
        if (header.format_type != 1) { // 1 = PCM
             std::cerr << "Error: Only unco;pressed WAV PCM  are supported." << std::endl;
             return;
        }

        std::cout << "File opened : " << filePath << "\n"
                  << "Sample rate: " << header.sample_rate << " Hz\n"
                  << "Channels: " << header.channels << "\n"
                  << "Bits per sample: " << header.bits_per_sample << std::endl;

        // Reading audio data
        std::vector<float> audio_samples;
        int16_t sample16;
        while (file.read(reinterpret_cast<char*>(&sample16), sizeof(int16_t))) {
            // Conversion in nor;alized float [-1.0, 1.0]
            audio_samples.push_back(static_cast<float>(sample16) / 32768.0f);
             // If stereo, the second channel is ignored
            if (header.channels == 2) {
                file.seekg(sizeof(int16_t), std::ios_base::cur);
            }
        }

        detectNoteSequence(audio_samples, header.sample_rate, detected_sequence, note_length);
    }

private:

    Note findClosestNote(double frequency) {
        if (frequency < SILENCE_TONE_THRESHOLD) return Note::SILENCE;

        double min_dist = std::numeric_limits<double>::max();
        Note best_match = Note::UNKNOWN;

        for (const auto& note : target_notes) {
            double dist = std::abs(note.second - frequency);
            if (dist < min_dist) {
                min_dist = dist;
                best_match = note.first;
            }
        }

        // Tolerance: frequency must be closer than ~3% of the target
        // Semi-tone is around 5.9%, so 3% is a good margin.
        if (min_dist / frequency < 0.03) {
            return best_match;
        }

        return Note::UNKNOWN;
    }

    void detectNoteSequence(const std::vector<float>& samples, uint32_t sampleRate, std::vector<Note>& detected_sequence, double note_length = DEFAULT_NOTE_LENGTH) {
        const double chunk_duration_s = note_length / 1000.0; // 100 ms
        const size_t samples_per_chunk = static_cast<size_t>(sampleRate * chunk_duration_s);
        // Determine FFT size (next power of 2 for best efficiency)
        size_t fft_size = 1;
        while (fft_size < samples_per_chunk) {
            fft_size *= 2;
        }
        
        std::cout << "\nDetecting note sequence (range : " << (int)note_length <<"ms)\n";

        for (size_t i = 0; i + samples_per_chunk <= samples.size(); i += samples_per_chunk) {

            std::vector<FFT::Complex> chunk(fft_size, 0.0);
            
            // Apply Hann window to reduce spectrall losses
            for (size_t j = 0; j < samples_per_chunk; ++j) {
                double hann_multiplier = 0.5 * (1 - cos(2 * std::numbers::pi * j / (samples_per_chunk - 1)));
                chunk[j] = samples[i + j] * hann_multiplier;
            }

            // Compute FFT
            FFT::fft(chunk);

            // Find peak frequency
            double max_magnitude = 0.0;
            size_t peak_index = 0;

            // Analyze only the first half of FFT result (symetrical)
            for (size_t j = 1; j < fft_size / 2; ++j) {
                double mag = std::abs(chunk[j]);
                if (mag > max_magnitude) {
                    max_magnitude = mag;
                    peak_index = j;
                }
            }

            double peak_frequency = static_cast<double>(peak_index) * sampleRate / fft_size;
            
            auto note = findClosestNote(peak_frequency);

            // to avoid infinite repetitions of "SILENCE" or "UNKNOWN"
            if (detected_sequence.empty() || detected_sequence.back() != note || (note != Note::SILENCE && note != Note::UNKNOWN)) {
                detected_sequence.push_back(note);
            }
        }
    }
public:
    std::vector<Note> get_clean_sequence(const std::vector<Note>& sequence) {
        std::vector<Note> cleanSeq;
        Note lastNote = Note::SILENCE;

        for(const auto& note : sequence) {
            if(note != lastNote) {
                cleanSeq.push_back(note);
                lastNote = note;
            }
        }

        return std::vector<Note>(cleanSeq);
    }
};

#endif // __NOTEDETECTOR_H__