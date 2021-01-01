; Inline; isn't a called function

QuattoRotM:
 ld ix,QC
 ld iy,RotM
 
 ld a,(ix+Q_D)
 ld l,(ix+Q_D+1)
 ld h,(ix+Q_D+2) ; AHL = QR.z
 call fSquare ; z^2
 push af \ push hl \ push de ; push AHLDE, {z^2}
 exx \ ex af,af' ; acc' = z^2
 
 ld a,(ix+Q_C)
 ld l,(ix+Q_C+1)
 ld h,(ix+Q_C+2) ; AHL = QR.y
 call fSquare ; y^2
 push af \ push hl \ push de ; push AHLDE, {z^2, y^2}
 call fAddAcc
 push af \ push hl \ push de ; push AHLDE, {z^2, y^2, z^2+y^2}
 exx \ ex af,af' ; acc' = z^2 + y^2
 
 ld a,(ix+Q_B)
 ld l,(ix+Q_B+1)
 ld h,(ix+Q_B+2) ; AHL = QR.x
 call fSquare ; x^2
 push af \ push hl \ push de ; push AHLDE {z^2, y^2, z^2+y^2, x^2}
 call fAddAcc
 exx \ ex af,af' ; acc' = z^2 + y^2 + x^2
 
 ld a,(ix+Q_A)
 ld l,(ix+Q_A+1)
 ld h,(ix+Q_A+2) ; AHL = QR.r
 call fSquare ; r^2
 call fAddAcc ; AHLDE = z^2 + y^2 + x^2 + r^2
 ld b,a
 and $7F ; == 0 or == -0
 ld a,b
 jp z,RotMIdentity ; AHLDE == 0
 call fTrunc
 
 ex de,hl \ ld c,a ; CDE = AHL
 ld a,$40 \ ld hl,$8000 ; AHL = 2.0
 call fDiv ; S = 2/CDE
 ld (QtR_S),a \ ld (QtR_S+1),hl ; S = AHL
 ld ix,QC ; clobbered by fDiv
  
 
 ld a,(ix+Q_C)
 ld l,(ix+Q_C+1)
 ld h,(ix+Q_C+2)
 ex de,hl \ ld c,a ; CDE = QR.y
 ld a,(ix+Q_B)
 ld l,(ix+Q_B+1)
 ld h,(ix+Q_B+2) ; AHL = QR.x
 call fMult ; B = y*x
 push af \ push hl \ push de ; push B, {z^2, y^2, z^2+y^2, x^2, B}
 exx \ ex af,af' ; acc' = B
 ld a,(ix+Q_A)
 ld l,(ix+Q_A+1)
 ld h,(ix+Q_A+2)
 ex de,hl \ ld c,a ; CDE = QR.r
 ld a,(ix+Q_D)
 ld l,(ix+Q_D+1)
 ld h,(ix+Q_D+2) ; AHL = QR.z
 call fMult ; I = r*z
 push af \ push hl \ push de ; push I, {z^2, y^2, z^2+y^2, x^2, B, I}
 xor $80 ; I = -I
 call fAddAcc ; B-I
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (B-I)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (B-I)*S
 call fTrunc
 ld (iy+RotM01+0),a
 ld (iy+RotM01+1),l
 ld (iy+RotM01+2),h ; M01 = (B-I)*S
 pop de \ pop hl \ pop af ; I
 exx \ ex af,af' ; acc' = I
 pop de \ pop hl \ pop af ; B
 call fAddAcc ; I+B
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (I+B)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (B+I)*S
 call fTrunc
 ld (iy+RotM10+0),a
 ld (iy+RotM10+1),l
 ld (iy+RotM10+2),h ; M10 = (B+I)*S
 
 ld a,(ix+Q_D)
 ld l,(ix+Q_D+1)
 ld h,(ix+Q_D+2)
 ex de,hl \ ld c,a ; CDE = QR.z
 ld a,(ix+Q_B)
 ld l,(ix+Q_B+1)
 ld h,(ix+Q_B+2) ; AHL = QR.x
 call fMult ; C = z*x
 push af \ push hl \ push de ; push C, {z^2, y^2, z^2+y^2, x^2, C}
 exx \ ex af,af' ; acc' = C
 ld a,(ix+Q_A)
 ld l,(ix+Q_A+1)
 ld h,(ix+Q_A+2)
 ex de,hl \ ld c,a ; CDE = QR.r
 ld a,(ix+Q_C)
 ld l,(ix+Q_C+1)
 ld h,(ix+Q_C+2) ; AHL = QR.y
 call fMult ; G = r*y
 push af \ push hl \ push de ; push I, {z^2, y^2, z^2+y^2, x^2, C, G}
 call fAddAcc ; C+G
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (C+G)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (C+G)*S
 call fTrunc
 ld (iy+RotM02+0),a
 ld (iy+RotM02+1),l
 ld (iy+RotM02+2),h ; M02 = (C+G)*S
 pop de \ pop hl \ pop af ; G
 xor $80 ; G = -G
 exx \ ex af,af' ; acc' = G
 pop de \ pop hl \ pop af ; C
 call fAddAcc ; C-G
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (C-G)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (C-G)*S
 call fTrunc
 ld (iy+RotM20+0),a
 ld (iy+RotM20+1),l
 ld (iy+RotM20+2),h ; M20 = (C-G)*S
 
 ld a,(ix+Q_D)
 ld l,(ix+Q_D+1)
 ld h,(ix+Q_D+2)
 ex de,hl \ ld c,a ; CDE = QR.z
 ld a,(ix+Q_C)
 ld l,(ix+Q_C+1)
 ld h,(ix+Q_C+2) ; AHL = QR.y
 call fMult ; F = z*y
 push af \ push hl \ push de ; push F, {z^2, y^2, z^2+y^2, x^2, F}
 exx \ ex af,af' ; acc' = F
 ld a,(ix+Q_A)
 ld l,(ix+Q_A+1)
 ld h,(ix+Q_A+2)
 ex de,hl \ ld c,a ; CDE = QR.r
 ld a,(ix+Q_B)
 ld l,(ix+Q_B+1)
 ld h,(ix+Q_B+2) ; AHL = QR.x
 call fMult ; D = r*x
 push af \ push hl \ push de ; push D, {z^2, y^2, z^2+y^2, x^2, F, D}
 xor $80 ; D = -D
 call fAddAcc ; F-D
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (F-D)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (F-D)*S
 call fTrunc
 ld (iy+RotM12+0),a
 ld (iy+RotM12+1),l
 ld (iy+RotM12+2),h ; M12 = (F-D)*S
 pop de \ pop hl \ pop af ; D
 exx \ ex af,af' ; acc' = D
 pop de \ pop hl \ pop af ; F
 call fAddAcc ; F+D
 call fTrunc
 ex de,hl \ ld c,a ; CDE = (F+D)
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (F+D)*S
 call fTrunc
 ld (iy+RotM21+0),a
 ld (iy+RotM21+1),l
 ld (iy+RotM21+2),h ; M20 = (F+D)*S
 
 pop de \ pop hl \ pop af ; x^2, {z^2, y^2, z^2+y^2}
 exx \ ex af,af' ; acc' = x^2
 pop de \ pop hl \ pop af ; AHLDE = z^2+y^2, {z^2, y^2}
 call fTrunc
 ex de,hl \ ld c,a ; CDE = z^2+y^2
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (zz+yy)*S
 xor $80 ; -(zz+yy)*S
 exx \ ex af,af' ; acc' = -(zz+yy)*S, AHLDE = x^2
 push af \ push hl \ push de ; x^2, {z^2, y^2, x^2}
 ld a,$3F \ ld hl,$8000 \ ld d,l \ ld e,l ; AHLDE = 1.0
 call fAddAcc ; 1 - (zz+yy)*S
 call fTrunc
 ld (iy+RotM00+0),a
 ld (iy+RotM00+1),l
 ld (iy+RotM00+2),h ; M00 = 1 - (zz+yy)*S
 
 pop de \ pop hl \ pop af ; x^2, {z^2, y^2}
 exx \ ex af,af' ; acc' = x^2
 pop de \ pop hl \ pop af ; y^2, {z^2}
 exx \ ex af,af' ; acc' = y^2, ahlde = x^2
 push af \ push hl \ push de ; x^2, {z^2, x^2}
 call fAddAcc ; x^2+y^2
 call fTrunc
 ex de,hl \ ld c,a ; CDE = x^2+y^2
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (xx+yy)*S
 xor $80 ; -(xx+yy)*S
 exx \ ex af,af' ; acc' = -(xx+yy)*S
 ld a,$3F \ ld hl,$8000 \ ld d,l \ ld e,l ; AHLDE = 1.0
 call fAddAcc ; 1 - (xx+yy)*S
 call fTrunc
 ld (iy+RotM22+0),a
 ld (iy+RotM22+1),l
 ld (iy+RotM22+2),h ; M22 = 1 - (xx+yy)*S
 
 pop de \ pop hl \ pop af ; x^2, {z^2}
 exx \ ex af,af' ; acc' = x^2
 pop de \ pop hl \ pop af ; z^2, {}
 call fAddAcc ; x^2+z^2
 call fTrunc
 ex de,hl \ ld c,a ; CDE = x^2+z^2
 ld a,(QtR_S) \ ld hl,(QtR_S+1) ; AHL = S
 call fMult ; (xx+zz)*S
 xor $80 ; -(xx+zz)*S
 exx \ ex af,af' ; acc' = -(xx+zz)*S
 ld a,$3F \ ld hl,$8000 \ ld d,l \ ld e,l ; AHLDE = 1.0
 call fAddAcc ; 1 - (xx+zz)*S
 call fTrunc
 ld (iy+RotM11+0),a
 ld (iy+RotM11+1),l
 ld (iy+RotM11+2),h ; M11 = 1 - (xx+zz)*S
; ret
 jr QuattoRotMExit

RotMIdentity:
 ld hl,3*4*2 ; un-push 12 times
 add hl,sp
 ld sp,hl
 ld hl,RotM
 call MLoadIdentity

QuattoRotMExit:
; End Quat to RotM
