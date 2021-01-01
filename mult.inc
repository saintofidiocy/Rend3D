; Multiplication Routines



; float AHL = FSquare(AHL)
; AHL = AHL * AHL
; Does fMult(AHL, AHL)
fSquare:
  ld c,a
  ld d,h
  ld e,l ; CDE = AHL
  jr fMult



; float AHL = FCube(AHL)
; AHL = AHL * AHL * AHL
; Does fMult(fMult(AHL, AHL), AHL)
fCube:
  ld c,a
  ld d,h
  ld e,l ; CDE = AHL
  push bc
  push de
  call fMult
  pop de
  pop bc
  jr fMult



; u32 HLDE = Mult(u16 HL, u16 DE)
; HLDE = DE * HL
; 16-bit multiplication with 32-bit result (HL high bits, DE low bits)
Mult:
  push hl ; Preserve Hin
  ld c,l
  xor a
  ld h,a
  ld l,a
  ld b,a
  cp e ; if(DE == 0)
  jr nz,MultNZ
  cp d
  jr z,MultZ ; 0*HL=0, so exit
MultNZ: ; DE != 0
  cp c ; if(c!=0)
  jr z,MultLZ
  
  push DE
MultL: ; AHL = BDE*C
  bit 0,c ; if(C&1)
  jr z, MultLNoAdd
  add HL,DE \ adc a,b ; AHL += BDE
MultLNoAdd:
  sla e \ rl d \ rl b ; BDE <<= 1
  srl c ; c >>= 1
  jr nz,MultL
  
  ; reorder accumulators for next operation
  ld c,l ; Lout
  ld l,a
  ld a,h
  ld h,c
  pop de
  
MultLZ:
  pop BC ; B = Hin
  ld c,h
  push bc ; Save Lout
  
  ; B = 0?
  ld c,a
  xor a
  cp b
  ld h,a ; clear high bits (might as well do it here)
  ld a,c
  jr z,MultHZ
  
  ; Reorder registers
  ld c,d
  ld d,b
  ld b,h ; clear high bits
  
MultH: ; HLA += BCE*D
  bit 0,d ; if(D&1)
  jr z,MultHNoAdd
  add a,e \ adc hl,bc ; HLA += BCE
MultHNoAdd:
  sla e \ rl c \ rl b ; BCE <<= 1
  srl d ; d >>= 1
  jr nz,MultH

MultHZ:
  pop de ; restore Lout
  ld d,a
  ret ; HLDE

MultZ:
  pop bc ; Clear stack
  ret ; HLDE = 0



; float AHL = fMult(float AHL, float CDE)
; AHL[DE] = AHL * CDE
; Float multiplication - DE contains extra precision bits (though no more sig. figs)
fMult:
  push de ; free up some regs
  
  ld b,a ; BHL = AHL
  and $7F
  jr z,fMultZ ; 0*CDE
  ld d,a ; exp(B)
  xor b
  ld b,a ; sgn(B)
  
  ld a,c
  and $7F
  jr z,fMultZ ; BHL*0
  ld e,a ; exp(C)
  xor c
  xor b
  ld b,a ; sgn(B) * sgn(C)
  
  ; todo: Check for infinity
  
  ; calc new exponent
  ld a,d
  sub 63
  add a,e ; exp(B) + exp(C)
  jp pe,fMultOverflow ; (exp(B) + exp(C) + 63) > 127
  jp m,fMultUnderflow ; (exp(B) + exp(C) + 63) < 0
  ld c,a ; c = exp
  
  pop de ; restore regs
  
  ld a,$80 ; hl == $8000 ?
  xor h
  or l
  jr z,fMultOne ; 1*CDE -- or 2^n*CDE
  ex de,hl ; de == $8000 ?
  ld a,$80
  xor h
  or l
  jr z,fMultOne ; HLE*1 -- or HLE*2^n
  
  ld a,b ; sgn + exp
  or c
  push af
  call Mult ; HLDE = HL * DE
  pop af
  inc a ; does it fix?
  bit 7,h ; Normalized?
  ret nz ; AHLDE
  dec a ; does it fix?
  sla e ; Normalize result
  rl d
  rl l
  rl h
  ret



fMultUnderflow: ; return 0
  xor a
; In: a = 0
fMultZ:
  ; 0*inf == ?
  pop de ; clear stack
  ld h,a
  ld l,a
  ld d,a
  ld e,a
  ret ; AHLDE = 0

; In: none
fMultOverflow:
  ; was input inf or NaN to begin with? ... who cares, inf == NaN
  pop de ; clear stack
  ld h,$00
  ld l,h
  ld d,h
  ld e,h
  ld a,b ; sgn
  or $7F
  ret ; AHLDE = sgn + $7F00000000 (infinity)

; In: A = 0, HL = $8000, DE = val
fMultOne:
  ex de,hl ; HLDE = $DDEE0000
  ld d,a
  ld a,c ; A = exp + sgn
  or b
  ret






; s16 DE = Mult(s16 HL, s16 DE)
; DE = DE * HL
; Signed 14.2 fixed-point multiplication
sMultFixed:
  ld a,h
  xor d
  push af ; sgnout = sgn(hl) * sgn(de)
  bit 7,h
  call nz,NegHL ; if(hl < 0) hl = -hl;
  bit 7,d
  call nz,NegDE ; if(de < 0) de = -de;
  call Mult ; DE = HL * DE -- 28.4 precision
  
  ; shift precision amount
  sra l \ rr d \ rr e
  inc de ; round result
  sra l \ rr d \ rr e
  
  pop af
  ret p ; positive result
  jp NegDE ; if(signout) return -DE
