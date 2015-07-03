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
  
  float m_note;             // assigned MIDI note (0-127)
  float m_vol;              // note velocity 0-1.0
  char  m_detuneFactor;  
  
  CLogicalVoice();
  void assign(CLogicalChannel *lch, CPokeyChannel *pch);
  void range(byte v);
  void update();
  void reset();
//  void quiet();  
};


