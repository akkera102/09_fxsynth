FX Synth for Arduboy


## Description
FX Synth is Gamebuino sound effect maker.
I porting Arduboy. and I maked simple Sound function.
You can use your game. but It can use ONLY ARDUBOY LIBRARY2.

## How to Use
effect maker is simple. parameter is only 8.
You make array variable. see. example/example.png and example.ino.

#include <Arduboy2.h>
#include "fx.h"

Arduboy2 ab;

PROGMEM const uint8_t soundfx[4][8] = {
	// PAQMAN
	{0, 30, 59, 1, 7, 0, 2, 5},  // 0  = backgroundsound 1
	{0, 35, 57, 1, 7, 0, 2, 5},  // 1  = backgroundsound 2
	{0,  6, 62, 1, 0, 0, 2, 5},  // 2  = frightened backgroundsound
	{1, 11, 66, 1, 0, 0, 7, 3},  // 3  = eat dots 1
};

void setup()
{
	ab.begin();

	FxInit(); // <---
}

void loop()
{
	if(!(ab.nextFrame()))
	{
		return;
	}

	if(ab.justPressed(LEFT_BUTTON))
	{
		FxPlay(soundfx[0]);
	}

	ab.clear();

	ab.setCursor(0, 0);
	ab.print("FXSYNTH TEST!");
	ab.display();
	ab.pollButtons();

	FxUpdate(); // <---
}

Don't forget FxInit(), FxUpdate().


## Editor
UP + DOWN button is SAVE.


## Original Posts
http://gamebuino.com/forum/viewtopic.php?f=17&t=1018


## Development
compiler : windows Arduino IDE 1.6.12


## Licence
LGPL


## History
v1.10 2016/12/16    remake editor. add save function
v1.00 2016/12/10    first version


