///////////////////////////////////////////////////////////
//
// POKEYSYNTH 
// hotchk155/2015
//
///////////////////////////////////////////////////////////
#include "Arduino.h"
#include "PortIO.h"
#include "PokeySynth.h"
#include "Pokey.h"

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY CHANNEL
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
CPokeyChannel::CPokeyChannel() {
  m_mode = CHAN_NONE;
  m_div_reg = CPokey::NO_REG;
  m_ctrl_reg = CPokey::NO_REG;
  m_div2_reg = CPokey::NO_REG;
  m_ctrl = DIST_NONE;
  m_flags = FLAG_NODIV;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::configure(CPokey *pokey, byte div_reg, byte ctrl_reg, byte div2_reg) {
  m_div_reg = div_reg;
  m_ctrl_reg = ctrl_reg;
  m_div2_reg = div2_reg;
  m_flags = FLAG_NODIV;
  m_pokey = pokey;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::test() {
  m_pokey->write(8, B01010000); // lsbit 0=higher 1=lower freq
  m_pokey->write(0, 2); //lsb
  m_pokey->write(1, 0);
  m_pokey->write(2, 10); //msb
  m_pokey->write(3, 0b10101111);
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::mode(byte mode) {
  m_mode = mode;
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::reset() {
  m_pokey->write(m_div_reg, 0);
  m_pokey->write(m_ctrl_reg, 0);
  if(m_div2_reg != CPokey::NO_REG) {
    m_pokey->write(m_div2_reg, 0);
  }
  m_ctrl = DIST_NONE;
  m_flags = FLAG_NODIV;
} 
///////////////////////////////////////////////////////////  
byte CPokeyChannel::hz_to_div8(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long v;
  if(m_pokey->m_audctl & CPokey::AUDCTL_DIVLOW) {
    v = hz*452;
  }
  else {
    v = hz*113;
  }  
  int div = 4000000.0/v;
  if(div < 0 || div > 255)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////  
unsigned int CPokeyChannel::hz_to_div16(float hz)
{
  if(hz<1) {
    return 0;
  }  
  unsigned long div = 1000000.0/hz;
  if(div < 0 || div > 65535)
    return 0;
  return div;
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::dist(int mode) {
  byte v = m_ctrl & ~DIST_MASK;
  v|=(mode & DIST_MASK);
  if(v != m_ctrl) {
    m_ctrl = v;
    m_pokey->write(m_ctrl_reg, m_ctrl);
  }
}
///////////////////////////////////////////////////////////  
// Accept a level 0-127 and set appropriate dist level
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
void CPokeyChannel::pitch(float hz) {    unsigned int v;
  switch(m_mode) {
    case CHAN_8:
    case CHAN_HPF:
      v = hz_to_div8(hz);
      if(v) {
        m_pokey->write(m_div_reg, v);
        m_flags &= ~FLAG_NODIV;
        return;
      }
      break;      
    case CHAN_16:    
      v = hz_to_div16(hz);
      if(v) {
        m_pokey->write(m_div_reg, (v>>8));
        m_pokey->write(m_div2_reg, (v & 0x00FF));
        m_flags &= ~FLAG_NODIV;
        return;
      }
      break;
  }
  // bad divider
  m_flags |= FLAG_NODIV;
  vol(0);
}

///////////////////////////////////////////////////////////  
// Volume is provided as 0-15
void CPokeyChannel::vol(byte level) {
  if(m_flags & FLAG_NODIV) {
    level = 0;
  }
  byte v = m_ctrl & 0xF0;
  v|=(level & 0x0F);
  if(v != m_ctrl) {
    m_ctrl = v;
    m_pokey->write(m_ctrl_reg, m_ctrl);
  }  
}

///////////////////////////////////////////////////////////  
void CPokeyChannel::hpf(int hz) {
  if(m_mode == CHAN_HPF) {
    byte v = hz_to_div8(hz);
    m_pokey->write(m_div2_reg, v);
  }
}
///////////////////////////////////////////////////////////  
void CPokeyChannel::hpf_lev(byte lev) {
  hpf((lev&0x7F)*10);
}

///////////////////////////////////////////////////////////  
//
// PHYSICAL POKEY DEVICE
//
///////////////////////////////////////////////////////////  

///////////////////////////////////////////////////////////  
CPokey::CPokey(byte which) 
{
  m_which = which;
  m_mode = MODE_UNDEFINED;
  m_audctl = 0;
  reset();
  
  m_chan[0].configure(this, AUDF1, AUDC1, NO_REG);
  m_chan[1].configure(this, AUDF2, AUDC2, AUDF1);
  m_chan[2].configure(this, AUDF3, AUDC3, NO_REG);
  m_chan[3].configure(this, AUDF4, AUDC4, AUDF3);
}
///////////////////////////////////////////////////////////  
int CPokey::configure(int mode, CPokeyChannel **channels) {
  int i;
  for(i=0; i<4; ++i) {
    m_chan[i].reset();      
  }
  switch(mode) {
  case MODE_8:
  case MODE_8L:
    audctl(0);
    range(MODE_8 == mode);
    for(i=0; i<4; ++i) {
      m_chan[i].mode(CPokeyChannel::CHAN_8);
      channels[i] = &m_chan[i];
    }
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 4;
  case MODE_8HPF:
  case MODE_8LHPF:
    audctl(CPokey::AUDCTL_CHAN1HPF | CPokey::AUDCTL_CHAN3HPF);        
    range(MODE_8HPF == mode);
    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
    m_chan[2].mode(CPokeyChannel::CHAN_HPF);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[2];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;
  case MODE_8_8HPF:
  case MODE_8L_8LHPF:
    audctl(AUDCTL_CHAN3HPF);        
    range(MODE_8_8HPF == mode);
    m_chan[0].mode(CPokeyChannel::CHAN_8);
    m_chan[1].mode(CPokeyChannel::CHAN_8);
    m_chan[2].mode(CPokeyChannel::CHAN_HPF);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    channels[2] = &m_chan[2];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 3;      
  case MODE_8_8_16:  
  case MODE_8L_8L_16:
    audctl(CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN3NODIV);        
    range(MODE_8_8_16 == mode);
    m_chan[0].mode(CPokeyChannel::CHAN_8);
    m_chan[1].mode(CPokeyChannel::CHAN_8);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[1];
    channels[2] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 3;      
  case MODE_8HPF_16: 
  case MODE_8LHPF_16:
    audctl(CPokey::AUDCTL_CHAN1HPF | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN3NODIV);        
    range(MODE_8HPF_16 == mode);
    m_chan[0].mode(CPokeyChannel::CHAN_HPF);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
    channels[0] = &m_chan[0];
    channels[1] = &m_chan[3];
    write(STIMER, 1);        
    write(SKCTL, 3);
    return 2;      
  case MODE_16:
    audctl(CPokey::AUDCTL_CHAN1DIVSCASCADE | CPokey::AUDCTL_CHAN3DIVSCASCADE | AUDCTL_CHAN1NODIV | AUDCTL_CHAN3NODIV);        
    m_chan[1].mode(CPokeyChannel::CHAN_16);
    m_chan[3].mode(CPokeyChannel::CHAN_16);
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
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, LOW);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, LOW);
  }
  delayMicroseconds(100);
  if(m_which) {
    _DIGITAL_WRITE(P_CS1, HIGH);
  }
  else {  
    _DIGITAL_WRITE(P_CS0, HIGH);
  }
  /*
  digitalWrite(P_AD0, !!(addr&0x01));
  digitalWrite(P_AD1, !!(addr&0x02));
  digitalWrite(P_AD2, !!(addr&0x04));
  digitalWrite(P_AD3, !!(addr&0x08));    
  digitalWrite(P_DB0, !!(data&0x01));
  digitalWrite(P_DB1, !!(data&0x02));
  digitalWrite(P_DB2, !!(data&0x04));
  digitalWrite(P_DB3, !!(data&0x08));
  digitalWrite(P_DB4, !!(data&0x10));
  digitalWrite(P_DB5, !!(data&0x20));
  digitalWrite(P_DB6, !!(data&0x40));
  digitalWrite(P_DB7, !!(data&0x80));      
  if(m_which) {
    digitalWrite(P_CS1, LOW);
  }
  else {  
    digitalWrite(P_CS0, LOW);
  }
  delayMicroseconds(100);
  if(m_which) {
    digitalWrite(P_CS1, HIGH);
  }
  else {  
    digitalWrite(P_CS0, HIGH);
  }
  */
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
  write(SKCTL, 3);
  write(STIMER, 1);        
}
///////////////////////////////////////////////////////////
void CPokey::range(byte v) {
  if(v) {
    audctl(m_audctl & ~AUDCTL_DIVLOW);
  } 
  else {    
    audctl(m_audctl | AUDCTL_DIVLOW);
  }
}
    
