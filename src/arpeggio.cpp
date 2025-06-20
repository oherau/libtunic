#include "arpeggio.h"

std::string Arpeggio::to_string(unsigned long elem) {

    // switch(elem) {
    //     case ARPEGGIO_VOWEL_A   :
    //         return "A";
    //     case ARPEGGIO_VOWEL_O   :
    //         return "O";
    //     case ARPEGGIO_VOWEL_I   :
    //         return "I";
    //     case ARPEGGIO_VOWEL_E   :
    //         return "E";
    //     case ARPEGGIO_VOWEL_OO_O:
    //         return "OO_O";
    //     case ARPEGGIO_VOWEL_ER  :
    //         return "ER";
    //     case ARPEGGIO_VOWEL_EE  :
    //         return "EE";
    //     case ARPEGGIO_VOWEL_OO_U:
    //         return "OO_U";
    //     case ARPEGGIO_VOWEL_IR  :
    //         return "IR";
    //     case ARPEGGIO_VOWEL_OOR :
    //         return "OOR";
    //     case ARPEGGIO_VOWEL_AR  :
    //         return "AR";
    //     case ARPEGGIO_VOWEL_ERE :
    //         return "ERE";
    //     case ARPEGGIO_VOWEL_AI  :
    //         return "AI";
    //     case ARPEGGIO_VOWEL_Y   :
    //         return "Y";
    //     case ARPEGGIO_VOWEL_U   :
    //         return "U";
    //     case ARPEGGIO_VOWEL_OW_H:
    //         return "OW_H";
    //     case ARPEGGIO_VOWEL_OW_S:
    //         return "OW_S";
    //     case ARPEGGIO_VOWEL_AIR :
    //         return "AIR";


    //     case ARPEGGIO_CONSONANT_B :
    //         return "B";
    //     case ARPEGGIO_CONSONANT_CH:
    //         return "CH";
    //     case ARPEGGIO_CONSONANT_D :
    //         return "D";
    //     case ARPEGGIO_CONSONANT_F :
    //         return "F";
    //     case ARPEGGIO_CONSONANT_G :
    //         return "G";
    //     case ARPEGGIO_CONSONANT_H :
    //         return "H";
    //     case ARPEGGIO_CONSONANT_J :
    //         return "J";
    //     case ARPEGGIO_CONSONANT_K :
    //         return "K";
    //     case ARPEGGIO_CONSONANT_L :
    //         return "L";
    //     case ARPEGGIO_CONSONANT_M :
    //         return "M";
    //     case ARPEGGIO_CONSONANT_N :
    //         return "N";
    //     case ARPEGGIO_CONSONANT_NG:
    //         return "NG";
    //     case ARPEGGIO_CONSONANT_P :
    //         return "P";
    //     case ARPEGGIO_CONSONANT_R :
    //         return "R";
    //     case ARPEGGIO_CONSONANT_S :
    //         return "S";
    //     case ARPEGGIO_CONSONANT_SH:
    //         return "SH";
    //     case ARPEGGIO_CONSONANT_T :
    //         return "T";
    //     case ARPEGGIO_CONSONANT_TH_S:
    //         return "TH_S";
    //     case ARPEGGIO_CONSONANT_TH_Z:
    //         return "TH_Z";
    //     case ARPEGGIO_CONSONANT_V :
    //         return "V";
    //     case ARPEGGIO_CONSONANT_W :
    //         return "W";
    //     case ARPEGGIO_CONSONANT_Y :
    //         return "Y";
    //     case ARPEGGIO_CONSONANT_Z :
    //         return "Z";
    //     case ARPEGGIO_CONSONANT_ZH:
    //         return "ZH";
    //}
    return "";
} 

std::string Arpeggio::to_string() {
    if(reverse)
        return Arpeggio::to_string(vowel) + Arpeggio::to_string(consonant);
    return Arpeggio::to_string(consonant) + Arpeggio::to_string(vowel);
}