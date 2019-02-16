#ifndef ZDCPolarimetryPlotsNames_H
#define ZDCPolarimetryPlotsNames_H

enum {kNbunch = 120}; // number of bunches at RHIC
enum {kNspin = 5}; // number of possible spin combinations at a bunch crossing
// for transverse running it is 4: (Yellow-Blue) 1=up-up, 2=up-down, 3=down-up, 4=down-down
// for longitudinal running it is 4: (Yellow-Blue) 1=plus-plus, 2=plus-minus, 3=minus-plus, 4=minus-minus
// 0=unknown/other

#define spinName "spin"
#define bxName "bx"
#define bxspinName "bxspin"
#define bxspinYName "bxspinY"
#define bxspinBName "bxspinB"

#define yellowPolName "yellowPol"
#define yellowPolWName "yellowPolW"
#define bluePolName "bluePol"
#define bluePolWName "bluePolW"

#define smdHitsSpinName "smdHitsSpin"
#define smdHitsBxName "smdHitsBx"
#define smdStripsSpinName "smdStripsSpin"
#define smdStripsBxName "smdStripsBx"

#endif
