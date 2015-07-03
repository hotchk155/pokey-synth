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

///////////////////////////////////////////////////////////////////////////////
// CONTROLLER NUMBERS
enum {  
  CC_MOD       = 1,

  CC_POKEYCFG   = 20,
  
  CC_MIDIVEL    = 30,     
  CC_UNISON     = 31,    
  CC_TRANSPOSE  = 32,
  CC_FINETUNE   = 33,
  CC_DIVRANGE   = 34,
  
  CC_ARPMODE    = 35,
  CC_ARPRATE    = 36, 
  CC_ARPCOUNT    = 37, 
  CC_ARP2ENV    = 38, 
  
  CC_LFOMODE    = 39,
  CC_LFOWAVE    = 40,
  CC_LFORATE    = 41,
  CC_LFODEPTH    = 43,
  
  CC_DETUNEMODE  = 44,
  CC_DETUNELEVEL = 45,
  
  CC_AENVMODE = 46,
  CC_AENVATTACK  = 47,
  CC_AENVRELEASE = 48,
      
  CC_PBRANGE    = 50,
  CC_HPF        = 51,
  CC_DIST       =52,    
  
  CC_MENVMODE   = 53,
  CC_MENVATTACK  = 54,
  CC_MENVRELEASE = 55,    

  CC_ENV_2_PITCH        = 56,
  CC_ENV_2_DISTORTION   = 57,
  CC_ENV_2_HPF          = 58,
  CC_ENV_2_DETUNE       = 59,
  CC_ENV_2_LFO_RATE     = 60,
  CC_ENV_2_LFO_DEPTH    = 61,
  CC_ENV_2_ARP_RATE     = 62,
  
  CC_LFO_2_PITCH          = 69,
  CC_LFO_2_VOL          = 70,
  CC_LFO_2_DIST          = 71,
  CC_LFO_2_HPF           = 72,
  CC_LFO_2_DETUNE        = 73,
  CC_LFO_2_ARP_RATE      = 74,

  CC_MOD_2_VOL            = 75,
  CC_MOD_2_DIST           = 76,
  CC_MOD_2_HPF            = 77,
  CC_MOD_2_DETUNE         = 78,
  CC_MOD_2_LFO_RATE        = 79,
  CC_MOD_2_LFO_DEPTH       = 80,
  CC_MOD_2_ARP_RATE       = 81    
  
};

///////////////////////////////////////////////////////////////////////////////
// ENVELOPE CONFIGURATION
typedef struct {
  enum {
    ATT_REL,              // attack, sustain until note off, release 
    ATT_DEC,              // attack, release 
    ATT_RPT_REL,          // repeat attack until note off, release 
    ATT_DEC_RPT_REL,      // repeat attack-release until note off, release 
    LOOP,                 // repeat attack-release forever
    MAX_MODE
  };
  byte    mode;           // mode as above
  float   fAttackStep;    // 
  float   fReleaseStep;   // 
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
     TO_VOL           = 0x01,           
     TO_PITCH         = 0x02,
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
    PORTAMENTO   = 0x10
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
    DETUNE_FINE,
    DETUNE_INTERVAL,
    DETUNE_SPREAD,
    DETUNE_STACK,
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
  
  byte ePokey1Mode;
  byte ePokey2Mode;  
  byte            flags;            // bit flags
  char            transpose;        // semitone offset -63 to +63 (semitones)
  float           fFineTune;         // fine tune offset (semitones)  
  char            pitchBendRange;   // pitch bend range (semitones)
  char            portaTime;        // portamento time
  char            detuneLevel;      // detune level -63 to +63 (units depend on mode)
  char            eDetuneMode;      // detune mode
  char            hpf;              // 0-127 high pass filter level
  char            dist;             // 0-127 distortion level
  char            eLFOMode;         // LFO run mode
  float           fLFOStep;         // LFO run step (increment per 8ms of the 0.0-1.0 full range)
  char            eLFOWave;         // LFO wave form
  char            lfoDepth;
  char            arpPeriod;        // time between consecutive arp notes (in 8ms increments)  
  char            arpCount;         // max notes to include in arpeggio
  ENVELOPE        ampEnv;
  ENVELOPE        modEnv;
  char            modEnv2Pitch;
  byte            modEnvDest;
  byte            modEnvDestNeg;    
  char            lfo2Pitch;
  byte            lfoDest;
  byte            lfoDestNeg;
  byte            modWheelDest;
  byte            modWheelDestNeg;
} TONE_CONFIG;

typedef union {
  TONE_CONFIG tone;  
} CHANNEL_CONFIG;

