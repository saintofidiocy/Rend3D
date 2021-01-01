; Division Routines

;#define IXAcc

; AHL = AHL / CDE
; !!! Destroys IX
fDiv:
  push de
  ld b,a ; BHL = AHL
  and $7F
  jr z, fDivZ
  ld d,a
  xor b
  ld b,a
  
  ld a,c
  and $7F
  jr z,fDivOverflow
  ld e,a
  xor c
  xor b
  ld b,a
  
  ld a,d
  sub e
  add a,63
  jp p,fDivInBounds
  jp po,fDivUnderflow
  jr fDivOverflow
  
fDivInBounds:
  jr z,fDivUnderflow
  ld c,a
  ld a,$7F
  cp c
  jr z,fDivOverflow
  cp d
  jr z,fDivOverflow
  cp e
  jr z,fDivUnderflow
  
  pop de
  ld a,$80 \ xor d \ or e
  jr z,fDivIdentity
  ld a,h \ xor d \ jr nz,fDivCont
  ld a,l \ xor c \ jr z,fDivOne
fDivCont:
  ld a,b \ or c
  push af
  ; - prep inputs -
  ld bc,$8000
;#ifdef IXAcc
;  ld ixh,c
;  ld ixl,c
;#else ; use stack
;  push hl
;  ld hl,$0000
;  ex (sp),hl ; (sp) = $0000
;  pop ix
;#endif
  ld ix,$0000
  
  ; if hl < de
  ld a,h
  cp d
  jr c,fDivPrep ; h < d
  jr nz,fDivLoop-1 ; h > d
  ; h == d
  ld a,l
  cp e
  jr c,fDivLoop-1 ; l >= e
fDivPrep:
  xor a
  sla l \ rl h \ rla ; hl <<= 1
  ex (sp),hl
  dec h ; exp--
  ex (sp),hl
  jr fDivSub
  
  xor a
fDivLoop:
  or a
  jr nz,fDivSub
  ld a,d
  cp h
  jr c,fDivSub
  jr nz,fDivNoSub
  ld a,l
  cp e
  jr c,fDivNoSub
fDivSub:
  call NegDE
  add hl,de
  call NegDE
#ifdef IXAcc
  ld a,ixh \ or b \ ld ixh,a
  ld a,ixl \ or c \ ld ixl,a
#else ; use stack
  push ix
  ex (sp),hl
  ld a,h \ or b \ ld h,a
  ld a,l \ or c \ ld l,a
  ex (sp),hl
  pop ix
#endif
fDivNoSub:
  xor a
  sla l \ rl h \ rla
  srl b \ rr c
  jr nc,fDivLoop
#ifdef IXAcc
  ld d,ixh
  ld e,ixl
#else
  push ix
  pop de
#endif
  ex de,hl ; hl = result, de = remainder
  ; - prep outputs -
  pop af
  ; normalize ?
  ret

fDivZ        .equ fMultZ         ; duplicate output
fDivOverflow .equ fMultOverflow  ; duplicate output
fDivUnderflow:
  pop de ; clear stack
  xor a
  ld h,a
  ld l,a
  ld d,a
  ld e,a
  or b
  ret ; AHLDE = sgn + 0
fDivOne      .equ fMultOne + 1 ; skip ex de,hl
fDivIdentity:
  ld d,e ; de = 0
  ld a,c \ or b ; sgn + exp
  ret



fDiv2:
  or a \ ret z ; AHL == 0
  cp $7F \ ret z ; AHL == inf
  cp $80 \ ret z ; AHL == -0
  cp $FF \ ret z ; AHL == -inf
  dec a
  ret

fMult2:
  or a \ ret z ; AHL == 0
  cp $7F \ ret z ; AHL == inf
  cp $80 \ ret z ; AHL == -0
  cp $FF \ ret z ; AHL == -inf
  inc a
  ret

fDiv2CDE:
  push af
  ld a,c
  or a \ jr z,fDiv2CExit ; AHL == 0
  cp $7F \ jr z,fDiv2CExit ; AHL == inf
  cp $80 \ jr z,fDiv2CExit ; AHL == -0
  cp $FF \ jr z,fDiv2CExit ; AHL == -inf
  dec c
fDiv2CExit:
  pop af
  ret

fMult2CDE:
  push af
  ld a,c
  or a \ jr z,fMult2CExit ; AHL == 0
  cp $7F \ jr z,fMult2CExit ; AHL == inf
  cp $80 \ jr z,fMult2CExit ; AHL == -0
  cp $FF \ jr z,fMult2CExit ; AHL == -inf
  inc c
fMult2CExit:
  pop af
  ret

