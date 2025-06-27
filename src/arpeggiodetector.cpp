#include "arpeggiodetector.h"
#include <arpeggio.h>
#include <queue>
#include <cmath>
#include <algorithm>
using Complex = std::complex<double>;

void FFT::fft(std::vector<Complex>& a) {
    int n = a.size();
    if (n <= 1) return;

    std::vector<Complex> a0(n / 2), a1(n / 2);
    for (int i = 0; 2 * i < n; i++) {
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0);
    fft(a1);

    double ang = 2 * std::numbers::pi / n;
    Complex w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; i++) {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        w *= wn;
    }
}

// "Enum to string" function
std::string note_to_string(Note note, bool no_octave) {
    static const std::map<Note, std::string> note_to_string_map = {
        {Note::UNKNOWN, "UNKNOWN"},
        {Note::SILENCE, "SILENCE"},

        // Only keep fundamentals and sharp notations 
        {Note::C0, "C0"}, {Note::D_FLAT0, "Db0"}, {Note::D0, "D0"}, {Note::E_FLAT0, "Eb0"},
        {Note::E0, "E0"}, {Note::F0, "F0"}, {Note::G_FLAT0, "Gb0"}, {Note::G0, "G0"},
        {Note::A_FLAT0, "Ab0"}, {Note::A0, "A0"}, {Note::B_FLAT0, "Bb0"}, {Note::B0, "B0"},
        {Note::C1, "C1"}, {Note::D_FLAT1, "Db1"}, {Note::D1, "D1"}, {Note::E_FLAT1, "Eb1"},
        {Note::E1, "E1"}, {Note::F1, "F1"}, {Note::G_FLAT1, "Gb1"}, {Note::G1, "G1"},
        {Note::A_FLAT1, "Ab1"}, {Note::A1, "A1"}, {Note::B_FLAT1, "Bb1"}, {Note::B1, "B1"},
        {Note::C2, "C2"}, {Note::D_FLAT2, "Db2"}, {Note::D2, "D2"}, {Note::E_FLAT2, "Eb2"},
        {Note::E2, "E2"}, {Note::F2, "F2"}, {Note::G_FLAT2, "Gb2"}, {Note::G2, "G2"},
        {Note::A_FLAT2, "Ab2"}, {Note::A2, "A2"}, {Note::B_FLAT2, "Bb2"}, {Note::B2, "B2"},
        {Note::C3, "C3"}, {Note::D_FLAT3, "Db3"}, {Note::D3, "D3"}, {Note::E_FLAT3, "Eb3"},
        {Note::E3, "E3"}, {Note::F3, "F3"}, {Note::G_FLAT3, "Gb3"}, {Note::G3, "G3"},
        {Note::A_FLAT3, "Ab3"}, {Note::A3, "A3"}, {Note::B_FLAT3, "Bb3"}, {Note::B3, "B3"},
        {Note::C4, "C4"}, {Note::D_FLAT4, "Db4"}, {Note::D4, "D4"}, {Note::E_FLAT4, "Eb4"},
        {Note::E4, "E4"}, {Note::F4, "F4"}, {Note::G_FLAT4, "Gb4"}, {Note::G4, "G4"},
        {Note::A_FLAT4, "Ab4"}, {Note::A4, "A4"}, {Note::B_FLAT4, "Bb4"}, {Note::B4, "B4"},
        {Note::C5, "C5"}, {Note::D_FLAT5, "Db5"}, {Note::D5, "D5"}, {Note::E_FLAT5, "Eb5"},
        {Note::E5, "E5"}, {Note::F5, "F5"}, {Note::G_FLAT5, "Gb5"}, {Note::G5, "G5"},
        {Note::A_FLAT5, "Ab5"}, {Note::A5, "A5"}, {Note::B_FLAT5, "Bb5"}, {Note::B5, "B5"}
    };

    static const std::map<Note, std::string> note_to_string_map_simplified = {
        {Note::UNKNOWN, "UNKNOWN"},
        {Note::SILENCE, "SILENCE"},
        {Note::C0, "C"}, {Note::D_FLAT0, "D"}, {Note::D0, "D"}, {Note::E_FLAT0, "E"},
        {Note::E0, "E"}, {Note::F0, "F"}, {Note::G_FLAT0, "G"}, {Note::G0, "G"},
        {Note::A_FLAT0, "A"}, {Note::A0, "A"}, {Note::B_FLAT0, "B"}, {Note::B0, "B"},
        {Note::C1, "C"}, {Note::D_FLAT1, "D"}, {Note::D1, "D"}, {Note::E_FLAT1, "E"},
        {Note::E1, "E"}, {Note::F1, "F"}, {Note::G_FLAT1, "G"}, {Note::G1, "G"},
        {Note::A_FLAT1, "A"}, {Note::A1, "A"}, {Note::B_FLAT1, "B"}, {Note::B1, "B"},
        {Note::C2, "C"}, {Note::D_FLAT2, "D"}, {Note::D2, "D"}, {Note::E_FLAT2, "E"},
        {Note::E2, "E"}, {Note::F2, "F"}, {Note::G_FLAT2, "G"}, {Note::G2, "G"},
        {Note::A_FLAT2, "A"}, {Note::A2, "A"}, {Note::B_FLAT2, "B"}, {Note::B2, "B"},
        {Note::C3, "C"}, {Note::D_FLAT3, "D"}, {Note::D3, "D"}, {Note::E_FLAT3, "E"},
        {Note::E3, "E"}, {Note::F3, "F"}, {Note::G_FLAT3, "G"}, {Note::G3, "G"},
        {Note::A_FLAT3, "A"}, {Note::A3, "A"}, {Note::B_FLAT3, "B"}, {Note::B3, "B"},
        {Note::C4, "C"}, {Note::D_FLAT4, "D"}, {Note::D4, "D"}, {Note::E_FLAT4, "E"},
        {Note::E4, "E"}, {Note::F4, "F"}, {Note::G_FLAT4, "G"}, {Note::G4, "G"},
        {Note::A_FLAT4, "A"}, {Note::A4, "A"}, {Note::B_FLAT4, "B"}, {Note::B4, "B"},
        {Note::C5, "C"}, {Note::D_FLAT5, "D"}, {Note::D5, "D"}, {Note::E_FLAT5, "E"},
        {Note::E5, "E"}, {Note::F5, "F"}, {Note::G_FLAT5, "G"}, {Note::G5, "G"},
        {Note::A_FLAT5, "A"}, {Note::A5, "A"}, {Note::B_FLAT5, "B"}, {Note::B5, "B"}
    };

    const auto& hash_table = no_octave ? note_to_string_map_simplified : note_to_string_map;
    
    auto it = hash_table.find(note);
    if(it != hash_table.end())
        return it->second;

    return "UNKNOWN";
}

std::string notes_to_string(const std::deque<Note>& notes, bool no_octave) {
    std::stringstream ss;
    for(const auto& note : notes) {
        ss << note_to_string(note, no_octave);
    }
    return std::string(ss.str());
}

std::string remove_octave(const std::string& str) {
    std::stringstream ss;
    for(const char& c : str) {
        if(c >= 'A' && c<= 'G')
            ss << c;
    }

    return std::string(ss.str());
}

bool ArpeggioDetector::load_dictionary(const Dictionary& dictionary)
{
    std::vector<std::string> hash_list;
    dictionary.get_hash_list(hash_list);
    for(const auto& hash : hash_list) {
		Word word(hash);
        auto arpeggio = Arpeggio(word);

		m_arpeggio_dictionary[hash] = arpeggio;

		m_arpeggio_min_length = std::min(m_arpeggio_min_length, arpeggio.get_length());
		m_arpeggio_max_length = std::max(m_arpeggio_max_length, arpeggio.get_length());
	}

    return true;
}

std::vector<Note> ArpeggioDetector::get_clean_sequence(const std::vector<Note>& sequence) {
    std::vector<Note> cleanSeq;
    Note lastNote = Note::SILENCE;

    for (const auto& note : sequence) {
        if (note != lastNote) {
            cleanSeq.push_back(note);
            lastNote = note;
        }
    }

    return std::vector<Note>(cleanSeq);
}

Note ArpeggioDetector::findClosestNote(double frequency) {
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

void ArpeggioDetector::detectNoteSequence(const std::vector<float>& samples, uint32_t sampleRate, std::vector<Note>& detected_sequence, double note_length) {
    const double chunk_duration_s = note_length / 1000.0; // 100 ms
    const size_t samples_per_chunk = static_cast<size_t>(sampleRate * chunk_duration_s);
    // Determine FFT size (next power of 2 for best efficiency)
    size_t fft_size = 1;
    while (fft_size < samples_per_chunk) {
        fft_size *= 2;
    }

    std::cout << "\nDetecting note sequence (range : " << (int)note_length << "ms)\n";

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

void ArpeggioDetector::processFile(const std::filesystem::path& filePath, std::vector<Note>& detected_sequence, double note_length) {
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


// Function to get the indexed note sequence from a deque of notes
// should get arpeggio in the form of a vector of integers. with lower note index = 1
// ex: {13, 12, 6, 2, 1, 3, 6, 10, 12, 13} = "enter
std::vector<int> ArpeggioDetector::get_indexed_note_sequence(std::deque<Note>& note_sequence)
{
    auto min_note = INT_MAX;
    for (const auto& note : note_sequence) {
        if (note != Note::SILENCE && note != Note::UNKNOWN) {
            min_note = std::min(min_note, static_cast<int>(note));
        }
    }

    std::vector<int> result;
    for (const auto& note : note_sequence) {
        if (note != Note::SILENCE && note != Note::UNKNOWN) {
            result.push_back(static_cast<int>(note) - min_note + 1);
        }
        else {
            result.push_back(0); // Use -1 to represent SILENCE or UNKNOWN
        }
    }

    return std::vector<int>(result);
}


Word ArpeggioDetector::find(const Arpeggio& arpeggio)
{
    for (const auto& [hash, arpeg] : m_arpeggio_dictionary) {
        
        if (arpeggio == arpeg)
        {
			return Word(hash);
        }
    }

    return Word();
}