#ifndef FXSYNTH_H
#define FXSYNTH_H

#include <avr/pgmspace.h>

//---------------------------------------------------------------------------
#define FX_FPS						60


//---------------------------------------------------------------------------
void FxInit();
void FxPlay(uint8_t* d);
void FxStop();
void FxUpdate();
void FxOutput();


#endif
