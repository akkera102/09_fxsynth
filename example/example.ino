#include <Arduboy2.h>
#include "fx.h"

Arduboy2 ab;

PROGMEM const uint8_t soundfx[4][8] = {
	// PAQMAN
//	{0, 30, 59, 1, 7, 0, 2, 5},  // 0  = backgroundsound 1
//	{0, 35, 57, 1, 7, 0, 2, 5},  // 1  = backgroundsound 2
//	{0,  6, 62, 1, 0, 0, 2, 5},  // 2  = frightened backgroundsound
//	{1, 11, 66, 1, 0, 0, 7, 3},  // 3  = eat dots 1

	// INVADERS
	{1, 57, 57, 1, 1, 1, 5,  6}, // 0  = sound shoot
	{0,  0, 68, 1, 0, 0, 7,  4}, // 1  = sound invader hit
	{1, 15, 57, 1, 1, 2, 7, 15}, // 2  = sound player hit
	{0, 10, 60, 1, 0, 0, 7,  6}, // 3  = sound saucer
};

//---------------------------------------------------------------------------
void setup()
{
//	ab.begin();

	ab.boot();
	ab.blank();
	ab.flashlight();
	ab.audio.begin();

	FxInit();
}
//---------------------------------------------------------------------------
void loop()
{
	if(!(ab.nextFrame()))
	{
		return;
	}

	if(ab.justPressed(RIGHT_BUTTON))
	{
		FxPlay(soundfx[0]);
	}

	if(ab.justPressed(LEFT_BUTTON))
	{
		FxPlay(soundfx[1]);
	}

	if(ab.justPressed(DOWN_BUTTON))
	{
		FxPlay(soundfx[2]);
	}

	if(ab.justPressed(UP_BUTTON))
	{
		FxPlay(soundfx[3]);
	}

	ab.clear();

	ab.setCursor(0, 0);
	ab.print("FXSYNTH TEST!");
	ab.display();
	ab.pollButtons();

	FxUpdate();
}
