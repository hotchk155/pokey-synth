///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CLogicalChannel;

extern CPokey Pokey1;
extern CPokey Pokey2;

#define VOICE_POKEY2 0x80  // OR'd with the voice to indicate POKEY2 

///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{  
  CLogicalChannel *m_lch;
  byte m_voice;  // physical POKEY voice number

public:  
  ENVELOPE_STATE m_amp;
  ENVELOPE_STATE m_mod;
  
  char m_midi_note;             // assigned MIDI note (0-127)
  char m_midi_vel;              // note velocity 0-1.0
  char  m_detuneFactor;

  
  
  CLogicalVoice();
  void assign(CLogicalChannel *lch, byte voice);
  void div8_high(byte v);
  void dist_poly9(byte v);
  void update();
  void reset();
  void quiet();
  void test();
};


