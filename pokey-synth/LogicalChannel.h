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
  byte m_envPhase;
  unsigned int m_envLevel;  // envelope modulated level (0xFFFF is max)
  float m_note;       // assigned MIDI note
  byte m_vel;        // assigned MIDI note velocity
  
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
    FLAG_UNISON          = 0x02      // when this flag is set, all available voices play in unison
  };
  
  byte            m_flags;
  byte            m_midiChannel;      // the midi channel for this logical channel;
  byte            m_bendRange;        // pitch bend range
  float           m_bend;             // pitch bend amount (MIDI note units)
  unsigned int    m_attack;           // attack phase step (increment per ms of 0xFFFF max level)
  unsigned int    m_release;          // release phase step (increment per ms of 0xFFFF max level)
  
  int             m_tremStep;
  unsigned int    m_tremCounter;
  byte            m_tremLevel;
  float           m_tremelo;

  int             m_vibStep;
  unsigned int    m_vibCounter;
  byte            m_vibLevel;
  float           m_vibrato;

  float           m_detune;          // detune amount for voices played in unison
  
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
};



