///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
class CPokeySynth
{
  void defaultToneConfig(TONE_CONFIG *conf);
//  void defaultGlobalConfig(GLOBAL_CONFIG *cfg);
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
  
  byte m_dualPatch;   // whether m_chan[1] has a separate patch

  // EEPROM storage manager
  CStorage m_storage;

  // This structure contains the entire, currently active
  // "patch" for the synth and is the data that is stored
  // to EEPROM when patches are saved.
  //GLOBAL_CONFIG m_globalConfig;
  
  
  CPokeySynth();
  void init();
  void run();
};




///////////////////////////////////////////////////////////////////////////////////

