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

#define NO_NOTE 0xFF
#define OMNI    0xFF

#define NUM_LOGICAL_CHANNELS 4


  enum {
    CF_FULLVELOCITY = 0x01,
    CF_UNISON       = 0x02,      // when this flag is set, all available voices play in unison
    CF_ARPEGGIATE   = 0x04,      // when this flag is set, all held notes are played sequentially
    CF_OMNINOTES    = 0x08,
    CF_OMNICC       = 0x10,
    CF_PORTAMENTO   = 0x80,
    CF_ENVLOOP      = 0x100
  };

  enum {
    LFO_TRI,          // triangle wave
    LFO_RAMP,         // ramp
    LFO_REVRAMP,      // reverse ramp
    LFO_SQ,           // square
    LFO_RND1,         // random levels at step rate
    LFO_MAX
  };
  enum {
    RUN_HOLD,        // lfo not running
    RUN_FREE,        // lfo is free running
    RUN_TRIG,        // lfo restarts on trig and runs freely
    RUN_TRIG_GATE,   // lfo restarts on trig and stops on untrig
    RUN_TRIG_ONCE,   // lfo restarts on trig and runs one wave
    RUN_GATE,        // lfo only runs when notes are held
    RUN_UNGATE,       // lfo only runs when no notes are held
    RUN_MAX
  };
  enum {
    MOD2NONE = 0,
    MOD2VOL,
    MOD2PITCH,
    MOD2DIST,
    MOD2HPF,
    MOD2DETUNE,
    MOD2LFORATE,
    MOD2LFOLEVEL,
    MOD2MAX
  };
  enum {
    ENV2NONE,
    ENV2VOL,
    ENV2PITCH,
    ENV2DIST,
    ENV2HPF,
    ENV2DETUNE,
    ENV2LFORATE,
    ENV2LFOLEVEL,
    ENV2MAX
  };
  enum {
    LFO2NONE,
    LFO2VOL,
    LFO2PITCH,
    LFO2DIST,
    LFO2HPF,
    LFO2DETUNE,
    LFO2MAX
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
    CC_MOD       = 1,
    
    CC_OMNI       = 30,
    CC_TRIGMIN    = 31,
    CC_TRIGMAX    = 32,    
    CC_MIDIVEL    = 33,     
    CC_UNISON     = 34,    
    CC_TRANSPOSE  = 35,
    CC_FINETUNE   = 36,
    CC_DIVRANGE   = 71,
    
    CC_ARPMODE    = 37,
    CC_ARPRATE    = 38, 
    CC_ARPCOUNT    = 70, 
    
    CC_MODDEST    = 39,   
    
    CC_LFOMODE    = 40,
    CC_LFOWAVE    = 41,
    CC_LFORATE    = 42,
    CC_LFODEST    = 43,
    CC_LFOLEVEL    = 72,
    
    CC_DETUNEMODE  = 44,
    CC_DETUNELEVEL = 45,
    
    CC_ENVATTACK  = 46,
    CC_ENVRELEASE = 47,
    CC_ENVDEST    = 48,
        
    CC_PBRANGE    = 49,
    CC_HPF        = 50,
    CC_DIST       =51    
  };


typedef struct {
   
  char            req_channel[8];           // physical channel indexes assigned (-1 to fill)  
  // CC Values
  char            midiChannel;      // the midi channel for this logical channel;
  unsigned int    flags;            // bit flags
  char            trigMin;          // lowest MIDI note that the channel responds to 
  char            trigMax;          // highest MIDI note that the channel responds to 
  char            transpose;        // semitone offset -63 to +63 (semitones)
  float           fineTune;         // fine tune offset (semitones)  
  char            pitchBendRange;   // pitch bend range (semitones)
  char            portaTime;        // portamento time
  char            detuneLevel;      // detune level -63 to +63 (units depend on mode)
  char            eDetuneMode;      // detune mode
  char            hpf;              // 0-127 high pass filter level
  char            dist;             // 0-127 distortion level
  char            eLFOMode;         // LFO run mode
  float           fLFOStep;         // LFO run step (increment per 8ms of the 0.0-1.0 full range)
  char            eLFOWave;         // LFO wave form
  float           fLFOLevel;          // intensity of LFO modulation
  float           fAttackStep;      // Envelope attack step (increment per 8ms of the 0.0-1.0 full range)
  float           fReleaseStep;     // Envelope release step (increment per 8ms of the 0.0-1.0 full range)
  char            arpPeriod;        // time between consecutive arp notes (in 8ms increments)  
  char            arpCount;         // max notes to include in arpeggio
  char            eModWheelDest;    // mod wheel modulation dest
  char            eEnvelopeDest;    // envelope modulation dest
  char            eLFODest;         // LFO modulation destination
} CHANNEL_CONFIG;


typedef struct {
  byte pokey1Mode;
  byte pokey2Mode;
  CHANNEL_CONFIG channel_config[NUM_LOGICAL_CHANNELS];
} GLOBAL_CONFIG;

