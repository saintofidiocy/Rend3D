; Addition/Subtraction Routines


fSub: ; AHL - CDE
  ld b,a
  ld a,c
  xor $80 ; neg A
  ld c,a
  ld a,b
  ; fall through to fAdd


fAdd: ; AHL + CDE
  push de
  
  ld b,a ; BHL = AHL
  and $7F ; exp(BHL)
  jr z,fAddRetCDE ; 0+CDE
  cp $7F ; exp(BHL) == inf
  jr z,fAddRetBHL ; inf+CDE
  ld d,a ; d = exp(BHL)
  xor b ; sgn(BHL)
  ld b,a
  
  ld a,c
  and $7F ; exp(CDE)
  jr z,fAddRetBHL ; BHL+0
  cp $7F ; exp(CDE) == inf
  jr z,fAddRetCDE ; BHL+inf
  ld e,a ; e = exp(CDE)
  xor c ; sgn(CDE)
  ld c,a
  
  ld a,d
  sub e ; exp(AHL) - exp(CDE)
  jr c,fAddSwap ; exp(AHL) < exp(CDE)
  jr nz,fAddNoSwap ; exp(AHL) > exp(CDE)
  ; exp(AHL) == exp(CDE)
  
  ld a,h \ ld e,l
  ex (sp),hl ; HL = CDE, (sp) = AHL
  cp h
  jr c,fAddEExpSwap ; AHL < CDE
  jr nz,fAddEExpNoSwap ; AHL > CDE
  
  ld a,e \ cp l
  jr c,fAddEExpSwap ; AHL < CDE
  jr nz,fAddEExpNoSwap ; AHL > CDE
  
  ; |AHL| == |CDE|
  pop hl
  ld a,b \ xor c ; sgn(AHL) * sgn(CDE)
  jr nz,fAddRetZero ; AHL + -AHL
  ; AHL + AHL
  ld a,d \ or b ; sgn + exp
  jp fMult2 ; AHL * 2
  ;ret


; higher up for jr's

fAddRetCDE:
  ld a,c
  pop hl ; de
  ret

fAddRetBHL:
  ld a,b \ or d ; sgn(AHL) + exp(AHL)
  pop de
  ret
  
fAddRetBDE:
  ld a,b \ or d ; sgn(AHL) + exp(AHL)
  pop hl
  ret

fAddRetZero:
  xor a
  ld h,a \ ld l,a
  ret


fAddEExpSwap:
  ld e,b \ ld b,c \ ld c,e
  xor a
  jr fAddNoSwap ; already swapped -- and need to restore hl and (sp)

fAddEExpNoSwap:
  xor a
  jr fAddCont ; hl and (sp) already switched

fAddSwap:
; ex (sp),hl ; hl <--> de
  neg ; abs(exp(AHL) - exp(CDE))
  ld d,e
  ld e,b \ ld b,c \ ld c,e
  jr fAddCont
fAddNoSwap:
  ex (sp),hl
fAddCont:
  jr z,fAddNoShift
  cp 16 ; shifts >= 16
  jr nc,fAddRetBDE
  cp 8 ; shifts < 8
  jr c,fAddShift
fAdd8shift:
  scf ; carry = 1
  rl l ; carry = round bit, zero = false
  ld l,h \ ld h,0
  jr nc,fAdd8NoRound
  inc hl
fAdd8NoRound:
  sub 8 ; did 8 shifts
; or a ; a == 0? -- redundant?
  jr z,fAddNoShift
fAddShift:
  srl h \ rr l ; hl >> 1
  dec a
  jr nz,fAddShift
  jr nc,fAddNoShift ; no round
  inc hl
fAddNoShift:
  ld a,b \ or d ; sgn + exp
  ld d,a
  ld a,b \ xor c ; sgn(ahl) * sgn(cde)
  jr nz,fAddSub
;Add:
  ld a,d ; a = sgn+exp
  pop de
  add hl,de ; ahl + cde
  ret nc ; no rounding
  rr h \ rr l ; hl >> 1
  inc a
  ret

fAddSub:
  call NegHL
  ld a,d ; a = sgn+exp
  pop de
  add hl,de ; ahl - cde
  bit 7,h ; Normalized?
  ret nz ; yes
fAddNormalize:
  dec a
  ret z ; Underflow
  jp pe,fAddRetZero ; signed underflow
  sla l \ rl h ; hl << 1
  ret m ; Normalized
  jr fAddNormalize




; if exp(BHL) = 0 ret CDE ; 0+CDE
; if exp(CDE) = 0 ret BHL ; BHL+0
; if BHL == CDE \ sgn(BHL)++ \ ret BHL
; if abs(exp(BHL)-exp(CDE))



; double AHLDE = fAdd(double AHLDE, double AHLDE')
; AHLDE = AHLDE + AHLDE'
; Double-precision addition for adding multiplication results
fAddAcc:
  ld b,a
  and $7F ; exp(AHLDE)
  jr z,fAddAccZero ; 0 + AHLDE'
  cp $7F ; exp == inf
  jr z,fAddAccOverflow ; inf + AHLDE'
  ld c,a ; c = exp
  xor b ; sgn(AHLDE)
  ld b,a ; b = sgn
  
  ld a,c ; a = exp(AHLDE)
  
  exx \ ex af,af' ; Load AHLDE'
  ld b,a
  and $7F ; exp(AHLDE')
  jr z,fAddAccSmall ; AHLDE + 0
  cp $7f ; exp == inf
  jr z,fAddAccOverflow ; AHLDE + inf
  ld c,a ; c = exp
  xor b ; sgn(AHLDE')
  ld b,a ; b = sgn
  
  ex af,af' ; a = exp(AHLDE)
  sub c ; exp - exp'
  jr c,fAddAccSwap    ; exp < exp'
  jr nz,fAddAccNoSwap ; exp > exp'
; exp == exp':
  push bc ; sgn,exp
  exx ; HLDE'
  push de ; lo(HLDE')
  push hl ; hi(HLDE')
  exx ; HLDE
  
  pop bc ; hi(HLDE')
  ld a,h \ cp b
  jr c,fAddAccEESwap ; HLDE < HLDE'
  jr nz,fAddAccEENoSwap ; HLDE > HLDE'
  ld a,l \ cp c
  jr c,fAddAccEESwap ; HLDE < HLDE'
  jr nz,fAddAccEENoSwap ; HLDE > HLDE'
  pop bc ; lo(HLDE')
  ld a,h \ cp b
  jr c,fAddAccEESwap+1 ; HLDE < HLDE'
  jr nz,fAddAccEENoSwap+1 ; HLDE > HLDE'
  ld a,l \ cp c
  jr c,fAddAccEESwap+1 ; HLDE < HLDE'
  jr nz,fAddAccEENoSwap+1 ; HLDE > HLDE'
 
  ; AHLDE == AHLDE'
  pop bc ; sgn,exp
  ld a,b ; sgn(AHLDE)
  exx ; HLDE'
  xor b ; sgn(AHLDE) ? sgn(AHLDE')
  jr nz,fAddAccRetZero ; sgn(A) + -sgn(A') = 0
  ld a,b \ or c ; sgn + exp
  inc a ; a+a = 2*a
  ret

fAddAccEESwap:
  pop bc ; lo(HLDE')
;EESwap+1:
  pop bc ; sgn,exp
; exx ; HLDE <-> HLDE'
  jr fAddAccNoShift

fAddAccEENoSwap:
  pop bc ; lo(HLDE')
;EENoSwap+1
  pop bc ; sgn,exp
  exx ; HLDE <-> HLDE'
  jr fAddAccNoShift



; Closer to the beginning so jr can reach ...
fAddAccZero: ; 0 + AHLDE', ret AHLDE'
  exx \ ex af,af'
  ret
fAddAccSmall: ; sm + AHLDE', ret AHLDE'
  exx
  ld a,b \ or c ; sgn + exp
  ret
fAddAccOverflow:
  ld a,$7F \ or b ; inf + sgn
  ret
;fAddAccUnderflow:
;   ?
;  ret
fAddAccRetZero:
  xor a
  ld h,a \ ld l,a \ ld d,a \ ld e,a
  ret


fAddAccSwap:
  exx ; HLDE <-> HLDE'
  neg ; a = abs(exp-exp')
fAddAccNoSwap:
  cp 32 ; shifts >= 32 ?
  jr nc,fAddAccSmall
  cp 9 ; shifts < 8 ?
  jr c,fAddAccShift
fAddAcc8Shift: ; hlde >> 8
  scf ; carry = 1
  rl e ; carry = round, nz = 1
  ld e,d \ ld d,l
  jr nc, fAddAccNoRoundDE
  inc e
  jr nz,fAddAccNoRoundDE
  inc d
fAddAccNoRoundDE:
  ld l,h \ ld h,0
  jr nz, fAddAccNoRoundHL
  inc hl
fAddAccNoRoundHL:
  sub 8 ; did 8 shifts
  cp 9 ; shifts < 8 ?
  jr nc,fAddAcc8Shift
  
  or a ; a == 0?
  jr z,fAddAccNoShift
fAddAccShift:
  srl h \ rr l \ rr d \ rr e ; HLDE >> 1
  dec a
  jr nz,fAddAccShift
  jr nc,fAddAccNoShift ; No rounding
  inc e
  jr nz,fAddAccNoShift ; No overflow
  inc d
  jr nz,fAddAccNoShift ; No overflow
  inc hl
fAddAccNoShift:
  push hl \ push de ; push HLDE'
  ld a,b ; a = sgn'
  exx ; Load AHLDE
  xor b ; a = sgn * sgn'
  jr nz,fAddAccSubtract
;Add:
  ld a,b \ or c ; a = sgn + exp
  ex de,hl ; ADEHL
  pop bc ; bc = DE'
  add hl,bc ; add low word
  jr nc,fAddAccNHC ; Overflow?
  inc de
fAddAccNHC:
  ex de,hl ; AHLDE
  pop bc ; bc = HL'
  add hl,bc ; add high word
  ret nc ; no rounding
  rr h \ rr l \ rr d \ rr e ; (HLDE >> 1) | $80000000
  inc a
  ret

fAddAccSubtract:
  ld a,b \ or c ; a = sgn + exp
  ex af,af'
  pop bc ; bc = DE'
  ld a,e \ sub c \ ld e,a ; e -= e'
  ld a,d \ sbc a,b \ ld d,a ; d -= d'
  pop bc ; bc = HL'
  ld a,l \ sbc a,c \ ld l,a ; l -= l'
  ld a,h \ sbc a,b \ ld h,a ; h -= h'
  ex af,af'
  bit 7,h ; normalized?
  ret nz ; yes
fAddAccNormalize:
  dec a
  ret z ; underflow
  jp pe,fAddAccRetZero ; signed underflow -- return 0
  sla e \ rl d \ rl l \ rl h
  ret m ; Normalized
  jr fAddAccNormalize


