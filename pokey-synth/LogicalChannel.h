class CLogicalChannel;
///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{
  CPokeyChannel *m_pch;   // physical pokey channel
public:  
  byte m_note;  // MIDI note
  byte m_vel;  // MIDI velocity
  
  CLogicalVoice();
  void assign(CPokeyChannel *pch);
  void trig(byte note, byte vel, CLogicalChannel *lch);
  void untrig(CLogicalChannel *lch);
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
    FLAG_FULLVELOCITY    = 0x01
  };
  float m_bend;
  byte   m_bendRange;               // pitch bend range
  byte   m_midiChannel;             // the midi channel for this logical channel;
  byte   m_flags;
  //float  m_deltaVolume;             // volume delta (for tremelo effect etc)
  //float  m_deltaPitch;              // pitch delta
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
};



