///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

class CPokeySynth
{
   
  void defaultToneConfig(TONE_CONFIG *conf);
  void configure();
public:  
  enum {
    MAX_POKEY = 2,
    MAX_CHANNEL = MAX_POKEY,
    MAX_VOICES_PER_CHANNEL = 4,
    MAX_VOICE = (MAX_CHANNEL * MAX_VOICES_PER_CHANNEL)    
  };
  
  enum {
    DUAL_POKEYS = 0x01   // there are two POKEYs installed
  };
  byte m_flags;
 
  byte m_lastTick;
  byte m_voiceToUpdate;
  
  // MIDI input manager
  CMidiInput m_midiInput;
  
  // Control panel (buttons, LEDs) manager
  CControlPanel m_controlPanel;    

  // List of physcial POKEY devices
  CPokey m_pokey[MAX_POKEY];

  // The loaded "patches"
  TONE_CONFIG m_conf[MAX_CHANNEL];

  // number of POKEY devices actually installed  
//  byte m_numPOKEYs;
  
  // Array of logical voices
  CLogicalVoice  m_voice[MAX_VOICE];
  byte m_voiceCount;
   
  // Array of logical channels (max one per POKEY)
  CLogicalChannel m_chan[MAX_CHANNEL];
  byte m_channelCount;

public:  
  CPokeySynth();
  void test();
  void init();
  void run();
  void quiet();
  void reset();  
};

