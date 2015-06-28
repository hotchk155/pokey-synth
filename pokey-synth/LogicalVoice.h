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
  void assign(CLogicalChannel *lch, CPokeyChannel *pch);
  void range(byte v);
  void update();
  void reset();
};


