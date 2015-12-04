///////////////////////////////////////////////////////////////////////////////
// PIN MAPPINGS
#define P_AD0  2
#define P_AD1  3
#define P_AD2  4
#define P_AD3  5

#define P_DB0  6
#define P_DB1  7
#define P_DB2  8
#define P_DB3  9
#define P_DB4  10
#define P_DB5  11
#define P_DB6  12
#define P_DB7  13

#define P_CS0    15
#define P_CS1    14
#define P_RW     16

#define P_LED1    18  
#define P_LED2    17
#define P_BUTTON    19


enum {
  MAX_CHANNEL = 1,
  MAX_VOICES_PER_CHANNEL = 4,
  MAX_VOICE = (MAX_CHANNEL * MAX_VOICES_PER_CHANNEL)    
};

///////////////////////////////////////////////////////////////////////////////
// CONTROLLER NUMBERS
enum {  
  CC_MOD       = 1,
  CC_PORTATIME   = 5,
  
  CC_MIDIVEL    = 14,     
  CC_UNISON     = 15,    
  CC_TRANSPOSE  = 16,
  CC_FINETUNE   = 17,
  CC_PBRANGE    = 18,
  CC_HPF        = 20,
  CC_DIST       =21,    

  CC_DETUNEMODE  = 22,
  CC_DETUNELEVEL = 23,

  CC_MOD_2_VOL            = 24,
  CC_MOD_2_DIST           = 25,
  CC_MOD_2_HPF            = 26,
  CC_MOD_2_DETUNE         = 27,
  CC_MOD_2_LFO_RATE        = 28,
  CC_MOD_2_LFO_DEPTH       = 29,
  CC_MOD_2_ARP_RATE       = 30,
  

  CC_AENVMODE = 71,
  CC_AENVATTACK  = 73,
  CC_AENVRELEASE = 72,

  CC_MENVMODE   = 103,
  CC_MENVATTACK  = 104,
  CC_MENVRELEASE = 105,    
  CC_ENV_2_PITCH        = 85,
  CC_ENV_2_DISTORTION   = 86,
  CC_ENV_2_HPF          = 87,
  CC_ENV_2_DETUNE       = 88,
  CC_ENV_2_LFO_RATE     = 89,
  CC_ENV_2_LFO_DEPTH    = 90,
  CC_ENV_2_ARP_RATE     = 91,
  CC_ENV_2_MATRIX       = 92,

  CC_LFOMODE    = 106,
  CC_LFOWAVE    = 107,
  CC_LFORATE    = 108,
  CC_LFO_2_MATRIX    = 109,
  CC_LFO_2_PITCH          = 110,
  CC_LFO_2_VOL          = 111,
  CC_LFO_2_DIST          = 112,
  CC_LFO_2_HPF           = 113,
  CC_LFO_2_DETUNE        = 114,
  CC_LFO_2_ARP_RATE      = 115,
  
  
  CC_ARPENABLE    = 116,
  CC_ARPRATE    = 117, 
  CC_ARPCOUNT    = 118, 
  CC_ARP2ENV    = 119, 

  CC_POKEYCFG   = 93,
  CC_POKEYDUAL  = 94,
  CC_POKEYRANGE   = 95,
//  CC_POKEYPOLY9   = 95,

  CC_RESET_CTRL = 121,
  CC_OMNI_ON = 125,
  CC_OMNI_OFF = 124,

  CC_MONO = 126,
  CC_POLY = 127,
  CC_ALL_SOUND_OFF = 120,
  CC_ALL_NOTES_OFF = 123,
  
  CC_FROM_NOTE = 75,
  CC_TO_NOTE = 76
};


///////////////////////////////////////////////////////////////////////////////
// ENVELOPE CONFIGURATION
typedef struct {
  enum {
    ATT_REL,              // attack, sustain until note off, release 
    ATT_DEC,              // attack, release 
    ATT_RPT_REL,          // repeat attack until note off, release 
    ATT_DEC_RPT,          // repeat attack-decay until note off
    LOOP,                 // repeat attack-release forever
    MAX_MODE
  };
  byte    mode;           // mode as above
  unsigned int attackSlope;     // } Attack and release slopes are defined by the
  unsigned int releaseSlope;    // } increment per 16ms, where 65535 is full volume
} ENVELOPE;

///////////////////////////////////////////////////////////////////////////////
// ENVELOPE STATE
typedef struct {
  enum {
    NONE,                  // envelope is idle at low level
    ATTACK,                // attack slope
    SUSTAIN,               // sustained at full level
    RELEASE                // release slope
  };
  byte ePhase;          // phase as above
  float fValue;         // envelope value (0.0 - 1.0)  
} ENVELOPE_STATE;

///////////////////////////////////////////////////////////////////////////////
// TONE CONFIGURATION
typedef struct {

  enum {
    POKEY_8,  
    POKEY_8_HPF,  
    POKEY_16,
    POKEY_MAX      
  };
  
  enum {
     TO_VOL           = 0x01,           
//     TO_PITCH         = 0x02,
     TO_DIST          = 0x04,
     TO_HPF           = 0x08,
     TO_DETUNE        = 0x10,
     TO_LFO_RATE      = 0x20,
     TO_LFO_DEPTH     = 0x40,
     TO_ARP_RATE      = 0x80
  };

  enum {
    USE_VELOCITY = 0x01,
    UNISON       = 0x02,      
    ARPEGGIATE   = 0x04,      
    ARP2ENV      = 0x08,
    POKEY_DUAL   = 0x10,
    POKEY_HIHZ   = 0x20,
//    POKEY_POLY9  = 0x40
  };

  enum {
    WAVE_TRI,          // triangle wave
    WAVE_RAMP,         // ramp
    WAVE_REVRAMP,      // reverse ramp
    WAVE_SQ,           // square
    WAVE_RANDOM,         // random levels at step rate
    WAVE_MAX
  };

  enum {
    DETUNE_NONE,
    DETUNE_SPREAD,
    DETUNE_INTERVALUP,
    DETUNE_INTERVALDOWN,
    DETUNE_STACKUP,
    DETUNE_STACKDOWN,
    DETUNE_MAX
  };

  enum {
    LFO_HOLD,        // lfo not running
    LFO_FREE,        // lfo is free running
    LFO_GATE,        // lfo only runs when notes are held
    LFO_TRIG_FREE,   // lfo restarts on trig and runs freely
    LFO_TRIG_GATE,   // lfo restarts on trig and stops on untrig
    LFO_ONE_SHOT,    // lfo restarts on trig and runs one wave
    LFO_UNGATE,      // lfo only runs when no notes are held
    LFO_MAX
  };
  
  byte            ePokeyMode;       // pokey mode
  byte            flags;            // bit flags
  char            transpose;        // semitone offset -63 to +63 (semitones)
  float           fFineTune;        // fine tune offset (1/10 semitones)  
  char            pitchBendRange;   // pitch bend range (semitones)
  char            portaTime;        // portamento time (16ms per increment)
  char            detuneLevel;      // detune level -63 to +63 (units depend on mode)
  char            eDetuneMode;      // detune mode
  char            hpf;              // 0-127 high pass filter level
  char            dist;             // 0-127 distortion level
  char            eLFOMode;         // LFO run mode
  float           fLFOStep;         // LFO run step (increment per 8ms of the 0.0-1.0 full range)
  char            eLFOWave;         // LFO wave form
  char            arpPeriod;        // time between consecutive arp notes 
  char            arpCount;         // max notes to include in arpeggio
  
  ENVELOPE        ampEnv;           // amplitude envelope configuration
  ENVELOPE        modEnv;           // modulation envelope configuration
  
  char            modEnv2Pitch;     
  char            modEnvDepth;      
  byte            modEnvDest;
  byte            modEnvDestNeg;    
  
  char            lfo2Vol;
  char            lfo2Pitch;
  char            lfoDepth;
  byte            lfoDest;
  byte            lfoDestNeg;
  
  byte            modWheelDest;
  byte            modWheelDestNeg;
} TONE_CONFIG;

extern TONE_CONFIG Patch[MAX_CHANNEL];

extern void failTest(int t);
extern void indicateTest(int test);

#define TEST_CONDITION(c, t) { if(!(c)) failTest(t); }



