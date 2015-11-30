///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "Defs.h"
#include "PortIO.h"
#include "Pokey.h"

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY CHANNEL
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
// CONSTRUCTOR
CPokeyChannel::CPokeyChannel() {
  //m_mode = CHAN_NONE;
  m_div_reg = CPokey::NO_REG;
  m_ctrl_reg = CPokey::NO_REG;
  m_div2_reg = CPokey::NO_REG;
  m_hpf_reg = CPokey::NO_REG;
  m_ctrl = DIST_NONE;
  m_div = 0;
  m_div2 = 0;  
  m_pokey = NULL;
}

///////////////////////////////////////////////////////////  
// CONFIGURE HARDWARE REGISTERS ASSOCIATED WITH POKEY VOICE
void CPokeyChannel::configure(CPokey *pokey, byte div_reg, byte ctrl_reg, byte div2_reg, byte hpf_reg) {
  m_pokey = pokey;
  m_div_reg = div_reg;
  m_ctrl_reg = ctrl_reg;
  m_div2_reg = div2_reg;
  m_hpf_reg = hpf_reg;
}

///////////////////////////////////////////////////////////  
// SET THE VOICE MODE FOR THE CHANNEL (8BIT, 16BIT, HPF)
//void CPokeyChannel::mode(byte mode) {
//  m_mode = mode;
//}

///////////////////////////////////////////////////////////  
// RESET THE VOICE STATE
void CPokeyChannel::reset() {
  quiet();
} 

///////////////////////////////////////////////////////////  
// STOP ALL SOUND ON THE VOICE
void CPokeyChannel::quiet() {
  m_pokey->write(m_div_reg, 0);
  m_pokey->write(m_ctrl_reg, 0);
  if(m_div2_reg != CPokey::NO_REG) {
    m_pokey->write(m_div2_reg, 0);
  }  
  m_ctrl = 0;
  m_div = 0;
  m_div2 = 0;  
}

///////////////////////////////////////////////////////////  
// FETCH 8 BIT DIVIDER VALUE FOR A SPECIFIC HZ FREQ
byte CPokeyChannel::hz_to_div8(float hz)
{
  if(hz<1) {
    return 0;
  }  
  int div;
  if(m_pokey->m_audctl & CPokey::AUDCTL_DIVLOW) {
    div =  (8769.0/hz) - 0.5;
  }
  else {
    div =  (35695.0/hz) - 0.5;
  }  
  if(div < 0 || div > 255)
    return 0;
  return div;  
}

///////////////////////////////////////////////////////////  
// FETCH 16 BIT DIVIDER VALUE FOR A SPECIFIC HZ FREQ 
unsigned int CPokeyChannel::hz_to_div16(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long div = 0.5 + 1000000.0/hz;
  if(div < 0 || div > 65535)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::pitch(float hz) {    
    switch(m_pokey->m_mode) {
      
      // 8 BIT PITCH MODES
      case CPokey::PCFG_8:
      case CPokey::PCFG_8HPF:
        {
          byte v = hz_to_div8(hz);
          if(!v) {
            quiet();
          }
          else if(v != m_div) {
            m_div = v;
            m_pokey->write(m_div_reg, m_div);
          }
        }
        break;      

      // 16 BIT PITCH MODE
      case CPokey::PCFG_16:    
        {
          unsigned int v = hz_to_div16(hz);
          if(!v) {
            quiet();
          }
          else {
            byte hi = v>>8;
            byte lo = (byte)v;
            if(hi != m_div || lo != m_div2) {
              m_div = hi;
              m_div2 = lo;
              m_pokey->write(m_div_reg, m_div);
              m_pokey->write(m_div2_reg, m_div2);
              return;
            }
          }
        }
        break;
    }
}

///////////////////////////////////////////////////////////  
// SET CHANNEL VOLUME
// Volume is provided as 0-15
void CPokeyChannel::vol(byte level) {
  if(m_div) { // only allow setting of volume if there is a valid pitch
    byte v = m_ctrl & 0xF0;
    v|=(level & 0x0F);
    if(v != m_ctrl) {
      m_ctrl = v;
      m_pokey->write(m_ctrl_reg, m_ctrl);
    }      
  }
}

///////////////////////////////////////////////////////////  
// SET DISTORTION POLYNOMIAL
void CPokeyChannel::dist(int mode) {
  byte v = m_ctrl & ~DIST_MASK;
  v|=(mode & DIST_MASK);
  if(v != m_ctrl) {
    m_ctrl = v;
    m_pokey->write(m_ctrl_reg, m_ctrl);
  }
}

///////////////////////////////////////////////////////////  
// Accept a level 0-127 and set appropriate dist poly
void CPokeyChannel::dist_lev(byte level) {
    switch((int)(level/22.0))
    {
      case 1: dist(DIST_4); break;
      case 2: dist(DIST_5); break;
      case 3: dist(DIST_5_4); break;
      case 4: dist(DIST_5_17); break;
      case 5: dist(DIST_17); break;
      default: dist(DIST_NONE); break;
    }
}

///////////////////////////////////////////////////////////  
// SET CHANNEL HPF DIVIDER USING A HZ FREQ
void CPokeyChannel::hpf_hz(int hz) {
  if(m_pokey->m_mode == CPokey::PCFG_8HPF) {
    byte v = hz_to_div8(hz);
    if(v != m_div2) {
      m_div2 = v;
      m_pokey->write(m_hpf_reg, m_div2);
    }            
  }
}

///////////////////////////////////////////////////////////  
// SET CHANNEL HPF DIVIDER DIRECTLY
// 1-255
void CPokeyChannel::hpf_div(byte div) {
  if(m_pokey->m_mode == CPokey::PCFG_8HPF) {
    if(div != m_div2) {
      m_div2 = div;
      m_pokey->write(m_hpf_reg, m_div2);
    }            
  }
}

///////////////////////////////////////////////////////////  
// SET THE DIVIDER RANGE FOR 8 BIT MODES
void CPokeyChannel::range(byte v) {
  m_pokey->range(v);
}

///////////////////////////////////////////////////////////  
// SET THE DIST POLYNOMIAL TYPE
void CPokeyChannel::poly9(byte v) {
  m_pokey->poly9(v);
}

///////////////////////////////////////////////////////////  
// TEST
void CPokeyChannel::test() {
  m_pokey->write(8, B01010000); // lsbit 0=higher 1=lower freq
  m_pokey->write(0, 2); //lsb
  m_pokey->write(1, 0);
  m_pokey->write(2, 10); //msb
  m_pokey->write(3, 0b10101111);
}

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY DEVICE
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
CPokey::CPokey() 
{
  m_which = 0;
  m_mode = PCFG_NONE;
  m_audctl = 0;
  reset();

  // Each channel 0-3 corresponds to a physical POKEY
  // voice with specific registers controlling it
  m_chan[0].configure(this, AUDF1, AUDC1, NO_REG, AUDF3);
  m_chan[1].configure(this, AUDF2, AUDC2, AUDF1,  AUDF4);
  m_chan[2].configure(this, AUDF3, AUDC3, NO_REG, NO_REG);
  m_chan[3].configure(this, AUDF4, AUDC4, AUDF3,  NO_REG);
  
}

///////////////////////////////////////////////////////////  
void CPokey::which(byte v) {
  m_which = v;
}

///////////////////////////////////////////////////////////  
void CPokey::test() {
  m_chan[0].test();
}

///////////////////////////////////////////////////////////  
int CPokey::configure(int mode, byte lowhz, CPokeyChannel **channels) {
  int i;
  
  // Clean up any previous config
  for(i=0; i<4; ++i) {
    m_chan[i].reset();      
    channels[i] = NULL;
  }

  // Get range bit for audctl
  byte divlow = lowhz? AUDCTL_DIVLOW:0;
  switch(mode) {
    
    // CONFIGURED AS 4 x 8-BIT VOICE
  case PCFG_8:
    audctl(divlow);
    for(i=0; i<4; ++i) {
//      m_chan[i].mode(CPokeyChannel::CHAN_8);
      channels[i] = &m_chan[i];
    }
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 4;

    // CONFIGURED AS 2 x 8-BIT VOICE WITH HPF
  case PCFG_8HPF:
    audctl(divlow | CPokey::AUDCTL_CHAN1HPF | CPokey::AUDCTL_CHAN2HPF);        
//    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
//    m_chan[1].mode(CPokeyChannel::CHAN_HPF);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;
    
    // CONFIGURED AS 2 x 16-BIT VOICE 
  case PCFG_16:
    audctl(divlow | CPokey::AUDCTL_CHAN1DIVSCASCADE | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN1NODIV | AUDCTL_CHAN3NODIV);        
//    m_chan[1].mode(CPokeyChannel::CHAN_16);
//    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[1];
    channels[1] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;          
    
  }
  return 0;
}

///////////////////////////////////////////////////////////
void CPokey::write(byte addr, byte data) 
{      
  _DIGITAL_WRITE(P_RW, LOW);  
  _DIGITAL_WRITE(P_AD0, !!(addr&0x01));
  _DIGITAL_WRITE(P_AD1, !!(addr&0x02));
  _DIGITAL_WRITE(P_AD2, !!(addr&0x04));
  _DIGITAL_WRITE(P_AD3, !!(addr&0x08));    
  _DIGITAL_WRITE(P_DB0, !!(data&0x01));
  _DIGITAL_WRITE(P_DB1, !!(data&0x02));
  _DIGITAL_WRITE(P_DB2, !!(data&0x04));
  _DIGITAL_WRITE(P_DB3, !!(data&0x08));
  _DIGITAL_WRITE(P_DB4, !!(data&0x10));
  _DIGITAL_WRITE(P_DB5, !!(data&0x20));
  _DIGITAL_WRITE(P_DB6, !!(data&0x40));
  _DIGITAL_WRITE(P_DB7, !!(data&0x80));      
  delayMicroseconds(10);
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, LOW);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, LOW);
  }
  delayMicroseconds(10);
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, HIGH);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, HIGH);
  }
}    

///////////////////////////////////////////////////////////
void CPokey::audctl(byte v) {
  write(AUDCTL, v);
  m_audctl = v;
}

///////////////////////////////////////////////////////////
void CPokey::reset() {
  for(int i=0;i<16;++i) {
    write(i, 0);
  }
  write(IRQEN,0);
}

///////////////////////////////////////////////////////////
void CPokey::quiet() {
  for(int i=0;i<4;++i) {
    m_chan[i].quiet();
  }
}

///////////////////////////////////////////////////////////
void CPokey::range(byte v) {
  if(v) {
    if(m_audctl & AUDCTL_DIVLOW) {
      audctl(m_audctl & ~AUDCTL_DIVLOW);
    }
  } 
  else {    
    if(!(m_audctl & AUDCTL_DIVLOW)) {
      audctl(m_audctl | AUDCTL_DIVLOW);
    }
  }
}

///////////////////////////////////////////////////////////    
void CPokey::poly9(byte v) {
  if(v) {
    if(!(m_audctl & AUDCTL_9BITPOLY)) {
      audctl(m_audctl | AUDCTL_9BITPOLY);
    }
  } 
  else {    
    if(m_audctl & AUDCTL_9BITPOLY) {
      audctl(m_audctl & ~AUDCTL_9BITPOLY);
    }
  }
}


