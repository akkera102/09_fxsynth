#include "fx.h"
#include <Arduino.h>


//---------------------------------------------------------------------------
#define FX_FPS						60
#define FX_MAX_PITCH				36
#define FX_MAX_VOLUME				4
#define FX_INSTRUMENT_TYPE_SQUARE	0
#define FX_INSTRUMENT_TYPE_NOISE	1

PROGMEM const static uint8_t halfPeriods[FX_MAX_PITCH] = {
	246,232,219,207,195,184,174,164,155,146,138,130,123,116,110,104,
	98,92,87,82,78,73,69,65,62,58,55,52,49,46,44,41,39,37,35,33
};

//---------------------------------------------------------------------------
static boolean notePlaying;
static int8_t  noteVolume;
static uint8_t notePitch;
static uint8_t noteDuration;

static uint8_t instrumentType;
static uint8_t instrumentLength;		// number of steps in the instrument

static uint8_t arpeggioStepDuration;
static int8_t  arpeggioStepSize;
static int8_t  volumeStepDuration;
static int8_t  volumeStepSize;

static boolean _chanState;				// if the waveform is currently high or low
static uint8_t _chanOutputVolume;		// amplitude of the outputted waveform
static uint8_t _chanOutput;				// current value of the outputted waveform
static uint8_t _chanCount;				// counts until the next change of the waveform
static uint8_t _chanHalfPeriod;			// duration of half the period of the waveform

static int8_t  commandsCounter;
static uint8_t  prescaler;
static uint8_t _rand;

//---------------------------------------------------------------------------
void FxInit()
{
	_rand = 1;
	FxSetFps(FX_FPS);

	pinMode(5, OUTPUT);
	pinMode(13, OUTPUT);

	 // lazy version to get the right register settings for PWM (hem)
	analogWrite(5, 1);

	// set timer 3 prescaler to 1 -> 30kHz PWM on pin 5
	TCCR3B = (TCCR3B & B11111000) | 1;

	// initialize timer 1
	noInterrupts();
	TCCR1A  = 0;
	TCCR1B  = 0;
	TCNT1   = 0;

	OCR1A   = 280;				// compare match register
	TCCR1B |= (1 << WGM12);		// CTC mode
	TCCR1B |= (1 << CS10);		// 1 prescaler
	TIMSK1 |= (1 << OCIE1A);	// enable timer compare interrupt
	interrupts();
}
//---------------------------------------------------------------------------
void FxPlay(uint8_t* d)
{
	uint8_t t[8];

	for(uint8_t i=0; i<8; i++)
	{
		t[i] = pgm_read_byte(d + i);
	}

	FxPlay2(t);
}
//---------------------------------------------------------------------------
void FxPlay2(uint8_t* d)
{
	noteVolume           = d[6];
	instrumentType       = d[0];
	instrumentLength     = prescaler;
	volumeStepDuration   = d[5] * prescaler;
	volumeStepSize       = d[4] * -1;
	arpeggioStepDuration = d[3] * prescaler;
	arpeggioStepSize     = d[2] - 58;
	noteDuration         = d[7] * prescaler;
	notePitch            = d[1];

	commandsCounter = 0;
	notePlaying     = true;
	_chanState      = true;
}
//---------------------------------------------------------------------------
void FxStop()
{
	notePlaying       = false;
	_chanState        = false;
	_chanOutput       = 0;
	_chanOutputVolume = 0;

	FxOutput();
}
//---------------------------------------------------------------------------
void FxUpdate()
{
	if(notePlaying == false)
	{
		return;
	}

	if(noteDuration == 0)
	{
		FxStop();
		return;
	}
	noteDuration--;


	commandsCounter++;

	// UPDATE VALUES

	// pitch
	uint8_t outputPitch = notePitch;
	if(arpeggioStepDuration)
	{
		outputPitch += commandsCounter / arpeggioStepDuration * arpeggioStepSize;
	}
	outputPitch = (outputPitch + FX_MAX_PITCH) % FX_MAX_PITCH;	// wrap

	// volume
	int8_t outputVolume = noteVolume;
	if(volumeStepDuration)
	{
		outputVolume += commandsCounter / volumeStepDuration * volumeStepSize;
	}
	outputVolume = constrain(outputVolume, 0, 9);

	noInterrupts();
	_chanHalfPeriod = pgm_read_byte(halfPeriods + outputPitch);
	_chanOutput = _chanOutputVolume = outputVolume * FX_MAX_VOLUME * 7;		// 7 = stepVolume
	interrupts();
}
//---------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	if(_chanOutputVolume == 0)
	{
		return;
	}

	if(++_chanCount < _chanHalfPeriod)
	{
		return;
	}
	_chanCount = 0;

	if(instrumentType == FX_INSTRUMENT_TYPE_NOISE)
	{
		_rand = 67 * _rand + 71;
		_chanOutput = _rand % _chanOutputVolume;
	}

	_chanState = !_chanState;

	FxOutput();
}
//---------------------------------------------------------------------------
void FxOutput()
{
	uint8_t output = 0;

	if(_chanState)
	{
		output += _chanOutput;
	}

	OCR3A = output;
}
//---------------------------------------------------------------------------
void FxSetFps(uint8_t fps)
{
	prescaler = max(fps / 20, 1);
}
