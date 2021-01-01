; General Float Routines


; float AHL to s16 HL
ftos16:
  ld c,a ; preserve sign
  and $7F ; |AHL|
  cp 62 ; check for underflow
  jr z,ftos16UC ; Underflow with carry
  jr nc,ftos16NoU ; no underflow
  ld h,0 \ ld l,a ; HL = 0
  ; underflow error
  ret

ftos16UC:
  sla h ; get msb
  ld h,0 \ ld l,h ; hl = 0
  rl l ; put rounded bit
  ; underflow error
  ret z ; can't invert
ftos16SgnOut:
  bit 7,c ; Negative
  jp nz,NegHL ; invert
  ret

ftos16NoU:
  cp 92 ; check for overflow
  jr nc,ftos16OF ; overflow
  cp 78 ; check for partial overflow ( int(AHL) & 0xFFFF )
  jr nc,ftos16POF ; partial overflow
  cp 71 ; check for 8 bits
  jr c,ftos168bits
  neg \ add a,78 ; 78-A
  jr z,ftos16SgnOut ; a == 0
  ld b,a
ftos16NoOFLoop:
  srl h \ rr l ; hl >>= 1
  djnz ftos16NoOFLoop
  jr nc,ftos16SgnOut ; no rounding, done
  inc l ; round
  jr nz,ftos16SgnOut ; done rounding, return
  inc h ; continue rounding
  jp p,ftos16SgnOut ; done rounding, return
  res 7,h ; clear sign
  ; overflow error
  jr ftos16SgnOut

ftos168bits: ; shift 8
  neg \ add a,70 ; 70-A
  ld l,h \ ld h,0 ; hl >>= 8
  jr z,ftos16SgnOut ; a == 0
  ld b,a
  jr ftos16NoOFLoop

ftos16OF: ; Overflow
  ld h,0 \ ld l,h
  ; overflow error
  ret

ftos16POF: ; Partial overflow (low bits in range)
  cp 87 ; check for byte swap
  jr nc,ftos16POF8bit
  sub 78 ; shift count
  ld b,a
ftos16POFLoop:
  sla l \ rl h ; hl <<= 1
  djnz ftos16POFLoop
  ; overflow error
  jr ftos16SgnOut
ftos16POF8bit:
  sub 87
  ld h,l \ ld l,0
  jr z,ftos16SgnOut
  ld b,a
  jr ftos16POFLoop



; float AHL to uint AHL
ftou24:
  and $7F ; no negative
  jr z,ftou24Zero ; AHL == 0
  sub 78 ; exp(AHL)-15
  ret z ; No shifting necessary, a == 0
  jr nc,ftou24LShift ; a > 0: Shift left
  ; a < 0: Shift right
  neg ; |a|
  cp 17 ; underflow?
  ; underflow signal
  jr nc,ftou24Zero ; shifts out of range
  ; no signal
  ld b,a ; # of shifts
  xor a ; high register = 0
ftou24RLoop:
  srl h \ rr l ; hl >>= 1
  djnz ftou24RLoop
  ret nc ; no rounding
  inc hl ; round
  cp h \ ret nz ; done rounding
  cp l \ ret nz ; done rounding
  inc a ; overflow to high reg
  ret

ftou24LShift:
  cp 24 ; overflow?
  ; overflow signal
  jr nc,ftou24Zero
  ; no signal
  ld b,a ; # of shifts
  xor a ; high register = 0
ftou24LLoop:
  sla l \ rl h \ rla ; AHL <<= 1
  djnz ftou24LLoop
  ret

ftou24Zero:
  xor a \ ld h,a \ ld l,a ; AHL = 0
  ret



; s8 A to float AHL
s8tof:
  bit 7,a ; z = sign
  jr z,s8tofloatPos
  neg ; two's complement ignoring bit 0
  ld h,a \ ld l,0
  ld a,70 | $80 ; exp(AHL) = 6, sgn(AHL) = 1
  jr fNormalize
s8tofloatPos:
  ld h,a \ ld l,0
  ld a,70 ; exp(AHL) = 6
  jr fNormalize



; Truncates fractional component of AHL
fInt: ; int(AHL) - no rounding
  ld c,a ; preserve exp(AHL)
  and $7F ; exp(AHL)
  sub 63 ; denormalized exponent
  jr c,fIntZero ; exponent < 0, return 0
  sub 15 ; # of fractional bits in HL
  jr c,fIntCont ; some
  ld a,c ; no change
  ret
fIntCont:
  ld de,$8000 ; AND mask
  add a,15 ; make not negative
  ld b,a ; # of shifts
  jr z,fIntFinish ; 0 shifts
fIntLoop:
  sra d \ rr e ; (s16) de >>= 1
  djnz fIntLoop
fIntFinish:
  ld a,h \ and d \ ld h,a ; h &= d
  ld a,l \ and e \ ld l,a ; l &= e
  ld a,c ; restore exp(AHL)
  ret
fIntZero:
  ld a,c \ and $80 ; sgn(AHL)
  ld h,0 \ ld l,h ; hl = 0
  ret



; float AHL = fTrunc(AHLDE)
; AHL = round(AHLDE)
; Rounds AHLDE to AHL
; TStates: 19,28,37,52 / 11 B
fTrunc:
  bit 7,d \ ret z ; No rounding
  inc l \ ret nz
  inc h \ ret nz ; increment
  ; Overflowed, renormalize
  scf ; set carry
  rr h \ rr l
  inc a
  ret



; float AHL = fNormalize(AHL)
; Renormalizes AHL to standard form (1.mantissa x 2^exp)
fNormalize:
  ; if(|a| == 0) return 0 ?
  bit 7,h
  ret nz ; Normalized
  ; todo:
  ;  if(h == 0){
  ;   if(l == 0){
  ;    a &= 0x80; return; // underflow
  ;   }else{
  ;    l=h; l=0; a-=8; // skip 8 shifts
  ;   }
  ;  }
fNormalizeShift:
  dec a
  jr z,fNormalizePUF ; Positive Underflow
  jp pe,fNormalizeNUF ; Negative Underflow
  sla l \ rl h ; HL <<= 1
  ret m ; H sign bit set = normalized
  jr fNormalizeShift

fNormalizePUF:
  ld h,a \ ld l,a ; AHL = 0
  ; Underflow Error?
  ret
fNormalizeNUF:
  ld a,$80 \ ld h,0 \ ld l,h ; AHL = -0
  ; Underflow Error?
  ret


; Compares AHL to CDE
; Return Flags:
;  c    AHL < CDE
;  nc   AHL >= CDE
;  z    AHL == CDE
;  nz   AHL != CDE
; Destroys: b (b == a)
fCmp:
  ld b,a
  push bc ; store original values
  xor $80 \ ld b,a ; b + 128
  ld a,c \ xor $80 \ ld c,a ; c + 128
  
  ld a,b
  cp c ; a - c
  jr nz,fCmpRet
  
  ld a,h
  cp d ; h - d
  jr nz,fCmpRet
  
  ld a,l
  cp e ; l - e
fCmpRet:
  pop bc ; restore exponents
  ld a,b
  ret





fRand:
;  call Rand
;  or a
;  jr nz,fRandNZ
;  ld h,a \ ld l,a
;  ret
;fRandNZ:
;  ld h,a \ ld l,0
;  ld a,$46
;  call fNormalize
;  ld c,$46 \ ld de,$FF00
;  call fDiv ; (float)Rand / 255.0
;  ret

; Random angle from -pi to pi
fRandAngle:
  call Rand
  call s8tof
  ld c,$39 \ ld de,$C910 ; cde = pi/128
  call fMult ; ahl = Rand * pi/128
  ret


; Returns a random byte in A
Rand:
  push hl
  ld hl,(randData)
  ld a,r
  ld d,a
  ld e,(hl)
  add hl,de
  add a,l
  xor h
  ld (randData),hl
  pop hl
  ret

; Random number seed pointer
randData:
  .dw $
