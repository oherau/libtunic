#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include <string>

// bits describing arpeggio sequence
// C	D	Eb	G	Ab2	C2	D2	Eb2	G2	Ab3	C3

const auto NOTE_C   = 0b10000000000;
const auto NOTE_D   = 0b01000000000;
const auto NOTE_Eb  = 0b00100000000;
const auto NOTE_G   = 0b00010000000;
const auto NOTE_Ab2 = 0b00001000000;
const auto NOTE_C2  = 0b00000100000;
const auto NOTE_D2  = 0b00000010000;
const auto NOTE_Eb2 = 0b00000001000;
const auto NOTE_G2  = 0b00000000100;
const auto NOTE_Ab3 = 0b00000000010;
const auto NOTE_C3  = 0b00000000001;


const auto ARPEGGIO_VOWEL_A         = NOTE_Ab3 | NOTE_C3; // cat
const auto ARPEGGIO_VOWEL_AR        = NOTE_Eb2 | NOTE_Ab3; // far
const auto ARPEGGIO_VOWEL_AW        = NOTE_Eb2 | NOTE_G2 | NOTE_Ab3; // swan
const auto ARPEGGIO_VOWEL_AY        = NOTE_D2 | NOTE_G2 | NOTE_C3; // bay
const auto ARPEGGIO_VOWEL_E         = NOTE_G2 | NOTE_Ab3; // bed
const auto ARPEGGIO_VOWEL_EE        = NOTE_D2 | NOTE_Eb2 | NOTE_Ab3; // sheep
const auto ARPEGGIO_VOWEL_EER       = NOTE_G2 | NOTE_C3; // beer

// // TO BE CHECKED
// const auto ARPEGGIO_VOWEL_O         = 0b011000000000; // on
// const auto ARPEGGIO_VOWEL_I         = 0b000110000000; // ship

// const auto ARPEGGIO_VOWEL_OO_O      = 0b001100000000; // good
// const auto ARPEGGIO_VOWEL_ER        = 0b110000000000; // teacher
// const auto ARPEGGIO_VOWEL_EER        = 0b110000000000; // teacher


// const auto ARPEGGIO_VOWEL_OO_U      = 0b111100000000; // shoot
// const auto ARPEGGIO_VOWEL_IR        = 0b101110000000; // bird
// const auto ARPEGGIO_VOWEL_OOR       = 0b111010000000; // door

// const auto ARPEGGIO_VOWEL_ERE       = 0b011010000000; // here

// const auto ARPEGGIO_VOWEL_AI        = 0b010000000000; // wait
// const auto ARPEGGIO_VOWEL_Y         = 0b100000000000; // my
// const auto ARPEGGIO_VOWEL_U         = 0b000100000000; // up
// const auto ARPEGGIO_VOWEL_OW_H      = 0b000010000000; // cow
// const auto ARPEGGIO_VOWEL_OW_S      = 0b111110000000; // show
// const auto ARPEGGIO_VOWEL_AIR       = 0b001010000000; // hair



// //                                  _____||||||_
// const auto ARPEGGIO_CONSONANT_M     = 0b000000001010;
// const auto ARPEGGIO_CONSONANT_N     = 0b000000011010;
// const auto ARPEGGIO_CONSONANT_NG    = 0b000000111110;
// const auto ARPEGGIO_CONSONANT_P     = 0b000001000100;
// const auto ARPEGGIO_CONSONANT_B     = 0b000000100010;
// const auto ARPEGGIO_CONSONANT_T     = 0b000001010100;


// const auto ARPEGGIO_CONSONANT_D     = 0b000000101010;
// const auto ARPEGGIO_CONSONANT_K     = 0b000001100010;
// const auto ARPEGGIO_CONSONANT_G     = 0b000001000010;
// const auto ARPEGGIO_CONSONANT_J     = 0b000000101000;
// const auto ARPEGGIO_CONSONANT_CH    = 0b000000010100;
// const auto ARPEGGIO_CONSONANT_F     = 0b000001001100;

// const auto ARPEGGIO_CONSONANT_V     = 0b000000110010;
// const auto ARPEGGIO_CONSONANT_TH_Z  = 0b000000101110;
// const auto ARPEGGIO_CONSONANT_TH_S  = 0b000001110100;
// const auto ARPEGGIO_CONSONANT_S     = 0b000001101100;
// const auto ARPEGGIO_CONSONANT_Z     = 0b000000110110;
// const auto ARPEGGIO_CONSONANT_SH    = 0b000001011110;

// const auto ARPEGGIO_CONSONANT_ZH    = 0b000001111010;
// const auto ARPEGGIO_CONSONANT_H     = 0b000000100110;
// const auto ARPEGGIO_CONSONANT_R     = 0b000001100100;
// const auto ARPEGGIO_CONSONANT_Y     = 0b000000110100;
// const auto ARPEGGIO_CONSONANT_W     = 0b000001010000;
// const auto ARPEGGIO_CONSONANT_L     = 0b000000100100;



class Arpeggio {
public:
    Arpeggio() = default;
};

#endif // __ARPEGGIO_H__