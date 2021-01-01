; Quaternion Routines

; RotM = Q1
RotMtoQ1:
  ld ix,RotM
  ld iy,Q1
  ld a,(ix+RotM00)
  ld l,(ix+RotM00+1)
  ld h,(ix+RotM00+2)
  ld c,(ix+RotM11)
  ld e,(ix+RotM11+1)
  ld d,(ix+RotM11+2)
  call fAdd ; AHL = M00+M11
  ld c,(ix+RotM22)
  ld e,(ix+RotM22+1)
  ld d,(ix+RotM22+2)
  call fAdd ; AHL = M00+M11+M22
  cp 0 ; A <= 0
  jp z,RotMtoQB ; Relative branch exceeded :(
  jp m,RotMtoQB
  
;RotMtoQA:
  call Sqrt1plusX
  ld c,a ; CHL = AHL
  call fDiv2 ;dec a ; AHL /= 2
  call fMult2CDE ;inc c ; CHL *= 2
  ld (IY+Q_A+0),a
  ld (IY+Q_A+1),l
  ld (IY+Q_A+2),h
  push bc \ push hl ; w
  push bc \ push hl
  push bc \ push hl
  
  ld a,(ix+RotM21)
  ld l,(ix+RotM21+1)
  ld h,(ix+RotM21+2)
  ld c,(ix+RotM12)
  ld e,(ix+RotM12+1)
  ld d,(ix+RotM12+2)
  call fSub ; AHL = M21-M12
  pop de \ pop bc ; w
  call fDiv ; (M21 - M12) / w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_B+0),a
  ld (iy+Q_B+1),l
  ld (iy+Q_B+2),h
  
  ld a,(ix+RotM02)
  ld l,(ix+RotM02+1)
  ld h,(ix+RotM02+2)
  ld c,(ix+RotM20)
  ld e,(ix+RotM20+1)
  ld d,(ix+RotM20+2)
  call fSub ; AHL = M02-M20
  pop de \ pop bc ; w
  call fDiv ; (M02-M20) / w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_C+0),a
  ld (iy+Q_C+1),l
  ld (iy+Q_C+2),h
  
  ld a,(ix+RotM10)
  ld l,(ix+RotM10+1)
  ld h,(ix+RotM10+2)
  ld c,(ix+RotM01)
  ld e,(ix+RotM01+1)
  ld d,(ix+RotM01+2)
  call fSub ; AHL = M10-M01
  pop de \ pop bc ; w
  call fDiv ; (M10-M01) / w
  ld (iy+Q_D+0),a
  ld (iy+Q_D+1),l
  ld (iy+Q_D+2),h
  ret

RotMtoQB:
  ld a,(ix+RotM00)
  ld l,(ix+RotM00+1)
  ld h,(ix+RotM00+2)
  call fCmp ; AHL <= CDE -- M00 <= M11
  ld b,c
  push bc \ push de ; m11
  jp z,RotMtoQC ; ACH == CDE ; Relative branch exceeded :(
  jp c,RotMtoQC ; AHL < CDE ; Relative branch exceeded :(
  ld c,(ix+RotM22)
  ld e,(ix+RotM22+1)
  ld d,(ix+RotM22+2)
  call fCmp ; AHL <= CDE -- M00 <= M22
  jp z,RotMtoQC ; AHL == CDE  ; Relative branch exceeded :(
  jp c,RotMtoQC ; AHL < CDE  ; Relative branch exceeded :(
  
  call fSub ; AHL = m00-m22
  pop de \ pop bc ; m11
  call fSub ; AHL = M00-m11-m22
  call Sqrt1plusX ; w
  ld c,a ; CHL = AHL
  call fDiv2 ;dec a ; AHL / 2
  call fMult2CDE ;inc c ; AHL * 2 = w
  ld (iy+Q_B+0),a
  ld (iy+Q_B+1),l
  ld (iy+Q_B+2),h
  
  push bc \ push hl ; w
  push bc \ push hl
  push bc \ push hl
  
  ld a,(ix+RotM21)
  ld l,(ix+RotM21+1)
  ld h,(ix+RotM21+2)
  ld c,(ix+RotM12)
  ld e,(ix+RotM12+1)
  ld d,(ix+RotM12+2)
  call fSub ; AHL = M21-M12
  pop de \ pop bc
  call fDiv ; AHL = (M21-M12)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_A+0),a
  ld (iy+Q_A+1),l
  ld (iy+Q_A+2),h
  
  ld a,(ix+RotM01)
  ld l,(ix+RotM01+1)
  ld h,(ix+RotM01+2)
  ld c,(ix+RotM10)
  ld e,(ix+RotM10+1)
  ld d,(ix+RotM10+2)
  call fAdd ; AHL = M01+M10
  pop de \ pop bc
  call fDiv ; AHL = (M01+M10)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_C+0),a
  ld (iy+Q_C+1),l
  ld (iy+Q_C+2),h
  
  ld a,(ix+RotM02)
  ld l,(ix+RotM02+1)
  ld h,(ix+RotM02+2)
  ld c,(ix+RotM20)
  ld e,(ix+RotM20+1)
  ld d,(ix+RotM20+2)
  call fAdd ; AHL = M02+M20
  pop de \ pop bc
  call fDiv ; AHL = (M02+M20)/w
  ld (iy+Q_D+0),a
  ld (iy+Q_D+1),l
  ld (iy+Q_D+2),h
  ret

RotMtoQC:
  pop hl \ pop af ; m11
  ; CDE = m22
  call fCmp ; m11 <= m22
  jp z,RotMtoQD ; m11 == m22 ; Relative branch exceeded :(
  jp c,RotMtoQD ; m11 < m22 ; Relative branch exceeded :(
  
  call fSub ; AHL = m11-m22
  ld c,(ix+RotM00)
  ld e,(ix+RotM00+1)
  ld d,(ix+RotM00+2)
  call fSub ; AHL = m11-m00-m22
  call Sqrt1plusX
  ld c,a
  dec c
  inc a
  ld (iy+Q_C+0),a
  ld (iy+Q_C+1),l
  ld (iy+Q_C+2),h
  
  push bc \ push de ; w
  push bc \ push de
  push bc \ push de
  
  ld a,(ix+RotM02)
  ld l,(ix+RotM02+1)
  ld h,(ix+RotM02+2)
  ld c,(ix+RotM20)
  ld e,(ix+RotM20+1)
  ld d,(ix+RotM20+2)
  call fSub ; AHL = M02-M20
  pop de \ pop bc
  call fDiv ; AHL = (M02-M20)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_A+0),a
  ld (iy+Q_A+1),l
  ld (iy+Q_A+2),h
  
  ld a,(ix+RotM01)
  ld l,(ix+RotM01+1)
  ld h,(ix+RotM01+2)
  ld c,(ix+RotM10)
  ld e,(ix+RotM10+1)
  ld d,(ix+RotM10+2)
  call fAdd ; AHL = M01+M10
  pop de \ pop bc
  call fDiv ; AHL = (M01+M10)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_B+0),a
  ld (iy+Q_B+1),l
  ld (iy+Q_B+2),h
  
  ld a,(ix+RotM12)
  ld l,(ix+RotM12+1)
  ld h,(ix+RotM12+2)
  ld c,(ix+RotM21)
  ld e,(ix+RotM21+1)
  ld d,(ix+RotM21+2)
  call fAdd ; AHL = M12+M21
  pop de \ pop bc
  call fDiv ; AHL = (M12+M21)/w
  ld (iy+Q_D+0),a
  ld (iy+Q_D+1),l
  ld (iy+Q_D+2),h
  ret

RotMtoQD:
  ex de,hl ; swap AHL / CDE
  ld b,a \ ld a,c \ ld c,b
  ; AHL = m22, CDE = m11
  call fSub ; AHL = m22 - m11
  ld c,(ix+RotM00)
  ld e,(ix+RotM00+1)
  ld d,(ix+RotM00+2)
  call fSub ; AHL = m22 - m11 - m00
  call Sqrt1plusX
  ld c,a
  call fDiv2	;dec a ; AHL / 2
  call fMult2CDE	;inc c ; CHL * 2
  ld (iy+Q_D+0),a
  ld (iy+Q_D+1),l
  ld (iy+Q_D+2),h
  
  push bc \ push de ; w
  push bc \ push de
  push bc \ push de
  
  ld a,(ix+RotM10)
  ld l,(ix+RotM10+1)
  ld h,(ix+RotM10+2)
  ld c,(ix+RotM01)
  ld e,(ix+RotM01+1)
  ld d,(ix+RotM01+2)
  call fSub ; AHL = M10-M01
  pop de \ pop bc
  call fDiv ; AHL = (M10-M01)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_A+0),a
  ld (iy+Q_A+1),l
  ld (iy+Q_A+2),h
  
  ld a,(ix+RotM02)
  ld l,(ix+RotM02+1)
  ld h,(ix+RotM02+2)
  ld c,(ix+RotM20)
  ld e,(ix+RotM20+1)
  ld d,(ix+RotM20+2)
  call fAdd ; AHL = M02+M20
  pop de \ pop bc
  call fDiv ; AHL = (M02+M20)/w
  ld ix,RotM ; clobbered by fDiv
  ld (iy+Q_B+0),a
  ld (iy+Q_B+1),l
  ld (iy+Q_B+2),h
  
  ld a,(ix+RotM21)
  ld l,(ix+RotM21+1)
  ld h,(ix+RotM21+2)
  ld c,(ix+RotM12)
  ld e,(ix+RotM12+1)
  ld d,(ix+RotM12+2)
  call fAdd ; AHL = M21+M12
  pop de \ pop bc
  call fDiv ; AHL = (M21+M12)/w
  ld (iy+Q_C+0),a
  ld (iy+Q_C+1),l
  ld (iy+Q_C+2),h
  ret



; Qr = IX*IY
; There has to be a better way
; Destroys: A,BC,DE,HL (&QC[3]+2), ABCDE'
QMult:
  ld a,(IX+0)
  ld l,(IX+1)
  ld h,(IX+2)
  ld c,(IY+0)
  ld e,(IY+1)
  ld d,(IY+2)
  call fMult ; AHLDE = QA[0] * QB[0]
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+3)
  ld l,(IX+4)
  ld h,(IX+5)
  ld c,(IY+3)
  ld e,(IY+4)
  ld d,(IY+5)
  call fMult ; AHLDE = QA[1] * QB[1]
  xor $80 ; neg
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+6)
  ld l,(IX+7)
  ld h,(IX+8)
  ld c,(IY+6)
  ld e,(IY+7)
  ld d,(IY+8)
  call fMult ; AHLDE = QA[2] * QB[2]
  xor $80
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+9)
  ld l,(IX+10)
  ld h,(IX+11)
  ld c,(IY+9)
  ld e,(IY+10)
  ld d,(IY+11)
  call fMult ; AHLDE = QA[3] * QB[3]
  xor $80
  call fAddAcc ; AHLDE += AHLDE'
  call fTrunc
  ex de,hl
  ld hl,QC ; QC[0] = result
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl ; QC[1]
  
  ld a,(IX+0)
  ld l,(IX+1)
  ld h,(IX+2)
  ld c,(IY+3*1+0)
  ld e,(IY+3*1+1)
  ld d,(IY+3*1+2)
  call fMult ; AHLDE = QA[0] * QB[1]
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+3)
  ld l,(IX+4)
  ld h,(IX+5)
  ld c,(IY+3*0+0)
  ld e,(IY+3*0+1)
  ld d,(IY+3*0+2)
  call fMult ; AHLDE = QA[1] * QB[0]
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+6)
  ld l,(IX+7)
  ld h,(IX+8)
  ld c,(IY+3*3+0)
  ld e,(IY+3*3+1)
  ld d,(IY+3*3+2)
  call fMult ; AHLDE = QA[2] * QB[3]
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+9)
  ld l,(IX+10)
  ld h,(IX+11)
  ld c,(IY+3*2+0)
  ld e,(IY+3*2+1)
  ld d,(IY+3*2+2)
  call fMult ; AHLDE = QA[3] * QB[2]
  xor $80
  call fAddAcc ; AHLDE += AHLDE'
  call fTrunc
  ex de,hl
  pop hl ; QC[1] = result
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl ; QC[2]
  
  ld a,(IX+0)
  ld l,(IX+1)
  ld h,(IX+2)
  ld c,(IY+3*2+0)
  ld e,(IY+3*2+1)
  ld d,(IY+3*2+2)
  call fMult ; AHLDE = QA[0] * QB[2]
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+3)
  ld l,(IX+4)
  ld h,(IX+5)
  ld c,(IY+3*3+0)
  ld e,(IY+3*3+1)
  ld d,(IY+3*3+2)
  call fMult ; AHLDE = QA[1] * QB[3]
  xor $80
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+6)
  ld l,(IX+7)
  ld h,(IX+8)
  ld c,(IY+3*0+0)
  ld e,(IY+3*0+1)
  ld d,(IY+3*0+2)
  call fMult ; AHLDE = QA[2] * QB[0]
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+9)
  ld l,(IX+10)
  ld h,(IX+11)
  ld c,(IY+3*1+0)
  ld e,(IY+3*1+1)
  ld d,(IY+3*1+2)
  call fMult ; AHLDE = QA[3] * QB[1]
  call fAddAcc ; AHLDE += AHLDE'
  call fTrunc
  ex de,hl
  pop hl ; QC[2] = result
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl ; QC[3]
  
  ld a,(IX+0)
  ld l,(IX+1)
  ld h,(IX+2)
  ld c,(IY+3*3+0)
  ld e,(IY+3*3+1)
  ld d,(IY+3*3+2)
  call fMult ; AHLDE = QA[0] * QB[3]
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+3)
  ld l,(IX+4)
  ld h,(IX+5)
  ld c,(IY+3*2+0)
  ld e,(IY+3*2+1)
  ld d,(IY+3*2+2)
  call fMult ; AHLDE = QA[1] * QB[2]
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+6)
  ld l,(IX+7)
  ld h,(IX+8)
  ld c,(IY+3*1+0)
  ld e,(IY+3*1+1)
  ld d,(IY+3*1+2)
  call fMult ; AHLDE = QA[2] * QB[1]
  xor $80
  call fAddAcc ; AHLDE += AHLDE'
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+9)
  ld l,(IX+10)
  ld h,(IX+11)
  ld c,(IY+3*0+0)
  ld e,(IY+3*0+1)
  ld d,(IY+3*0+2)
  call fMult ; AHLDE = QA[3] * QB[0]
  call fAddAcc ; AHLDE += AHLDE'
  call fTrunc
  ex de,hl
  pop hl ; QC[3] = result
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d
  
  ret


; QInverse(Quat *Qd, Quat *Qs) // Qd = Qs^(-1)
; (DE) = QInverse( (HL) )
; Copies the inverse of the Quat pointed to by HL to the Quat pointed by DE
; Destroys: A (exp(Qd(3)), BC ($8000)
; TStates: 295 / 41 Bytes
QInverse:
  ;push HL
  ;push DE ; store original pointers
  ld bc,$8009 ; b = $80, c = 9
  
  ; Qd(0) = Qs(0)
  ldi \ ldi \ ldi
  
  ; Qd(1) = -Qs(1)
  ld a,(hl) \ xor b \ ld (de),a ; negate
  inc hl \ inc de
  ldi \ ldi
  
  ; Qd(2) = -Qs(2)
  ld a,(hl) \ xor b \ ld (de),a ; negate
  inc hl \ inc de
  ldi \ ldi
  
  ; Qd(3) = -Qs(3)
  ld a,(hl) \ xor b \ ld (de),a ; negate
  inc hl \ inc de
  ldi \ ldi
  
  ;pop de
  ;pop hl ; restore pointers
  ret

