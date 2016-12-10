#include "fx.h"
#include <Arduino.h>


//---------------------------------------------------------------------------
#define FX_MAX_PITCH				36
#define FX_MAX_VOLUME				4


//---------------------------------------------------------------------------
PROGMEM const uint8_t halfPeriods[FX_MAX_PITCH] = {
	246,232,219,207,195,184,174,164,155,146,138,130,123,116,110,104,
	98,92,87,82,78,73,69,65,62,58,55,52,49,46,44,41,39,37,35,33
};

PROGMEM const uint16_t  squareWaveInstrument[]     = {0x0101, 0x03F7};
PROGMEM const uint16_t  noiseInstrument[]          = {0x0101, 0x03FF};
PROGMEM const uint16_t* const defaultInstruments[] = {squareWaveInstrument, noiseInstrument};


//---------------------------------------------------------------------------
boolean    notePlaying;
int8_t     noteVolume;
uint8_t    notePitch;
uint8_t    noteDuration;

uint16_t*  instrumentData;
uint8_t    instrumentLength; //number of steps in the instrument
uint8_t    instrumentLooping; //how many steps to loop on when the last step of the instrument is reached
uint16_t   instrumentCursor; //which step is being played
uint8_t    instrumentNextChange; //how many frames before the next step

uint8_t    arpeggioStepDuration;
int8_t     arpeggioStepSize;
int8_t     volumeSlideStepDuration;
int8_t     volumeSlideStepSize;
uint8_t    tremoloStepDuration;
int8_t     tremoloStepSize;

boolean    _chanState; //if the waveform is currently high or low
uint8_t    _chanOutputVolume; //amplitude of the outputted waveform
uint8_t    _chanOutput; //current value of the outputted waveform
uint8_t    _chanCount; //counts until the next change of the waveform
uint8_t    _chanHalfPeriod; //duration of half the period of the waveform
boolean    _chanNoise; //if a random value should be added to the waveform to generate noise

int8_t     commandsCounter;
uint8_t    _rand;
uint8_t    prescaler;

int8_t     stepVolume;
uint8_t    stepPitch;

int8_t     outputVolume;
uint8_t    outputPitch;

// int8_t     patternPitch;


//---------------------------------------------------------------------------
void FxInit()
{
	_rand = 1;
	prescaler = max(FX_FPS / 20, 1);

	pinMode( 5, OUTPUT);
	pinMode(13, OUTPUT);

	 // lazy version to get the right register settings for PWM (hem)
	analogWrite(5, 1);

	// set timer 2 prescaler to 1 -> 30kHz PWM on pin 5
	TCCR3B = (TCCR3B & B11111000) | 1;

	// initialize timer1
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
	noteVolume = constrain(__LPM(d + 6), 0, 10);

	// set waveform
	instrumentData    = (uint16_t*)pgm_read_word(&(defaultInstruments[__LPM(d + 0)]));
	instrumentLength  = pgm_read_word(&(instrumentData[0])) & 0x00FF * prescaler;
	instrumentLooping = min((pgm_read_word(&(instrumentData[0])) >> 8), instrumentLength) * prescaler;

	// set volume slide
	volumeSlideStepDuration =  __LPM(d + 5) * prescaler;
	volumeSlideStepSize     = -__LPM(d + 4);

	// set pitch slide
	arpeggioStepDuration = __LPM(d + 3) * prescaler;
	arpeggioStepSize     = __LPM(d + 2) - 58;

	// set note
	notePitch    = __LPM(d + 1);
	noteDuration = __LPM(d + 7) * prescaler;

	// reinit vars
	instrumentCursor     = 0;
	instrumentNextChange = 0;

	// play note
	notePlaying     = true;
	_chanState      = true;
	commandsCounter = 0;
}
//---------------------------------------------------------------------------
void FxStop()
{
	notePlaying = false;

	// counters
	noteDuration     = 0;
	instrumentCursor = 0;
	commandsCounter  = 0;

	// output
	_chanOutput       = 0;
	_chanOutputVolume = 0;
	_chanState        = false;

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

	if(instrumentNextChange == 0)
	{
		//read the step data from the progmem and decode it
		uint16_t thisStep = pgm_read_word(&(instrumentData[instrumentCursor + 1]));

		stepVolume = thisStep & 0x0007;
		thisStep >>= 3;

		uint8_t stepNoise = thisStep & 0x0001;
		thisStep >>= 1;

		uint8_t stepDuration = thisStep & 0x003F;
		thisStep >>= 6;

		stepPitch = thisStep;

		//apply the step settings
		instrumentNextChange = stepDuration * prescaler;

		_chanNoise = stepNoise;

		instrumentCursor++;

		if(instrumentCursor >= instrumentLength)
		{
			if(instrumentLooping)
			{
				instrumentCursor = instrumentLength - instrumentLooping;
			}
			else
			{
				FxStop();
			}
		}
	}
	instrumentNextChange--;


	commandsCounter++;

	// UPDATE VALUES

	// pitch
	outputPitch = notePitch + stepPitch; // + patternPitch
	if(arpeggioStepDuration)
	{
	  outputPitch += commandsCounter / arpeggioStepDuration * arpeggioStepSize;
	}
	outputPitch = (outputPitch + FX_MAX_PITCH) % FX_MAX_PITCH; // wrap


	// volume
	outputVolume = noteVolume;
	if(volumeSlideStepDuration)
	{
		outputVolume += commandsCounter / volumeSlideStepDuration * volumeSlideStepSize;
	}

	// tremolo
	if(tremoloStepDuration)
	{
		outputVolume += ((commandsCounter / tremoloStepDuration) % 2) * tremoloStepSize;
	}

	outputVolume = constrain(outputVolume, 0, 9);

	if(notePitch == 63)
	{
		outputVolume = 0;
	}

	noInterrupts();
	_chanHalfPeriod = pgm_read_byte(halfPeriods + outputPitch);
	_chanOutput = _chanOutputVolume = outputVolume * FX_MAX_VOLUME * stepVolume;
	interrupts();
}
//---------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	if(_chanOutputVolume)
	{
		_chanCount++;

		if(_chanCount >= _chanHalfPeriod)
		{
			_chanState = !_chanState;
			_chanCount = 0;

			if(_chanNoise)
			{
				_rand = 67 * _rand + 71;
				_chanOutput = _rand % _chanOutputVolume;
			}

			FxOutput();
		}
	}
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
