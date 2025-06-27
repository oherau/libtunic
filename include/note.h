#include <vector>
#ifndef __NOTE_H__
#define __NOTE_H__

#include <deque>

// Enum definition for all musical notes
enum class Note {
    UNKNOWN,
    SILENCE,
    C0, D_FLAT0, D0, E_FLAT0, E0, F0, G_FLAT0, G0, A_FLAT0, A0, B_FLAT0, B0,
    C1, D_FLAT1, D1, E_FLAT1, E1, F1, G_FLAT1, G1, A_FLAT1, A1, B_FLAT1, B1,
    C2, D_FLAT2, D2, E_FLAT2, E2, F2, G_FLAT2, G2, A_FLAT2, A2, B_FLAT2, B2,
    C3, D_FLAT3, D3, E_FLAT3, E3, F3, G_FLAT3, G3, A_FLAT3, A3, B_FLAT3, B3,
    C4, D_FLAT4, D4, E_FLAT4, E4, F4, G_FLAT4, G4, A_FLAT4, A4, B_FLAT4, B4,
    C5, D_FLAT5, D5, E_FLAT5, E5, F5, G_FLAT5, G5, A_FLAT5, A5, B_FLAT5, B5
};

#endif // __NOTE_H__