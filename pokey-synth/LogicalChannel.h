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
    ENV_SUSTAIN,     // The voice is triggered, the envelope is in the sustain phase
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
    FLAG_FULLVELOCITY    = 0x01,
    FLAG_UNISON          = 0x02,      // when this flag is set, all available voices play in unison
  };
  enum {
    LFO_TRI,
    LFO_RAMP,
    LFO_REVRAMP,
    LFO_SQ
  };

  byte            m_flags;
  byte            m_midiChannel;      // the midi channel for this logical channel;
  byte            m_bendRange;        // pitch bend range
  float           m_bend;             // pitch bend amount (MIDI note units)
  unsigned int    m_attack;           // attack phase step (increment per ms of 0xFFFF max level)
  unsigned int    m_release;          // release phase step (increment per ms of 0xFFFF max level)
  
  int             m_lfoCount;         // lfo counter value 0-65535  
  int             m_lfoStep;          // counts per ms (lfo freq);
  byte            m_lfoWave;          // lfo waveform
    
  char            m_tremLevel;        // intensity of amplitude modulation (0-127)
  float           m_tremelo;          // amplitude modulation 0 to +1.0 (fraction of full amplitude)

  char            m_vibLevel;         // intensity of pitch modulation (0-127)
  float           m_vibrato;          // pitch modulation -12.0 to +12.0 (fractional midi notes)

  byte            m_portaLevel;      // portamento time for mono modes
  byte            m_portaTarget;     // target note for portamento
  float           m_portaStep;       // pitch step per ms
  float           m_portamento;      // actual note played by portamento engine

  
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



