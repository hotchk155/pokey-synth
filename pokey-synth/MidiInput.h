///////////////////////////////////////////////////////////
//
// POKEYSYNTH 
// hotchk155/2015
//
///////////////////////////////////////////////////////////
class CMidiInput 
{
  byte m_runningStatus;
  byte m_numParams;
  char m_paramIndex;
  
public:
  byte m_params[2];
  
  ////////////////////////////////////////////////////////////////////////////////
  void init() {
    Serial.begin(31250);    
    reset();
  }

  ////////////////////////////////////////////////////////////////////////////////
  void reset() {
    Serial.flush();
    byte m_runningStatus = 0;
    byte m_numParams = 0;
    char m_paramIndex = 0;
  }

  ////////////////////////////////////////////////////////////////////////////////
  byte read()
  {
    // loop while we have incoming MIDI serial data
    while(Serial.available())
    {    
      // fetch the next byte
      byte ch = Serial.read();
  
      // REALTIME MESSAGE
      if((ch & 0xf0) == 0xf0)
      {
        // ignore
      }      
      // CHANNEL STATUS MESSAGE
      else if(!!(ch & 0x80))
      {
        m_paramIndex = 0;
        m_runningStatus = ch; 
        switch(ch & 0xF0)
        {
        case 0xA0: //  Aftertouch  1  key  touch  
        case 0xC0: //  Patch change  1  instrument #   
        case 0xD0: //  Channel Pressure  1  pressure  
          m_numParams = 1;
          break;    
        case 0x80: //  Note-off  2  key  velocity  
        case 0x90: //  Note-on  2  key  veolcity  
        case 0xB0: //  Continuous controller  2  controller #  controller value  
        case 0xE0: //  Pitch bend  2  lsb (7 bits)  msb (7 bits)  
        default:
          m_numParams = 2;
          break;        
        }
      }    
      else if(m_runningStatus)
      {
        m_params[m_paramIndex++] = ch;
        if(m_paramIndex >= m_numParams)
        {
          m_paramIndex = 0;
          switch(m_runningStatus & 0xF0)
          {
          case 0x80: // note off
          case 0x90: // note on
          case 0xB0: // CC
          case 0xE0: // bend
            return m_runningStatus; 
          }
        }
      }
    }
    return 0;
  }
};

CMidiInput MidiInput;
