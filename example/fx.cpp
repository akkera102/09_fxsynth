#include "fx.h"
#include <Arduino.h>


//---------------------------------------------------------------------------
#define FX_FPS						60
#define FX_MAX_PITCH				36
#define FX_MAX_VOLUME				4
#define FX_INSTRUMENT_TYPE_SQUARE	0
#define FX_INSTRUMENT_TYPE_NOISE	1

PROGMEM const uint8_t halfPeriods[FX_MAX_PITCH] = {
	246,232,219,207,195,184,174,164,155,146,138,130,123,116,110,104,
	98,92,87,82,78,73,69,65,62,58,55,52,49,46,44,41,39,37,35,33
};

//---------------------------------------------------------------------------
boolean    notePlaying;
int8_t     noteVolume;
uint8_t    notePitch;
uint8_t    noteDuration;

uint8_t    instrumentType;
uint8_t    instrumentLength;		// number of steps in the instrument

uint8_t    arpeggioStepDuration;
int8_t     arpeggioStepSize;
int8_t     volumeStepDuration;
int8_t     volumeStepSize;
uint8_t    tremoloStepDuration;
int8_t     tremoloStepSize;

boolean    _chanState;				// if the waveform is currently high or low
uint8_t    _chanOutputVolume;		// amplitude of the outputted waveform
uint8_t    _chanOutput;				// current value of the outputted waveform
uint8_t    _chanCount;				// counts until the next change of the waveform
uint8_t    _chanHalfPeriod;			// duration of half the period of the waveform

int8_t     commandsCounter;
uint8_t    prescaler;
uint8_t    _rand;

//---------------------------------------------------------------------------
void FxInit()
{
	_rand = 1;
	FxSetFps(FX_FPS);

	 // lazy version to get the right register settings for PWM (hem)
	pinMode(5, OUTPUT);
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
	// set volume
	noteVolume = pgm_read_byte(d + 6);

	// set waveform
	instrumentType    = pgm_read_byte(d + 0);
	instrumentLength  = prescaler;

	// set volume
	volumeStepDuration =  pgm_read_byte(d + 5) * prescaler;
	volumeStepSize     = -pgm_read_byte(d + 4);

	// set pitch
	arpeggioStepDuration = pgm_read_byte(d + 3) * prescaler;
	arpeggioStepSize     = pgm_read_byte(d + 2) - 58;

	// set note
	noteDuration = pgm_read_byte(d + 7) * prescaler;
	notePitch    = pgm_read_byte(d + 1);

	// play note
	notePlaying     = true;
	_chanState      = true;
	commandsCounter = 0;
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

	// tremolo
	if(tremoloStepDuration)
	{
		outputVolume += ((commandsCounter / tremoloStepDuration) % 2) * tremoloStepSize;
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
