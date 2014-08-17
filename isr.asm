global _ticks
global _rfreq1, _rwave1, _rvol1
global _rfreq2, _rwave2, _rvol2
global _rfreq3, _rwave3, _rvol3
global _waveform, _vol_modify

#include <xc.inc>
INDF0 equ 0xFEF ; XC8 forgot this


psect isrVars,class=BANK0,space=1

acc1: ds 3 // channel 1 20-bit accumulator
acc2: ds 2 // channel 2 16-bit accumulator
acc3: ds 2 // channel 3 16-bit accumulator
chan: ds 1 // current channel being sent to output; increments every iteration (0-indexed: chan = 0 means channel 1)
fsr0h_temp: ds 1
fsr0l_temp: ds 1


psect isr,class=CODE,abs,ovrld

org     0x2008

highIsr:
    ; red LED off
    bsf     TRISC, 6, c

    ; clear Timer2-to-PR2-match interrupt flag
    bcf     PIR1, PIR1_TMR2IF_POSN, c

    ; select bank 1 (assumption: all variables are in the same bank)
    movlb   0 ;

    ; save context
    movff    FSR0H, BANKMASK(fsr0h_temp)
    movff    FSR0L, BANKMASK(fsr0l_temp)

    ; ticks++
    infsnz  BANKMASK(_ticks), f, b
    incf    BANKMASK(_ticks)+1, f, b

    ; acc1 += rfreq1
    movf    BANKMASK(_rfreq1), w, b
    addwf   BANKMASK(acc1), f, b
    movf    BANKMASK(_rfreq1)+1, w, b
    addwfc  BANKMASK(acc1)+1, f, b
    movf    BANKMASK(_rfreq1)+2, w, b
    addwfc  BANKMASK(acc1)+2, f, b

    ; acc2 += rfreq2
    movf    BANKMASK(_rfreq2), w, b
    addwf   BANKMASK(acc2), f, b
    movf    BANKMASK(_rfreq2)+1, w, b
    addwfc  BANKMASK(acc2)+1, f, b

    ; acc3 += rfreq3
    movf    BANKMASK(_rfreq3), w, b
    addwf   BANKMASK(acc3), f, b
    movf    BANKMASK(_rfreq3)+1, w, b
    addwfc  BANKMASK(acc3)+1, f, b

    lfsr    0, 0x200 ; point FSR0 at waveform table

bra     channel2

    incf    BANKMASK(chan), f, b    ; channel++
    dcfsnz  BANKMASK(chan), w, b    ; if (chan == 1)
    bra     channel2                ;   goto channel2
    dcfsnz  WREG, w, c              ; if (chan == 2)
    bra     channel3                ;   goto channel3
    clrf    BANKMASK(chan), b       ; chan >= 3; reset to 0 and continue to channel1

channel1:
    movlw   15
    movwf   VREFCON2, c
    bra     highIsrDone

channel2:
    movf    BANKMASK(acc2)+1, w, b  ; W = acc2h
    andlw   0xf8                    ; W = acc2h & 0xf8
    iorwf   BANKMASK(_rwave2), w, b ; W = (acc2h & 0xf8) | rwave2               (assumption: rwave2 is 3 bits)
    movwf   FSR0L, c
    swapf   INDF0, w, c             ; W = waveform_sample << 4                  (assumption: waveform samples are 4 bits)
    iorwf   BANKMASK(_rvol2), w, b  ; W = (waveform_sample << 4) | rvol2        (assumption: rvol2 is 4 bits)
    lfsr    0, 0x300                ; point FSR0 at volume modification table
    movwf   FSR0L, c
    movff   INDF0, VREFCON2         ; DACR = volume_modified_waveform_sample    (assumption: volume-modified waveform samples are 5 bits)
    bra     highIsrDone

channel3:
    movlw   15
    movwf   VREFCON2, c
    bra     highIsrDone

highIsrDone:
    ; red LED on
    bcf     TRISC, 6, c

    ; restore context
    movff   BANKMASK(fsr0h_temp), FSR0H
    movff   BANKMASK(fsr0l_temp), FSR0L

    retfie  f

