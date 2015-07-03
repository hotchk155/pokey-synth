///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CLogicalChannel;
///////////////////////////////////////////////////////////////////////////////////////////
class CLogicalVoice 
{  
  CLogicalChannel *m_lch;
  CPokeyChannel *m_pch;
  
//  CPokeyChannel *m_pch;   // physical pokey channel
public:  
  ENVELOPE_STATE m_amp;
  ENVELOPE_STATE m_mod;
  
  char m_midi_note;             // assigned MIDI note (0-127)
  char m_midi_vel;              // note velocity 0-1.0
  char  m_detuneFactor;  
  
  CLogicalVoice();
  void assign(CLogicalChannel *lch, CPokeyChannel *pch);
  void range(byte v);
  void poly9(byte v);
  void update();
  void reset();
//  void quiet();  
};


