///////////////////////////////////////////////////////////
// POKEYPIG
// hotchk155/2015
///////////////////////////////////////////////////////////
#define PULSE_PERIOD 20
#define HOLD_PERIOD 500
#define LONGHOLD_PERIOD 5000

class CControlPanel {
  char m_endPulse;
  unsigned int m_buttonPress;
public:
  byte m_buttonHold;
  enum {
      NOHOLD = 0,
      HOLD,
      LONGHOLD
  };
  CControlPanel() {
    m_endPulse = 0;
    m_buttonPress = 0;
    m_buttonHold = NOHOLD;
  }
  void pulse() 
  {
    digitalWrite(P_LED2, 1);
    m_endPulse = PULSE_PERIOD;
  }
  byte run() {
    if(m_endPulse) {
      if(--m_endPulse == 0) {
        digitalWrite(P_LED2, 0);
      }
    }
    m_buttonHold = NOHOLD;
    if(digitalRead(P_BUTTON)) {
      m_buttonPress = 0;
    }
    else {
      if(m_buttonPress > LONGHOLD_PERIOD)
        m_buttonHold = LONGHOLD;
      else if(++m_buttonPress > HOLD_PERIOD)
        m_buttonHold = HOLD;
      else
        m_buttonHold = NOHOLD;
    }
  }
  void led1(byte v) {
    digitalWrite(P_LED1, !!v);
  }
  void led2(byte v) {
    digitalWrite(P_LED2, !!v);
  }
  void flashCode(char count) 
  {
    while(count-- > 0) {
      digitalWrite(P_LED1, 1);
      delay(200);
      digitalWrite(P_LED1, 0);
      delay(200);
    }
  }
};

