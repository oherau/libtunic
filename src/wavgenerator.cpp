#include "wavgenerator.h"
#include <note.h>

WaveGenerator::WaveGenerator(uint32_t sampleRate, uint16_t bitsPerSample, uint16_t numChannels, double amplitude)
	: sampleRate_(sampleRate), bitsPerSample_(bitsPerSample), numChannels_(numChannels),
	amplitude_(amplitude) // Default amplitude, adjust as needed (0.0 to 1.0)
{
	if (bitsPerSample_ != 16) {
		std::cerr << "Warning: Only 16-bit samples are fully supported in this example." << std::endl;
	}
}

// Add a note to the sequence, now using the Note enum
void WaveGenerator::addNote(Note note_enum, double duration_seconds) {
	auto it = note_frequencies.find(note_enum);
	if (it != note_frequencies.end()) {
		notes_.push_back({ it->second, duration_seconds });
	}
	else {
		// This should ideally not happen if using the enum correctly
		std::cerr << "Error: Unknown note enum value. Skipping." << std::endl;
	}
}

// Generate and save the WAV file
bool WaveGenerator::save(const std::string& filename) {
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Error: Could not open file " << filename << std::endl;
		return false;
	}

	// Calculate total number of samples
	size_t total_samples = 0;
	for (const auto& note : notes_) {
		total_samples += static_cast<size_t>(note.duration * sampleRate_);
	}

	// Calculate sizes
	uint32_t data_size = total_samples * (bitsPerSample_ / 8) * numChannels_;
	uint32_t file_size = data_size + sizeof(WavHeaderGen) + sizeof(WavFormat) + sizeof(WavData) - 8; // -8 for RIFF/WAVE IDs

	// Write WAV Header
	WavHeaderGen header;
	std::memcpy(header.riffID, "RIFF", 4);
	header.fileSize = file_size;
	std::memcpy(header.waveID, "WAVE", 4);
	file.write(reinterpret_cast<const char*>(&header), sizeof(header));

	// Write Format Subchunk
	WavFormat format;
	std::memcpy(format.fmtID, "fmt ", 4);
	format.fmtSize = 16; // PCM format size
	format.audioFormat = 1; // PCM
	format.numChannels = numChannels_;
	format.sampleRate = sampleRate_;
	format.bitsPerSample = bitsPerSample_;
	format.byteRate = sampleRate_ * numChannels_ * (bitsPerSample_ / 8);
	format.blockAlign = numChannels_ * (bitsPerSample_ / 8);
	file.write(reinterpret_cast<const char*>(&format), sizeof(format));

	// Write Data Subchunk header
	WavData data_chunk;
	std::memcpy(data_chunk.dataID, "data", 4);
	data_chunk.dataSize = data_size;
	file.write(reinterpret_cast<const char*>(&data_chunk), sizeof(data_chunk));

	// Generate and write audio samples
	long double current_time_offset = 0.0; // To keep track of the cumulative time
	for (const auto& note : notes_) {
		size_t samples_for_note = static_cast<size_t>(note.duration * sampleRate_);
		for (size_t i = 0; i < samples_for_note; ++i) {
			// Time for the current sample within the entire sequence
			double t = (static_cast<double>(i) / sampleRate_) + current_time_offset;

			double sample_value;
			if (note.frequency == 0.0) { // Handle rests (frequency 0)
				sample_value = 0.0;
			}
			else {
				// Simple sine wave generation
				sample_value = amplitude_ * std::sin(2 * M_PI * note.frequency * t);
			}

			// Convert to 16-bit integer (int16_t)
			// Range for int16_t is -32768 to 32767
			int16_t pcm_sample = static_cast<int16_t>(sample_value * (std::numeric_limits<int16_t>::max()));
			file.write(reinterpret_cast<const char*>(&pcm_sample), sizeof(pcm_sample));

			// For stereo, you'd write a second sample here
			if (numChannels_ == 2) {
				file.write(reinterpret_cast<const char*>(&pcm_sample), sizeof(pcm_sample)); // Simple stereo (duplicate mono)
			}
		}
		current_time_offset += note.duration; // Accumulate duration for next note's time calculation
	}

	file.close();
	return true;
}

