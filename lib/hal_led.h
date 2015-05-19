#ifndef hal_led_h
#define hal_led_h

#define INIT_IO P1DIR = 0x00; P2DIR = 0x00; \
                P3DIR = 0x00; P4DIR = 0x00; \
                P5DIR = 0x00; P6DIR = 0x00; \
                P1OUT = 0xff; P2OUT = 0xff; \
                P3OUT = 0xff; P4OUT = 0xff; \
                P5OUT = 0xff; P6OUT = 0xff

#define LED_ON(x) P4OUT&=~(0x01<<x);P4DIR|=(0x01<<x)

#define LED_OFF(x) P4DIR&=~(0x01<<x);P4OUT|=(0x01<<x)

#define LED_STATE_ON(x) P4DIR|=0x0f;P4OUT&=~(x)

#define LED_STATE_OFF(x) P4DIR&=0xf0;P4OUT|=0x0f

#endif