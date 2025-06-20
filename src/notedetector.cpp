#include "notedetector.h"
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
        // {Note::C0, "C0"}, {Note::C_SHARP0_D_FLAT0, "C#0/Db0"}, {Note::D0, "D0"}, {Note::D_SHARP0_E_FLAT0, "D#0/Eb0"},
        // {Note::E0, "E0"}, {Note::F0, "F0"}, {Note::F_SHARP0_G_FLAT0, "F#0/Gb0"}, {Note::G0, "G0"},
        // {Note::G_SHARP0_A_FLAT0, "G#0/Ab0"}, {Note::A0, "A0"}, {Note::A_SHARP0_B_FLAT0, "A#0/Bb0"}, {Note::B0, "B0"},
        // {Note::C1, "C1"}, {Note::C_SHARP1_D_FLAT1, "C#1/Db1"}, {Note::D1, "D1"}, {Note::D_SHARP1_E_FLAT1, "D#1/Eb1"},
        // {Note::E1, "E1"}, {Note::F1, "F1"}, {Note::F_SHARP1_G_FLAT1, "F#1/Gb1"}, {Note::G1, "G1"},
        // {Note::G_SHARP1_A_FLAT1, "G#1/Ab1"}, {Note::A1, "A1"}, {Note::A_SHARP1_B_FLAT1, "A#1/Bb1"}, {Note::B1, "B1"},
        // {Note::C2, "C2"}, {Note::C_SHARP2_D_FLAT2, "C#2/Db2"}, {Note::D2, "D2"}, {Note::D_SHARP2_E_FLAT2, "D#2/Eb2"},
        // {Note::E2, "E2"}, {Note::F2, "F2"}, {Note::F_SHARP2_G_FLAT2, "F#2/Gb2"}, {Note::G2, "G2"},
        // {Note::G_SHARP2_A_FLAT2, "G#2/Ab2"}, {Note::A2, "A2"}, {Note::A_SHARP2_B_FLAT2, "A#2/Bb2"}, {Note::B2, "B2"},
        // {Note::C3, "C3"}, {Note::C_SHARP3_D_FLAT3, "C#3/Db3"}, {Note::D3, "D3"}, {Note::D_SHARP3_E_FLAT3, "D#3/Eb3"},
        // {Note::E3, "E3"}, {Note::F3, "F3"}, {Note::F_SHARP3_G_FLAT3, "F#3/Gb3"}, {Note::G3, "G3"},
        // {Note::G_SHARP3_A_FLAT3, "G#3/Ab3"}, {Note::A3, "A3"}, {Note::A_SHARP3_B_FLAT3, "A#3/Bb3"}, {Note::B3, "B3"},
        // {Note::C4, "C4"}, {Note::C_SHARP4_D_FLAT4, "C#4/Db4"}, {Note::D4, "D4"}, {Note::D_SHARP4_E_FLAT4, "D#4/Eb4"},
        // {Note::E4, "E4"}, {Note::F4, "F4"}, {Note::F_SHARP4_G_FLAT4, "F#4/Gb4"}, {Note::G4, "G4"},
        // {Note::G_SHARP4_A_FLAT4, "G#4/Ab4"}, {Note::A4, "A4"}, {Note::A_SHARP4_B_FLAT4, "A#4/Bb4"}, {Note::B4, "B4"},
        // {Note::C5, "C5"}, {Note::C_SHARP5_D_FLAT5, "C#5/Db5"}, {Note::D5, "D5"}, {Note::D_SHARP5_E_FLAT5, "D#5/Eb5"},
        // {Note::E5, "E5"}, {Note::F5, "F5"}, {Note::F_SHARP5_G_FLAT5, "F#5/Gb5"}, {Note::G5, "G5"},
        // {Note::G_SHARP5_A_FLAT5, "G#5/Ab5"}, {Note::A5, "A5"}, {Note::A_SHARP5_B_FLAT5, "A#5/Bb5"}, {Note::B5, "B5"}
        // Only keep fundamentals and sharp notations 
        {Note::C0, "C0"}, {Note::C_SHARP0_D_FLAT0, "Db0"}, {Note::D0, "D0"}, {Note::D_SHARP0_E_FLAT0, "Eb0"},
        {Note::E0, "E0"}, {Note::F0, "F0"}, {Note::F_SHARP0_G_FLAT0, "Gb0"}, {Note::G0, "G0"},
        {Note::G_SHARP0_A_FLAT0, "Ab0"}, {Note::A0, "A0"}, {Note::A_SHARP0_B_FLAT0, "Bb0"}, {Note::B0, "B0"},
        {Note::C1, "C1"}, {Note::C_SHARP1_D_FLAT1, "Db1"}, {Note::D1, "D1"}, {Note::D_SHARP1_E_FLAT1, "Eb1"},
        {Note::E1, "E1"}, {Note::F1, "F1"}, {Note::F_SHARP1_G_FLAT1, "Gb1"}, {Note::G1, "G1"},
        {Note::G_SHARP1_A_FLAT1, "Ab1"}, {Note::A1, "A1"}, {Note::A_SHARP1_B_FLAT1, "Bb1"}, {Note::B1, "B1"},
        {Note::C2, "C2"}, {Note::C_SHARP2_D_FLAT2, "Db2"}, {Note::D2, "D2"}, {Note::D_SHARP2_E_FLAT2, "Eb2"},
        {Note::E2, "E2"}, {Note::F2, "F2"}, {Note::F_SHARP2_G_FLAT2, "Gb2"}, {Note::G2, "G2"},
        {Note::G_SHARP2_A_FLAT2, "Ab2"}, {Note::A2, "A2"}, {Note::A_SHARP2_B_FLAT2, "Bb2"}, {Note::B2, "B2"},
        {Note::C3, "C3"}, {Note::C_SHARP3_D_FLAT3, "Db3"}, {Note::D3, "D3"}, {Note::D_SHARP3_E_FLAT3, "Eb3"},
        {Note::E3, "E3"}, {Note::F3, "F3"}, {Note::F_SHARP3_G_FLAT3, "Gb3"}, {Note::G3, "G3"},
        {Note::G_SHARP3_A_FLAT3, "Ab3"}, {Note::A3, "A3"}, {Note::A_SHARP3_B_FLAT3, "Bb3"}, {Note::B3, "B3"},
        {Note::C4, "C4"}, {Note::C_SHARP4_D_FLAT4, "Db4"}, {Note::D4, "D4"}, {Note::D_SHARP4_E_FLAT4, "Eb4"},
        {Note::E4, "E4"}, {Note::F4, "F4"}, {Note::F_SHARP4_G_FLAT4, "Gb4"}, {Note::G4, "G4"},
        {Note::G_SHARP4_A_FLAT4, "Ab4"}, {Note::A4, "A4"}, {Note::A_SHARP4_B_FLAT4, "Bb4"}, {Note::B4, "B4"},
        {Note::C5, "C5"}, {Note::C_SHARP5_D_FLAT5, "Db5"}, {Note::D5, "D5"}, {Note::D_SHARP5_E_FLAT5, "Eb5"},
        {Note::E5, "E5"}, {Note::F5, "F5"}, {Note::F_SHARP5_G_FLAT5, "Gb5"}, {Note::G5, "G5"},
        {Note::G_SHARP5_A_FLAT5, "Ab5"}, {Note::A5, "A5"}, {Note::A_SHARP5_B_FLAT5, "Bb5"}, {Note::B5, "B5"}
    };

    static const std::map<Note, std::string> note_to_string_map_simplified = {
        {Note::UNKNOWN, "UNKNOWN"},
        {Note::SILENCE, "SILENCE"},
        {Note::C0, "C"}, {Note::C_SHARP0_D_FLAT0, "D"}, {Note::D0, "D"}, {Note::D_SHARP0_E_FLAT0, "E"},
        {Note::E0, "E"}, {Note::F0, "F"}, {Note::F_SHARP0_G_FLAT0, "G"}, {Note::G0, "G"},
        {Note::G_SHARP0_A_FLAT0, "A"}, {Note::A0, "A"}, {Note::A_SHARP0_B_FLAT0, "B"}, {Note::B0, "B"},
        {Note::C1, "C"}, {Note::C_SHARP1_D_FLAT1, "D"}, {Note::D1, "D"}, {Note::D_SHARP1_E_FLAT1, "E"},
        {Note::E1, "E"}, {Note::F1, "F"}, {Note::F_SHARP1_G_FLAT1, "G"}, {Note::G1, "G"},
        {Note::G_SHARP1_A_FLAT1, "A"}, {Note::A1, "A"}, {Note::A_SHARP1_B_FLAT1, "B"}, {Note::B1, "B"},
        {Note::C2, "C"}, {Note::C_SHARP2_D_FLAT2, "D"}, {Note::D2, "D"}, {Note::D_SHARP2_E_FLAT2, "E"},
        {Note::E2, "E"}, {Note::F2, "F"}, {Note::F_SHARP2_G_FLAT2, "G"}, {Note::G2, "G"},
        {Note::G_SHARP2_A_FLAT2, "A"}, {Note::A2, "A"}, {Note::A_SHARP2_B_FLAT2, "B"}, {Note::B2, "B"},
        {Note::C3, "C"}, {Note::C_SHARP3_D_FLAT3, "D"}, {Note::D3, "D"}, {Note::D_SHARP3_E_FLAT3, "E"},
        {Note::E3, "E"}, {Note::F3, "F"}, {Note::F_SHARP3_G_FLAT3, "G"}, {Note::G3, "G"},
        {Note::G_SHARP3_A_FLAT3, "A"}, {Note::A3, "A"}, {Note::A_SHARP3_B_FLAT3, "B"}, {Note::B3, "B"},
        {Note::C4, "C"}, {Note::C_SHARP4_D_FLAT4, "D"}, {Note::D4, "D"}, {Note::D_SHARP4_E_FLAT4, "E"},
        {Note::E4, "E"}, {Note::F4, "F"}, {Note::F_SHARP4_G_FLAT4, "G"}, {Note::G4, "G"},
        {Note::G_SHARP4_A_FLAT4, "A"}, {Note::A4, "A"}, {Note::A_SHARP4_B_FLAT4, "B"}, {Note::B4, "B"},
        {Note::C5, "C"}, {Note::C_SHARP5_D_FLAT5, "D"}, {Note::D5, "D"}, {Note::D_SHARP5_E_FLAT5, "E"},
        {Note::E5, "E"}, {Note::F5, "F"}, {Note::F_SHARP5_G_FLAT5, "G"}, {Note::G5, "G"},
        {Note::G_SHARP5_A_FLAT5, "A"}, {Note::A5, "A"}, {Note::A_SHARP5_B_FLAT5, "B"}, {Note::B5, "B"}
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