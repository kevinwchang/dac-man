global _ticks
global _rfreq, _rwave, _rvol
global _waveform, _vol_modify

#include <xc.inc>
INDF0 equ 0xFEF ; XC8 forgot this


psect isrVars,class=BANK0,space=1

toggle: ds 1
acc1: ds 3 // channel 1 20-bit accumulator
acc2: ds 2 // channel 2 16-bit accumulator
acc3: ds 2 // channel 3 16-bit accumulator
chan: ds 1 // current channel being sent to output; increments every iteration (0-indexed: chan == 0 means channel 1)
fsr0h_temp: ds 1
fsr0l_temp: ds 1


psect isr,class=CODE,abs,ovrld

org     0x2008

highIsr:
    ; red LED off
    bsf     TRISC, 6, c

    ; clear Timer2-to-PR2-match interrupt flag
    bcf     PIR1, PIR1_TMR2IF_POSN, c

    ; select bank 0 (assumption: all variables are in the same bank)
    movlb   0

    decfsz    toggle, f, b
    bra       highIsrDone
    movlw   3
    movwf toggle, b

    ; save context
    movff    FSR0H, fsr0h_temp
    movff    FSR0L, fsr0l_temp

    ; ticks++
    infsnz  BANKMASK(_ticks), f, b
    incf    BANKMASK(_ticks)+1, f, b

    ; acc1 += rfreq[0]
    movf    BANKMASK(_rfreq), w, b
    addwf   BANKMASK(acc1), f, b
    movf    BANKMASK(_rfreq)+1, w, b
    addwfc  BANKMASK(acc1)+1, f, b
    movf    BANKMASK(_rfreq)+2, w, b
    addwfc  BANKMASK(acc1)+2, f, b

    ; acc2 += rfreq[1]
    movf    BANKMASK(_rfreq)+4, w, b
    addwf   BANKMASK(acc2), f, b
    movf    BANKMASK(_rfreq)+5, w, b
    addwfc  BANKMASK(acc2)+1, f, b

    ; acc3 += rfreq[2]
    movf    BANKMASK(_rfreq)+8, w, b
    addwf   BANKMASK(acc3), f, b
    movf    BANKMASK(_rfreq)+9, w, b
    addwfc  BANKMASK(acc3)+1, f, b

    lfsr    0, 0x200 ; point FSR0 at waveform table

    incf    BANKMASK(chan), f, b    ; channel++
    dcfsnz  BANKMASK(chan), w, b    ; if (chan == 1)
    bra     channel2                ;   goto channel2
    dcfsnz  WREG, w, c              ; if (chan == 2)
    bra     channel3                ;   goto channel3
    clrf    BANKMASK(chan), b       ; chan >= 3; reset to 0 and continue to channel1

channel1:
    swapf   BANKMASK(acc1)+2, w, b      ; W = acc1u[3:0] << 4 | acc1u[7:4] >> 4
    andlw   0xf0                        ; W = acc1u[3:0] << 4
    btfsc   BANKMASK(acc1)+1, 7, b
    bsf     WREG, 3, c                  ; W = acc1u[3:0] << 4 | acc1h[7] >> 4
    iorwf   BANKMASK(_rwave), w, b      ; W = (acc1u[3:0] << 4 | acc1h[7] >> 4) | rwave[0]  (assumption: wave is 3 bits)
    movwf   FSR0L, c
    swapf   INDF0, w, c                 ; W = waveform_sample << 4                          (assumption: waveform samples are 4 bits)
    iorwf   BANKMASK(_rvol), w, b       ; W = (waveform_sample << 4) | rvol[0]              (assumption: vol is 4 bits)
    bra     modifyVolume

channel2:
    movf    BANKMASK(acc2)+1, w, b      ; W = acc2h
    andlw   0xf8                        ; W = acc2h & 0xf8
    iorwf   BANKMASK(_rwave)+1, w, b    ; W = (acc2h & 0xf8) | rwave[1]         (assumption: wave is 3 bits)
    movwf   FSR0L, c
    swapf   INDF0, w, c                 ; W = waveform_sample << 4              (assumption: waveform samples are 4 bits)
    iorwf   BANKMASK(_rvol)+1, w, b     ; W = (waveform_sample << 4) | rvol[1]  (assumption: vol is 4 bits)
    bra     modifyVolume

channel3:
    movf    BANKMASK(acc3)+1, w, b      ; W = acc3h
    andlw   0xf8                        ; W = acc3h & 0xf8
    iorwf   BANKMASK(_rwave)+2, w, b    ; W = (acc3h & 0xf8) | rwave[2]         (assumption: wave is 3 bits)
    movwf   FSR0L, c
    swapf   INDF0, w, c                 ; W = waveform_sample << 4              (assumption: waveform samples are 4 bits)
    iorwf   BANKMASK(_rvol)+2, w, b     ; W = (waveform_sample << 4) | rvol[2]  (assumption: vol is 4 bits)
    bra     modifyVolume

modifyVolume:
    lfsr    0, 0x300                ; point FSR0 at volume modification table
    movwf   FSR0L, c
    movff   INDF0, CCPR1L         ; DACR = volume_modified_waveform_sample    (assumption: volume-modified waveform samples are 5 bits)

    ; restore context
    movff   fsr0h_temp, FSR0H
    movff   fsr0l_temp, FSR0L

highIsrDone:
    ; red LED on
    bcf     TRISC, 6, c

    retfie  f

