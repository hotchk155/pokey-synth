///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CPokeySynth
{
  void defaultToneConfig(TONE_CONFIG *conf);
  void configure();
  void quiet();
public:  
  
  // MIDI input manager
  CMidiInput m_midiInput;
  
  // Control panel (buttons, LEDs) manager
  CControlPanel m_controlPanel;    

  // The following objects manage the POKEY devices
  CPokey m_pokey1;
  CPokey m_pokey2;

  CLogicalVoice  m_voice[8];
  byte m_voiceCount;
  
  TONE_CONFIG m_conf[2];
  CLogicalChannel m_chan[2];
  
  byte m_numPOKEYs;
  byte m_dualPatch;   

  // EEPROM storage manager
  CStorage m_storage;

  CPokeySynth();
  void init();
  void run();
};

