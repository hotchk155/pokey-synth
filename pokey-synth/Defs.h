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
  CC_TRANSPOSE  = 16,
  CC_FINETUNE   = 17,
  CC_PBRANGE    = 18,
  CC_HPF        = 20,
  CC_DIST       =21,    

  CC_UNISONTYPE  = 22,
  CC_DETUNELEVEL = 23,

  CC_LFODEPTH_MODSRC  = 25,
  CC_DETUNE_MODSRC  = 26,
  CC_HPF_MODSRC  = 27,
  CC_DIST_MODSRC  = 28,
  CC_LFORATE_MODSRC  = 29,
  CC_ARPATE_MODSRC  = 30,

  CC_AENVMODE = 71,
  CC_AENVATTACK  = 73,
  CC_AENVRELEASE = 72,

  CC_MENVMODE   = 103,
  CC_MENVATTACK  = 104,
  CC_MENVRELEASE = 105,    
  
  CC_ENV_2_PITCH        = 85,
  
  CC_LFOMODE    = 106,
  CC_LFOWAVE    = 107,
  CC_LFORATE    = 108,
  CC_LFODEPTH    = 109,
  CC_LFO_2_PITCH          = 110,
  CC_LFO_2_VOL          = 111,
  
  CC_ARPENABLE    = 116,
  CC_ARPRATE    = 117, 
  CC_ARPCOUNT    = 118, 
  CC_ARP2ENV    = 119, 

  CC_POKEYCFG   = 93,
  CC_POKEYDUAL  = 94,
  CC_POKEYRANGE   = 95,

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
    USE_VELOCITY = 0x01,
    ARPEGGIATE   = 0x02,      
    ARP2ENV      = 0x04,
    POKEY_DUAL   = 0x08,
    POKEY_HIHZ   = 0x10,
    MONO         = 0x20
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
    UNISON_OFF,
    UNISON_SPREAD,
    UNISON_INTERVALUP,
    UNISON_INTERVALDOWN,
    UNISON_STACKUP,
    UNISON_STACKDOWN,
    UNISON_MAX
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
  
  enum {
     MODSRC_NONE,    // no modulation / use level controller to set value
     MODSRC_ENV,     // mod envelope
     MODSRC_LFO,     // LFO
     MODSRC_WHEEL,    // Mod wheel
     MODSRC_MAX
  };
         
  byte            ePokeyMode;       // pokey mode  
  byte            flags;            // bit flags
  char            transpose;        // semitone offset -63 to +63 (semitones)
  float           fFineTune;        // fine tune offset (1/10 semitones)  
  char            pitchBendRange;   // pitch bend range (semitones)
  char            portaTime;        // portamento time (16ms per increment)
  char            eUnisonMode;      // unison mode

  // ENVELOPES
  
  ENVELOPE        ampEnv;           // amplitude envelope configuration
  ENVELOPE        modEnv;           // modulation envelope configuration

  // PITCH MODULATION
  
  char            lfoToPitch;         // vibrato
  char            modEnvToPitch;      // pitch modulation by envelope
  
  // AMPLITUDE MODULATION
  
  char            lfoToAmp;         // tremelo

  // DETUNE
  
  char            detuneLevel;      // detune level -63 to +63 (units depend on mode)
  byte            detuneModSrc;

  // HIGHPASS
  
  char            hpf;              // 0-127 high pass filter level
  byte            hpfModSrc;

  // DISTORTION
  
  char            dist;             // 0-127 distortion level
  byte            distModSrc;

  // LFO 
  
  char            eLFOMode;         // LFO run mode
  char            eLFOWave;         // LFO wave form
  
  char            lfoRate;          // LFO rate
  char            lfoDepth;         // LFO rate
  byte            lfoRateModSrc;    // mod source for LFO rate
  byte            lfoDepthModSrc;   // mod source for LFO depth
    
  // ARPEGGIATION
  
  char            arpRate;          // arp rate from 0 (slowest) to 127 (fastest)
  byte            arpRateModSrc;    // mod source for arp rate
  
  char            arpCount;         // max notes to include in arpeggio
    
} TONE_CONFIG;

extern TONE_CONFIG Patch[MAX_CHANNEL];

extern void failTest(int t);
extern void indicateTest(int test);

#define TEST_CONDITION(c, t) { if(!(c)) failTest(t); }



