#ifndef FXSYNTH_H
#define FXSYNTH_H

#include <avr/pgmspace.h>

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
void FxInit();
void FxPlay(uint8_t* d);
void FxPlay2(uint8_t* d);
void FxStop();
void FxUpdate();
void FxOutput();

void FxSetFps(uint8_t fps);


#endif
