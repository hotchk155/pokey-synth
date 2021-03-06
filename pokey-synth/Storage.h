///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////

// NB: EEPROM size is 1024 bytes
/*
///////////////////////////////////////////////////////////
class CStorage 
{
  enum {
    EEPROM_MAGIC_COOKIE  = 0x6703,  // "magic number" that identifies initialised eeprom
    EEADDR_MAGIC_COOKIE  = 0,       // address for magic cookie
    EEADDR_CURRENT_PATCH = 3,       // location to store current patch
    EEADDR_NUM_POKEYS = 4,       // address of configured number 
    EEADDR_PATCHES       = 64,      // base address for patches
    EEPROM_PATCH_SIZE    = 64,      // size allocated per patch
    EEPROM_NUM_PATCHES   = 12        // number of patches that fit in eeprom
  };
public:
  byte getNumPatches() {
    return EEPROM_NUM_PATCHES;
  }
  ///////////////////////////////////////////////////////////
  byte isInitialised() {
    unsigned int d = EEPROM.read(EEADDR_MAGIC_COOKIE);
    d<<=8;
    d|= EEPROM.read(EEADDR_MAGIC_COOKIE+1);
    return (d==EEPROM_MAGIC_COOKIE);
  }
  
  ///////////////////////////////////////////////////////////
  void setInitialised() {
    EEPROM.write(EEADDR_MAGIC_COOKIE, EEPROM_MAGIC_COOKIE>>8);
    EEPROM.write(EEADDR_MAGIC_COOKIE+1, (byte)EEPROM_MAGIC_COOKIE);
  }

  ///////////////////////////////////////////////////////////
  void setCurrentPatch(byte v) {
    EEPROM.write(EEADDR_CURRENT_PATCH, v);
  }

  ///////////////////////////////////////////////////////////
  byte getCurrentPatch() {
    byte v = EEPROM.read(EEADDR_CURRENT_PATCH);
    return constrain(v,0,EEPROM_NUM_PATCHES-1);
  }

  ///////////////////////////////////////////////////////////
  void setNumPokeys(byte v) {
    EEPROM.write(EEADDR_NUM_POKEYS, v);
  }

  ///////////////////////////////////////////////////////////
  byte getNumPokeys() {
    byte v = EEPROM.read(EEADDR_NUM_POKEYS);
    if(v != 1 && v!= 2) {
      setNumPokeys(1);
      return 1;
    }
  }
  
  ///////////////////////////////////////////////////////////
  void savePatch(int index, TONE_CONFIG *block) {
    if(index < 0 || index >= EEPROM_NUM_PATCHES)
      return;
    int addr = EEADDR_PATCHES + EEPROM_PATCH_SIZE * index;
    for(int i=0; i<sizeof(TONE_CONFIG); ++i) {
      EEPROM.write(addr++, ((byte*)block)[i]);      
    }
  }

  ///////////////////////////////////////////////////////////
  void loadPatch(char index, TONE_CONFIG *block) {
    if(index < 0 || index >= EEPROM_NUM_PATCHES)
      return;
    int addr = EEADDR_PATCHES + EEPROM_PATCH_SIZE * index;
    for(int i=0; i<sizeof(TONE_CONFIG); ++i) {
      ((byte*)block)[i] = EEPROM.read(addr++);      
    }    
  }
};*/
