```
  .org $8000
  port_b=$6000
  port_a=$6001
  ddr_b=$6002
  ddr_a=$6003
  r0=$5001
  r1=$5002
  N=%11001000
  E=%10000110
  P=%10001100
  G=%11000010
  _A=%10001000
  R=%11001110
loop:
  jsr erase
  sta ddr_a
  lda #N
  sta port_a
  lda #$ff
  sta ddr_b
  lda #E
  sta port_b
  lda #P
  sta r0
  jsr erase
  sta ddr_a 
  lda #G
  sta port_a
  lda #$ff
  sta ddr_b
  lda #E
  sta port_b
  lda #_A
  sta r0
  lda #R
  sta r1
  
  jmp loop
erase:
  lda #$ff
  sta ddr_a
  sta port_a
  sta ddr_b
  sta port_b
  sta r0
  sta r1
  rts
  
  .org $fffc
  .word $8000,0
```
