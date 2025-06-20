#ifndef __RUNE_H__
#define __RUNE_H__

#include <string>

/*
* BIT representation of the TUNIC RUNE DECODER
BASED ON THIS DECODER NOTATION
https://lunar.exchange/tunic-decoder/#

                --
         1   -- -- --   2
          --    --    --
       --       --6      --
       -- --    --    --
       --   3-- -- --4
     5 --       --
       --       --8
       --       --
     ------------------------
       --       --
       --       --
     13--       --
       --   9-- -- --10
       -- --    --    --
       --       --       --
          --    --    --
           11-- -- --12
                --
                00
              00  00 16
                00

                
VOWELS
          /  \
       1 /    \ 2
        /      \
       |
     5 |
       |
     ------------------
       |
    13 |
       |
        \       /
      11 \     / 12
          \   /
            O
            

CONSONANTS
            7
         \  |  /
        8 \ | / 6
           \|/
            |
     ------------------
            |
           /|\
        9 / | \ 11
         /  |  \
            10

*/


// MASKS
const auto RUNE_REVERSE         = 0x8000;
const auto RUNE_VOWEL           = 0x1c13;
const auto RUNE_CONSONANT       = 0x23ac;
const auto RUNE_NULL            = 0x0000;

// VOWELS
const auto RUNE_VOWEL_A         = 0x1013;
const auto RUNE_VOWEL_AR        = 0x0c03;
const auto RUNE_VOWEL_AH        = 0x1011;
const auto RUNE_VOWEL_AY        = 0x0001;
const auto RUNE_VOWEL_E         = 0x1c10;
const auto RUNE_VOWEL_EE        = 0x1c11;
const auto RUNE_VOWEL_EER       = 0x1811;
const auto RUNE_VOWEL_EH        = 0x0003;
const auto RUNE_VOWEL_ERE       = 0x1810;
const auto RUNE_VOWEL_I         = 0x0c00;
const auto RUNE_VOWEL_IE        = 0x0002;
const auto RUNE_VOWEL_IR        = 0x1c12;
const auto RUNE_VOWEL_OH        = 0x1c13;
const auto RUNE_VOWEL_OI        = 0x0400;
const auto RUNE_VOWEL_OO        = 0x1413;
const auto RUNE_VOWEL_OU        = 0x1410;
const auto RUNE_VOWEL_OW        = 0x0800;
const auto RUNE_VOWEL_ORE       = 0x1813;

// CONSONANTS
const auto RUNE_CONSONANT_B     = 0x02a0;
const auto RUNE_CONSONANT_CH    = 0x2084;
const auto RUNE_CONSONANT_D     = 0x03a0;
const auto RUNE_CONSONANT_F     = 0x2188;
const auto RUNE_CONSONANT_G     = 0x2288;
const auto RUNE_CONSONANT_H     = 0x22a0;
const auto RUNE_CONSONANT_J     = 0x01a0;
const auto RUNE_CONSONANT_K     = 0x02a8;
const auto RUNE_CONSONANT_L     = 0x20a0;
const auto RUNE_CONSONANT_M     = 0x0300;
const auto RUNE_CONSONANT_N     = 0x0304;
const auto RUNE_CONSONANT_NG    = 0x23ac;
const auto RUNE_CONSONANT_P     = 0x2088;
const auto RUNE_CONSONANT_R     = 0x20a8;
const auto RUNE_CONSONANT_S     = 0x21a8;
const auto RUNE_CONSONANT_SH    = 0x238c;
const auto RUNE_CONSONANT_T     = 0x208c;
const auto RUNE_CONSONANT_THS   = 0x20ac;
const auto RUNE_CONSONANT_THZ   = 0x23a0;
const auto RUNE_CONSONANT_V     = 0x02a4;
const auto RUNE_CONSONANT_W     = 0x000c;
const auto RUNE_CONSONANT_Y     = 0x20a4;
const auto RUNE_CONSONANT_Z     = 0x22a4;
const auto RUNE_CONSONANT_ZH    = 0x03ac;


class Rune {
public:
    Rune() = default;
    Rune(unsigned long bin);
    //Rune(std::string name);
    Rune(const Rune& rune);
    std::string to_pseudophonetic() const;
	std::string to_hexa() const;
	bool from_hexa(const std::string& hexString);
    
	//operator overloads
    Rune operator+(const Rune& other) const { return Rune(m_rune | other.m_rune);}
	bool operator<(const Rune& other) const { return m_rune < other.m_rune; }
    bool operator==(const Rune& other) const { return m_rune == other.m_rune; }
    std::string get_hexa() const;
private:
    unsigned long m_rune;
    std::string to_string(unsigned long bin) const;
};

#endif // __RUNE_H__