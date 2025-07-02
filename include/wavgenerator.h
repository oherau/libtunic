#ifndef __WAVGENERATOR_H__
#define __WAVGENERATOR_H__

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
#include "note.h"

// Define PI if not available (M_PI is a common GNU extension)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- WAV File Header Structures ---
// These structs are packed to ensure correct byte alignment for file writing
#pragma pack(push, 1) // Push current packing alignment and set to 1 byte

// RIFF Header
struct WavHeaderGen {
    char riffID[4];     // Contains "RIFF"
    uint32_t fileSize;  // Size of the overall file - 8 bytes
    char waveID[4];     // Contains "WAVE"
};

// Format Subchunk
struct WavFormat {
    char fmtID[4];          // Contains "fmt " (note the space)
    uint32_t fmtSize;       // Size of the fmt subchunk (16 for PCM)
    uint16_t audioFormat;   // Audio format (1 for PCM)
    uint16_t numChannels;   // Number of channels (1 for mono, 2 for stereo)
    uint32_t sampleRate;    // Sample rate (e.g., 44100)
    uint32_t byteRate;      // Byte rate (sampleRate * numChannels * bitsPerSample / 8)
    uint16_t blockAlign;    // Block align (numChannels * bitsPerSample / 8)
    uint16_t bitsPerSample; // Bits per sample (e.g., 16)
};

// Data Subchunk
struct WavData {
    char dataID[4];     // Contains "data"
    uint32_t dataSize;  // Size of the data section
};

#pragma pack(pop) // Restore original packing alignment

class WaveGenerator {
public:
    WaveGenerator(uint32_t sampleRate = 44100, uint16_t bitsPerSample = 16, uint16_t numChannels = 1, double amplitude = 0.5);

    // Add a note to the sequence, now using the Note enum
    void addNote(Note note_enum, double duration_seconds);

    // Generate and save the WAV file
    bool save(const std::string& filename);

private:
    uint32_t sampleRate_;
    uint16_t bitsPerSample_;
    uint16_t numChannels_;
    double amplitude_; // 0.0 to 1.0

    struct NoteData { // Renamed from 'Note' to avoid conflict with enum
        double frequency;
        double duration; // in seconds
    };
    std::vector<NoteData> notes_; // Vector of NoteData structs
};

#endif // __WAVGENERATOR_H__