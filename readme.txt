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
	// INVADERS
	{1, 57, 57, 1, 1, 1, 5,  6},
	{0,  0, 68, 1, 0, 0, 7,  4}, // <---
	{1, 15, 57, 1, 1, 2, 7, 15},
	{0, 10, 60, 1, 0, 0, 7,  6},
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
		FxPlay(soundfx[1]); // <---
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


