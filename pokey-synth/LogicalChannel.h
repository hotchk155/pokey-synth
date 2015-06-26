///////////////////////////////////////////////////////////
//
// POKEYSYNTH 
// hotchk155/2015
//
///////////////////////////////////////////////////////////
class CLogicalChannel;
///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{
  CPokeyChannel *m_pch;   // physical pokey channel
public:  
  enum {
    ENV_NONE,        // The voice is not playing
    ENV_ATTACK,      // The voice is triggered, the envelope is in the attack phase
    ENV_SUSTAIN,
    ENV_RELEASE      // The voice is no longer triggered, the envelope is releasing
  };
  byte m_eEnvelopePhase;          // as above
  float m_fEnvelope;         // envelope value (0.0 - 1.0)  
  float m_note;             // assigned MIDI note (0-127)
  float m_vol;              // note velocity 0-1.0
  char  m_detuneFactor;  
  
  CLogicalVoice();
  void assign(CPokeyChannel *pch);
  void range(byte v);
  void update(CLogicalChannel *lch);
};


///////////////////////////////////////////////////////////////////////////////////////////
//
// LOGICAL CHANNEL (MIDI CHANNEL INPUT)
//
///////////////////////////////////////////////////////////////////////////////////////////

class CLogicalChannel 
{
protected:  
  typedef struct {
    byte note;
    byte velocity;
  } NOTE;
  enum {
    MAX_NOTES = 10
  };
  CLogicalVoice *m_voices;
  byte m_voiceCount;
  NOTE m_notes[MAX_NOTES];
  byte m_noteCount;
    
public:  
  enum {
    FLAG_FULLVELOCITY = 0x01,
    FLAG_UNISON       = 0x02,      // when this flag is set, all available voices play in unison
    FLAG_ARPEGGIATE   = 0x04,      // when this flag is set, all held notes are played sequentially
    FLAG_OMNINOTES    = 0x08,
    FLAG_OMNICC       = 0x10,
    FLAG_LFOSIGN      = 0x20,
    FLAG_LFOCOMPLETE  = 0x40,
    FLAG_PORTAMENTO   = 0x80,
    FLAG_ENVLOOP      = 0x100
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

  char            m_midiChannel;      // the midi channel for this logical channel;
  
  unsigned int    m_flags;           // bit flags
  char            m_updateChan;      // the logical channel being updated

  // CC Values
  char            m_trigMin;          // lowest MIDI note that the channel responds to 
  char            m_trigMax;          // highest MIDI note that the channel responds to 
  char            m_transpose;        // semitone offset -63 to +63 (semitones)
  float           m_fineTune;         // fine tune offset (semitones)  
  float           m_fPitchBend;       // pich bend offset (semitones)
  float           m_fModWheel;        // mod wheel displacement (0.0 - 1.0)
  char            m_pitchBendRange;   // pitch bend range (semitones)
  char            m_portaTime;        // portamento time
  char            m_detuneLevel;      // detune level -63 to +63 (units depend on mode)
  char            m_eDetuneMode;      // detune mode
  char            m_hpf;              // 0-127 high pass filter level
  char            m_dist;             // 0-127 distortion level
  char            m_eLFOMode;         // LFO run mode
  float           m_fLFOStep;         // LFO run step (increment per 8ms of the 0.0-1.0 full range)
  char            m_eLFOWave;         // LFO wave form
  float           m_fLFOLevel;          // intensity of LFO modulation
  float           m_fAttackStep;      // Envelope attack step (increment per 8ms of the 0.0-1.0 full range)
  float           m_fReleaseStep;     // Envelope release step (increment per 8ms of the 0.0-1.0 full range)
  char            m_arpPeriod;        // time between consecutive arp notes (in 8ms increments)  
  char            m_arpCount;         // max notes to include in arpeggio
  char            m_eModWheelDest;    // mod wheel modulation dest
  char            m_eEnvelopeDest;    // envelope modulation dest
  char            m_eLFODest;         // LFO modulation destination
  
  // Information calculated at the level of logical channel 
  float           m_fLFO;               // LFO in 0.0 to +1.0 range
  float           m_fLFOBipolar;        // LFO in -1.0 to +1.0 range
  float           m_fPortamentoNote;    // current note played by portamento engine  
  float           m_fDetuneStep;        // detune step in semitones
  
  // Internal counters
  int             m_arpCounter;      // countdown until next arp step (8ms increments)
  byte            m_arpIndex;        // arp index in chord
  float           m_fLFOCount;       // actual LFO count 0.0-1.0 value before wave applied
  char            m_portaTargetNote; // the target note of portamento engine 
  float           m_fPortaStep;      // portamento pitch change per 8ms
  
  CLogicalChannel(char ch); 
  void init(int voices);
  void assign(int voice, CPokeyChannel *pch);
  byte deleteNote(byte note);
  void handle(byte status, byte *params);
  void handleNoteOn(byte note, byte velocity);
  void handleNoteOff(byte note);
  void handleCC(char cc, char value);
  void handlePitchBend(byte lo, byte hi);
  void trig(byte note, byte velocity);  
  void untrig(byte note);
  void tick(byte counter) ;
  void ccOmni(char value);  
  void ccFlag(int flag, char value);
  byte ccMapValue(char value, int maxValue);
  void recalc_detune();
  void reset();
  void range(byte v);
  
};




