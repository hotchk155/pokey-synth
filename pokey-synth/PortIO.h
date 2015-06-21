///////////////////////////////////////////////////////////////////////////
//
// DIRECT PORT IO DEFINITIONS FOR ARDUINO UNO
//
// hotchk155/2014
//
// Allow inline direct port access via _DIGITAL_WRITE(n) and _DIGITAL_READ(n)
// where n is a literal integer (not a variable or expression)
//
///////////////////////////////////////////////////////////////////////////

#ifndef _PORTIO_DIRECT_H_
#define _PORTIO_DIRECT_H_

// DEFINE ARDUINO PIN TO OUTPUT PORT MAPPING
#define _GET_OUT_FOR_PIN0      PORTD
#define _GET_OUT_FOR_PIN1      PORTD
#define _GET_OUT_FOR_PIN2      PORTD
#define _GET_OUT_FOR_PIN3      PORTD
#define _GET_OUT_FOR_PIN4      PORTD
#define _GET_OUT_FOR_PIN5      PORTD
#define _GET_OUT_FOR_PIN6      PORTD
#define _GET_OUT_FOR_PIN7      PORTD
#define _GET_OUT_FOR_PIN8      PORTB
#define _GET_OUT_FOR_PIN9      PORTB
#define _GET_OUT_FOR_PIN10     PORTB
#define _GET_OUT_FOR_PIN11     PORTB
#define _GET_OUT_FOR_PIN12     PORTB
#define _GET_OUT_FOR_PIN13     PORTB
#define _GET_OUT_FOR_PIN14     PORTC
#define _GET_OUT_FOR_PIN15     PORTC
#define _GET_OUT_FOR_PIN16     PORTC
#define _GET_OUT_FOR_PIN17     PORTC
#define _GET_OUT_FOR_PIN18     PORTC
#define _GET_OUT_FOR_PIN19     PORTC

// DEFINE ARDUINO PIN TO INPUT PORT MAPPING
#define _GET_IN_FOR_PIN0      PIND
#define _GET_IN_FOR_PIN1      PIND
#define _GET_IN_FOR_PIN2      PIND
#define _GET_IN_FOR_PIN3      PIND
#define _GET_IN_FOR_PIN4      PIND
#define _GET_IN_FOR_PIN5      PIND
#define _GET_IN_FOR_PIN6      PIND
#define _GET_IN_FOR_PIN7      PIND
#define _GET_IN_FOR_PIN8      PINB
#define _GET_IN_FOR_PIN9      PINB
#define _GET_IN_FOR_PIN10     PINB
#define _GET_IN_FOR_PIN11     PINB
#define _GET_IN_FOR_PIN12     PINB
#define _GET_IN_FOR_PIN13     PINB
#define _GET_IN_FOR_PIN14     PINC
#define _GET_IN_FOR_PIN15     PINC
#define _GET_IN_FOR_PIN16     PINC
#define _GET_IN_FOR_PIN17     PINC
#define _GET_IN_FOR_PIN18     PINC
#define _GET_IN_FOR_PIN19     PINC

// DEFINE ARDUINO PIN TO PORT BIT
#define _GET_BIT_FOR_PIN0     (1<<0)
#define _GET_BIT_FOR_PIN1     (1<<1)
#define _GET_BIT_FOR_PIN2     (1<<2)
#define _GET_BIT_FOR_PIN3     (1<<3)
#define _GET_BIT_FOR_PIN4     (1<<4)
#define _GET_BIT_FOR_PIN5     (1<<5)
#define _GET_BIT_FOR_PIN6     (1<<6)
#define _GET_BIT_FOR_PIN7     (1<<7)
#define _GET_BIT_FOR_PIN8     (1<<0)
#define _GET_BIT_FOR_PIN9     (1<<1)
#define _GET_BIT_FOR_PIN10    (1<<2)
#define _GET_BIT_FOR_PIN11    (1<<3)
#define _GET_BIT_FOR_PIN12    (1<<4)
#define _GET_BIT_FOR_PIN13    (1<<5)
#define _GET_BIT_FOR_PIN14    (1<<0)
#define _GET_BIT_FOR_PIN15    (1<<1)
#define _GET_BIT_FOR_PIN16    (1<<2)
#define _GET_BIT_FOR_PIN17    (1<<3)
#define _GET_BIT_FOR_PIN18    (1<<4)
#define _GET_BIT_FOR_PIN19    (1<<5)

// MACROS TO GET PIN-SPECIFIC EXPRESSION
#define _GET_IN(n)  _GET_IN_FOR_PIN##n
#define _GET_OUT(n)  _GET_OUT_FOR_PIN##n
#define _GET_BIT(n)  _GET_BIT_FOR_PIN##n

// DEFINE INLINE DIGITAL WRITE/READ
#define _DIGITAL_WRITE(n, s) { if(s) { _GET_OUT(n) |= _GET_BIT(n); } else { _GET_OUT(n) &= ~_GET_BIT(n); } }
#define _DIGITAL_READ(n) (_GET_IN(n) | _GET_BIT(n))

#endif // _PORTIO_DIRECT_H_
