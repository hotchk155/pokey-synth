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
  byte m_envPhase;          // as above
  unsigned int m_envLevel;  // envelope level 0-65535 
  
  float m_note;             // assigned MIDI note (0-127)
  float m_vol;              // note velocity 0-1.0
  float m_detune;           // voice detune in +/- fractional midi notes
  
  CLogicalVoice();
  void assign(CPokeyChannel *pch);
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
    FLAG_ARPEGGIATE   = 0x04      // when this flag is set, all held notes are played sequentially
//    FLAG_LFO2VOL      = 0x10,      
//    FLAG_LFO2PITCH    = 0x20,      
//    FLAG_LFO2DIST     = 0x40,      
//    FLAG_LFO2HPF      = 0x80       
  };
  enum {
    LFO_TRI,          // triangle wave
    LFO_RAMP,         // ramp
    LFO_REVRAMP,      // reverse ramp
    LFO_SQ,           // square
    LFO_RND1,         // random levels at step rate
    LFO_RND2          // random levels, random rate
  };
  enum {
    RUN_HOLD,        // lfo not running
    RUN_FREE,        // lfo is free running
    RUN_TRIG,        // lfo restarts on trig and runs freely
    RUN_TRIG_GATE,   // lfo restarts on trig and stops on untrig
    RUN_GATE,        // lfo only runs when notes are held
    RUN_UNGATE       // lfo only runs when no notes are held
  };
  enum {
    ENV2NONE,
    ENV2VOL,
    ENV2PITCH,
    ENV2DIST,
    ENV2HPF,
    ENV2LFORATE,
    ENV2LFOLEVEL
  };
  enum {
    LFO2NONE,
    LFO2VOL,
    LFO2PITCH,
    LFO2DIST,
    LFO2HPF
  };

  byte            m_flags;
  byte            m_midiChannel;      // the midi channel for this logical channel;
  char            m_transpose;
  char            m_fineTune;
  char            m_bendRange;        // pitch bend range
  float           m_bend;             // pitch bend amount (MIDI note units)
  char            m_hpf;
  char            m_dist;
  
  unsigned int    m_attack;           // attack phase step (increment per ms of 0xFFFF max level)
  unsigned int    m_release;          // release phase step (increment per ms of 0xFFFF max level)
  byte            m_envDest;          // lfo destination
  
  int             m_lfoCount;         // lfo counter value 0-65535  
  int             m_lfoStep;          // counts per ms (lfo freq);
  byte            m_lfoWave;          // lfo waveform
  byte            m_lfoDest;          // lfo destination
  byte            m_lfoRun;           // lfo run mode
  
  char            m_lfoLevel;         // level of lfo output (0-127)
  float           m_lfo;              // lfo value -1.0 to +1.0 
  float           m_lfo_positive;     // lfo value 0.0 to +1.0 
      
  byte            m_portaLevel;      // portamento time for mono modes
  byte            m_portaTarget;     // target note for portamento
  float           m_portaStep;       // pitch step per ms
  float           m_portamento;      // actual note played by portamento engine

  int             m_arpPeriod;       // rate of note arpeggiation (ms period)
  byte            m_arpIndex;        // arp index in chord
  int             m_arpCounter;      // time until next arp step
  
  CLogicalChannel(); 
  void init(int voices);
  void assign(int voice, CPokeyChannel *pch);
  byte deleteNote(byte note);
  void handle(byte status, byte *params);
  void handleNoteOn(byte note, byte velocity);
  void handleNoteOff(byte note);
  void handlePitchBend(byte lo, byte hi);
  void trig(byte note, byte velocity);  
  void untrig(byte note);
  void tick();
  void detune(float detune);
};



