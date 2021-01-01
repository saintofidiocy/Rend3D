; Inline; isn't a called function

; QC = Q0 * (Q0^-1 * Q1)^t
Slerp:
  ld hl,Q0
  ld de,QA
  call QInverse ; QA = QInv(Q0)
  
  ld ix,QA
  ld iy,Q1
  call QMult ; QC = QA*Q1
  
  ld iy,QC
  ld a,(iy+Q_B)
  ld l,(iy+Q_B+1)
  ld h,(iy+Q_B+2)
  call fSquare ; QCB*QCB
  exx \ ex af,af' ; acc = QCB*QCB
  ld a,(iy+Q_C)
  ld l,(iy+Q_C+1)
  ld h,(iy+Q_C+2)
  call fSquare ; QCC*QCC
  call fAddAcc
  exx \ ex af,af' ; acc = QCB*QCB + QCC*QCC
  ld a,(iy+Q_D)
  ld l,(iy+Q_D+1)
  ld h,(iy+Q_D+2)
  call fSquare ; QCD*QCD
  call fAddAcc ; acc = QCB*QCB + QCC*QCC + QCD*QCD
  push af \ push hl ; push AHL == S
  exx \ ex af,af' ; acc
  ld a,(iy+Q_A)
  ld l,(iy+Q_A+1)
  ld h,(iy+Q_A+2)
  call fSquare ; QCA*QCA
  call fAddAcc ; acc = QCB*QCB + QCC*QCC + QCD*QCD + QCA*QCA
  
  ;<<Optimise>>
  call Sqrt ; AHL = N
  exx \ ex af,af' ; acc' = N
  pop hl \ pop af ; AHL = S
  or a ; a == 0 ?
  jp z,SlerpA ; Range of relative branch exceeded :(
  call InvSqrt ; S = 1/sqrt(S)
  exx \ ex af,af' ; acc' = S; AHL = N
  push af \ push hl ; push N
  exx \ ex af,af' ; acc' = N; AHL = S
  push af \ push hl ; push S
  exx \ ex af,af' ; acc' = S; AHL = N
  ex de,hl \ ld c,a ; CDE = N
  ;<</Optimise>>
  
  ld a,(iy+Q_A)
  ld l,(iy+Q_A+1)
  ld h,(iy+Q_A+2)
  call fDiv ; AHL = QCA / N
  ld ix,QA ; restore ix after division
  call ACos ; cos-1(QCA/N)
  push hl ; preserve
  ld hl,T
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl)          ; CDE = T
  pop hl ; restore
  call fMult ; AHL = cos-1(QCA/N)*t
  pop de \ pop bc \ ld c,b ; CDE = S
  call fMult ; AHL = cos-1(QCA/n)*t*s
  call fTrunc
  ex de,hl \ ld c,a ; CDE = trunc(AHLDE) -> B
  push bc \ push de ; B
  push bc \ push de ; B
  
  ld a,(iy+Q_B)
  ld l,(iy+Q_B+1)
  ld h,(iy+Q_B+2)
  call fMult ; QCB * B
  call fTrunc ; AHL = trunc(AHLDE)
  ld (ix+Q_B+0),a
  ld (ix+Q_B+1),l
  ld (ix+Q_B+2),h ; QAB = QRB*B
  
  pop de \ pop bc ; CDE = B
  ld a,(iy+Q_C)
  ld l,(iy+Q_C+1)
  ld h,(iy+Q_C+2)
  call fMult ; QCC * B
  call fTrunc ; AHL = trunc(AHLDE)
  ld (ix+Q_C+0),a
  ld (ix+Q_C+1),l
  ld (ix+Q_C+2),h ; QAC = QRC*B
  
  pop de \ pop bc ; CDE = B
  ld a,(iy+Q_D)
  ld l,(iy+Q_D+1)
  ld h,(iy+Q_D+2)
  call fMult ; QCD * B
  call fTrunc ; AHL = trunc(AHLDE)
  ld (ix+Q_D+0),a
  ld (ix+Q_D+1),l
  ld (ix+Q_D+2),h ; QAD = QRD*B
  
  pop HL \ pop AF ; N
  jr SlerpB

SlerpA:
  xor a
  ld b,9
SlerpALoop:
  ld hl,QA+Q_B
  ld (hl),a \ inc hl
  djnz SlerpALoop
  exx \ ex af,af' ; AHL = N
  
SlerpB:
  call Log ; AHL = ln(N)
  ld ix,QA ; restore ix after Log
  push hl ; preserve
  ld hl,T
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl)          ; CDE = t
  pop hl ; restore
  call fMult ; AHL = ln(N) * t
  ld (ix+Q_A+0),a
  ld (ix+Q_A+1),l
  ld (ix+Q_A+2),h ; QAA = ln(N)*t
  call Exp ; exp(QAA)
  ld ix,QA ; restore ix after Exp
  push AF \ push HL ; exp(QAA)
  
  ld a,(ix+Q_B)
  ld l,(ix+Q_B+1)
  ld h,(ix+Q_B+2) ; AHL = QAB
  call fSquare ; AHL = QAB*QAB
  exx \ ex af,af' ; acc = QAB*QAB
  ld a,(ix+Q_C)
  ld l,(ix+Q_C+1)
  ld h,(ix+Q_C+2) ; AHL = QAC
  call fSquare ; AHL = QAC*QAC
  call fAddAcc
  exx \ ex af,af' ; acc = QAB*QAB + QAC*QAC
  ld a,(ix+Q_D)
  ld l,(ix+Q_D+1)
  ld h,(ix+Q_D+2) ; AHL = QAD
  call fSquare ; AHL = QAD*QAD
  call fAddAcc ; acc = QAB*QAB + QAC*QAC + QAD*QAD
  call Sqrt ; V
  or a ; V == 0?
  jp z,SlerpC ; V == 0
  
  push af \ push hl ; V
  push af \ push hl ; V
  call Sin ; sin(V)
  pop de \ pop bc \ ld c,b ; CDE = V
  call fDiv ; AHL = sin(V)/V
  ld ix,QA ; restore ix after division
  exx \ ex af,af' ; AHL' = sin(V)/V
  pop hl \ pop af ; AHL = V
  call Cos ; AHL = cos(V)
  pop de \ pop bc \ ld c,b ; CDE = exp(QAA)
  push bc \ push de ; exp(QAA)
  call fMult ; AHL = cos(V) * exp(QAA)
  ld iy,QB
  ld (iy+Q_A+0),a
  ld (iy+Q_A+1),l
  ld (iy+Q_A+2),h ; QBA = cos(V) * exp(QAA)
  
  exx \ ex af,af' ; AHL = sin(V)/V
  pop de \ pop bc \ ld c,b ; CDE = exp(QAA)
  call fMult ; AHL = sin(V)/V * exp(QAA) ; B
  ex de,hl \ ld c,a ; CDE = B
  push bc \ push de ; B
  push bc \ push de ; B
  
  ld a,(ix+Q_B)
  ld l,(ix+Q_B+1)
  ld h,(ix+Q_B+2) ; AHL = QAB
  call fMult ; AHL = QAB * B
  ld (iy+Q_B+0),a
  ld (iy+Q_B+1),l
  ld (iy+Q_B+2),h ; QBB = QAB * sin(V)/V * exp(QAA)
  
  ld a,(ix+Q_C)
  ld l,(ix+Q_C+1)
  ld h,(ix+Q_C+2) ; AHL = QAC
  pop de \ pop bc ; B
  call fMult ; AHL = QAC * B
  ld (iy+Q_C+0),a
  ld (iy+Q_C+1),l
  ld (iy+Q_C+2),h ; QBC = QAC * sin(V)/V * exp(QAA)
  
  ld a,(ix+Q_D)
  ld l,(ix+Q_D+1)
  ld h,(ix+Q_D+2) ; AHL = QAD
  pop de \ pop bc ; B
  call fMult ; AHL = QAD * B
  ld (iy+Q_D+0),a
  ld (iy+Q_D+1),l
  ld (iy+Q_D+2),h ; QBD = QAD * sin(V)/V * exp(QAA)
  jr SlerpD

SlerpC: ; QB = {1.0, 0, 0, 0}
  pop hl \ pop hl ; clear stack
  ld hl,QB
  ld (hl),$3F
  inc hl
  xor a
  ld (hl),a
  inc hl
  ld (hl),$80 ; QB_A = $3F,$8000
  ld b,9
SlerpCLoop:
  inc hl
  ld (hl),a
  djnz SlerpCLoop
  
SlerpD:
  ld ix,Q0
  ld iy,QB
  call QMult ; QC = Q0*QB
  
; End Slerp
Slerp_End:
