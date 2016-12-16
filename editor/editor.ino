#include <Arduboy2.h>
#include "fx.h"

Arduboy2 ab;

//---------------------------------------------------------------------------
#define EDIT_MAX_MEM				16
#define EDIT_MAX_PARAM				8
#define EDIT_MAX_SAMPLE				8

//---------------------------------------------------------------------------
PROGMEM const static unsigned char imgSquare[] = {
	0x40, 0x40, 0x7e, 0x2, 0x7e, 0x40, 0x40, 0x00,
};

PROGMEM const static unsigned char imgNoise[] = {
	0x20, 0x18, 0x60, 0x10, 0x20, 0x1c, 0x60, 0x00,
};

const static int8_t paramLo[8] = {
	0, 0, 0, 0, 0, 0, 1, 0,
};
const static int8_t paramHi[8] = {
	1, 58, 116, 19, 7, 19, 7, 57,
};

const static int8_t sample[EDIT_MAX_SAMPLE][EDIT_MAX_PARAM] = {
	// PAQMAN
	{0, 30, 59, 1, 7, 0, 2, 5},  // backgroundsound 1
	{0, 35, 57, 1, 7, 0, 2, 5},  // backgroundsound 2
	{0,  6, 62, 1, 0, 0, 2, 5},  // frightened backgroundsound
	{1, 11, 66, 1, 0, 0, 7, 3},  // eat dots 1

	// INVADERS
	{1, 57, 57, 1, 1, 1, 5,  6}, // sound shoot
	{0,  0, 68, 1, 0, 0, 7,  4}, // sound invader hit
	{1, 15, 57, 1, 1, 2, 7, 15}, // sound player hit
	{0, 10, 60, 1, 0, 0, 7,  6}, // sound saucer
};

static int8_t   mem[EDIT_MAX_MEM][EDIT_MAX_PARAM];
static int8_t   index;
static int8_t   cursor;
static int8_t   repCnt;
static int8_t   popCnt;
static uint8_t* eepAdr;

//---------------------------------------------------------------------------
void setup()
{
	ab.boot();
	ab.blank();
	ab.flashlight();
	ab.audio.begin();

	FxInit();
	EditInit();
}
//---------------------------------------------------------------------------
void loop()
{
	if(!(ab.nextFrame()))
	{
		return;
	}

	ab.clear();

	EditExec();
	EditDraw();

	FxUpdate();

	ab.pollButtons();
	ab.display();
}
//---------------------------------------------------------------------------
void EditInit()
{
	index  = 0;
	cursor = 0;
	repCnt = 0;
	popCnt = 0;

	if(EditIsSave())
	{
		EditLoad();
		return;
	}


	int8_t i, j;

	for(i=0; i<EDIT_MAX_SAMPLE; i++)
	{
		for(j=0; j<EDIT_MAX_PARAM; j++)
		{
			mem[i][j] = sample[i][j];
		}
	}

	for(; i<EDIT_MAX_MEM; i++)
	{
		for(j=0; j<EDIT_MAX_PARAM; j++)
		{
			mem[i][j] = paramLo[j];
		}
	}

	EditSave();
}
//---------------------------------------------------------------------------
void EditExec()
{
	if(ab.justPressed(A_BUTTON))
	{
		FxPlay2(mem[index]);
	}


	if(ab.justPressed(B_BUTTON))
	{
		mem[index][0] = ++mem[index][0] & 0x1;
	}

	if(ab.justPressed(UP_BUTTON))
	{
		cursor = --cursor & 7;
	}

	if(ab.justPressed(DOWN_BUTTON))
	{
		cursor = ++cursor & 7;
	}

	if(ab.pressed(UP_BUTTON) && ab.pressed(DOWN_BUTTON) && popCnt == 0)
	{
		EditSave();
		popCnt = 100;
	}

	// param or MEM
	if(!ab.pressed(RIGHT_BUTTON) && !ab.pressed(LEFT_BUTTON))
	{
		repCnt = 0;
		return;
	}

	if(cursor != 7)
	{
		int8_t c = cursor + 1;

		if(EditRepeatButton(RIGHT_BUTTON))
		{
			if(++mem[index][c] > paramHi[c])
			{
				mem[index][c] = paramLo[c];
			}
		}

		if(EditRepeatButton(LEFT_BUTTON))
		{
			if(--mem[index][c] < paramLo[c])
			{
				mem[index][c] = paramHi[c];
			}
		}
	}
	else
	{
		if(EditRepeatButton(RIGHT_BUTTON))
		{
			if(++index >= EDIT_MAX_MEM)
			{
				index = 0;
			}
		}

		if(EditRepeatButton(LEFT_BUTTON))
		{
			if(--index < 0)
			{
				index = EDIT_MAX_MEM - 1;
			}
		}
	}
}
//---------------------------------------------------------------------------
void EditDraw()
{
	// msg
	EditDrawMsg(0, 1*8, F("PMD"));
	EditDrawMsg(0, 2*8, F("PMT"));
	EditDrawMsg(0, 3*8, F("VMD"));
	EditDrawMsg(0, 4*8, F("VMT"));
	EditDrawMsg(0, 5*8, F("VOL"));
	EditDrawMsg(0, 6*8, F("LEN"));
	EditDrawMsg(0, 7*8, F("MEM"));

	// img
	if(mem[0][index] == 0)
	{
		ab.drawBitmap(0, 0, imgSquare, 8, 8);
	}
	else
	{
		ab.drawBitmap(0, 0, imgNoise, 8, 8);
	}

	// num
	for(int8_t i=1; i<EDIT_MAX_PARAM; i++)
	{
		EditDrawNum(128-6*3, (i-1)*8, mem[index][i]);
	}
	EditDrawNum(6*2, 0, mem[index][0]);

	// line
	for(int8_t i=1; i<EDIT_MAX_PARAM; i++)
	{
		// left
		ab.drawLine(6*3 + 1, (i-1)*8, 6*3 + 1, 6 + ((i-1)*8));

		// mid
		ab.drawLine(6*3 + 1, 3 + ((i-1)*8), 128 - 6*3 - 3, 3 + ((i-1)*8));

		//right
		ab.drawLine(128 - 6*3 - 3, (i-1)*8, 128 - 6*3 - 3, 6 + ((i-1)*8));

	}

	// param
	for(int8_t i=1; i<EDIT_MAX_PARAM; i++)
	{
		double c, d;

		// VOL
		if(i != 6)
		{
			c = (double)(mem[index][i]);
			d = paramHi[i];
		}
		else
		{
			c = (double)(mem[index][i] - 1);
			d = paramHi[i] - 1;
		}

		ab.drawRect((c / d)*88 + 6*3, (i-1)*8, 3, 7);
	}

	// select
	if(ab.everyXFrames(5))
	{
		ab.fillRect(0, 8*cursor, 6*3, 7);
	}

	// MEM index
	EditDrawNum(6*4 , 7*8, index);

	// save message
	if(popCnt != 0)
	{
		EditDrawMsg(128-6*5, 7*8, F("SAVE!"));

		if(!ab.everyXFrames(10))
		{
			popCnt--;
		}
	}
}
//---------------------------------------------------------------------------
void EditDrawMsg(uint8_t x, uint8_t y, const __FlashStringHelper* m)
{
	ab.setCursor(x, y);
	ab.print(m);
}
//---------------------------------------------------------------------------
void EditDrawNum(uint8_t x, uint8_t y, int8_t n)
{
	ab.setCursor(x, y);
	ab.print(n);
}
//---------------------------------------------------------------------------
bool EditIsSave()
{
	EditSeek(0);

	if(EditRead() != 'F') return false;
	if(EditRead() != 'X') return false;
	if(EditRead() != 'S') return false;
	if(EditRead() != 'Y') return false;

	return true;
}
//---------------------------------------------------------------------------
void EditSeek(uint16_t adr)
{
	eepAdr = (uint8_t*)EEPROM_STORAGE_SPACE_START + adr;
}
//---------------------------------------------------------------------------
uint8_t EditRead()
{
	eeprom_busy_wait();
	return eeprom_read_byte(eepAdr++);
}
//---------------------------------------------------------------------------
void EditWrite(uint8_t dat)
{
	eeprom_busy_wait();
	eeprom_write_byte(eepAdr++, dat);
}
//---------------------------------------------------------------------------
void EditLoad()
{
	EditSeek(4);

	for(int8_t i=0; i<EDIT_MAX_MEM; i++)
	{
		for(int8_t j=0; j<EDIT_MAX_PARAM; j++)
		{
			mem[i][j] = EditRead();
		}
	}
}
//---------------------------------------------------------------------------
void EditSave()
{
	EditSeek(0);

	EditWrite('F');
	EditWrite('X');
	EditWrite('S');
	EditWrite('Y');

	for(int8_t i=0; i<EDIT_MAX_MEM; i++)
	{
		for(int8_t j=0; j<EDIT_MAX_PARAM; j++)
		{
			EditWrite(mem[i][j]);
		}
	}
}
//---------------------------------------------------------------------------
bool EditRepeatButton(uint8_t button)
{
	if(!ab.pressed(button))
	{
		return false;
	}

	if(ab.pressed(button) && repCnt == 0)
	{
		repCnt++;
		return true;
	}

	repCnt = ++repCnt % 6;
	return false;
}
