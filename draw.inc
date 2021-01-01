; Draw & Graphical Routines


; Clears the draw buffer
ClearBuffer:
  ;ld a,$00 ; clear
  ld a,$FF ; fill
  ld bc,$0003 ; $300 big-endian
  ld hl,GBuffB
ClearBufferLoop:
  ld (hl),a
  inc hl
  djnz ClearBufferLoop
  dec c
  jr nz,ClearBufferLoop
  ret


; Fills draw buffer with 50% halftone texture
FillBuffer50:
  ld a,10101010b
  ld bc,$0C40 ; b=12, c=64
  ld hl,GBuffB
FillBuffer50Loop:
  ld (hl),a
  inc hl
  djnz FillBuffer50Loop
  cpl ; ~a
  ld b,12
  dec c
  jr nz,FillBuffer50Loop
  ret


; Copies draw buffer to LCD
DrawBuffer:
  ld hl,GBuffB
  ld a,$80
  call LCD_Busy_Wait
  out (LCDinstPort),a ; set to row 0
  ld c,$20 ; column 0
DrawBufX:
  ld a,c
  cp $2c
  ret z ; done
  call LCD_Busy_Wait
  out (LCDinstPort),a ; set column
  ld b,64
  ld de,12 ; buffer width
DrawBufY:
  ld a,(hl)
  call LCD_Busy_Wait
  out (LCDdataPort),a
  add hl,de ; 1 row down in buffer
  djnz DrawBufY
  ld a,$80
  call LCD_Busy_Wait
  out (LCDinstPort),a ; set to row 0
  ld de, -(12*64)+1 ; row 0 of next column in buffer
  add hl,de
  inc c ; next column
  jr DrawBufX
  


; DrawTri(TListPtr)
; HL = pointer to triangle vertices
; DE = pointer to shading texture
; adapted from http://forum.devmaster.net/t/advanced-rasterization/6145
DrawTri:
  push de ; ShadePtr, S = {ShadePtr}
  ld de,TriA ; X1
  ld b,3 ; repeat for X, Y, Z
DrawTriLoad:
  ld a,(hl) \ inc hl
  push hl \ push bc \ push de ; S = {ShadePtr, TListPtr+1, i, &X1}
  ld e,a \ ld d,0 ; de = a
  ld l,a \ ld h,d ; hl = a
  add hl,hl \ add hl,hl \ add hl,hl
  add hl,de ; hl = a * 9
  ex de,hl
  ld hl,(VertexListCalc)
  add hl,de ; hl = &VListCalc[a].x
  ld a,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) \ inc hl
  ex de,hl ; AHL = VListCalc[a].x
  add a,2 ; AHL *= 4
  call ftos16 ; HL = int(AHL) -- preserve DE
  ex de,hl ; de = val, hl = &VListCalc[a].y
  ex (sp),hl ; hl = X1, S = {ShadePtr, TListPtr+1, i, &VLC.y}
  
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; X1 = val
  ex (sp),hl ; hl = VLC.y, S = {ShadePtr, TListPtr+1, i, &Y1}
  ld a,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) ;\ inc hl
  ex de,hl ; AHL = VListCalc[a].y
  add a,2 ; AHL *= 4
  call ftos16 ; HL = int(AHL)
  ex de,hl ; de = val
  pop hl ; hl = Y1, S = {ShadePtr, TListPtr+1, i}
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; Y1 = val
  ex de,hl ; de = Y1
  
  pop bc \ pop hl ; hl = TLPtr, S = {ShadePtr}
  djnz DrawTriLoad
  
  ex (sp),hl ; hl = ShadePtr, (sp) = TListPtr+1
  push hl ; (ShadePtr)
  
  ; calc DX's
  ld hl,(TriB) ; X2
  ex de,hl
  ld hl,(TriA) ; hl = X1, de = X2
  push hl \ push de
  call NegDE \ add hl,de
  ld (DX12),hl ; DX12 = X1-X2
  ld hl,(TriC) \ ex de,hl ; de = X3
  pop hl ; hl = X2
  push de ; X3
  call NegDE \ add hl,de
  ld (DX23),hl ; DX23 = X2-X3
  pop hl \ pop de ; hl = X3 ; de = X1
  call NegDE \ add hl,de
  ld (DX31),hl ; DX31 = X3-X1
  
  ; calc DY's
  ld hl,(TriB+2) ; Y2
  ex de,hl
  ld hl,(TriA+2) ; hl = Y1, de = Y2
  push hl \ push de
  call NegDE \ add hl,de
  ld (DY12),hl ; DY12 = Y1-Y2
  ld hl,(TriC+2) \ ex de,hl ; de = Y3
  pop hl ; hl = Y2
  push de ; Y3
  call NegDE \ add hl,de
  ld (DY23),hl ; DY23 = Y2-Y3
  pop hl \ pop de ; hl = Y3 ; de = X1
  call NegDE \ add hl,de
  ld (DY31),hl ; DY31 = Y3-Y1
  
  ; Calc MinX/MaxX
  ld bc,(TriA) ; X1
  ld de,(TriB) ; X2
  ld hl,(TriC) ; X3
  call MinMax ; HL < DE < BC
  
  ; Rounding & Bounds checking on MinX
;  ld de,$0002
;  add hl,de ; round MinX
  bit 7,h ; MinX < 0 ?
  jr nz,MinXDone ; allow negative
  
  or a ; clear carry
  ld a,h \ rr a ; carry = bit 0,h
  jp nz,DrawTriExit ; HL >= 128.0 -- Triangle beyond screen
  ld a,l \ rra ; a = HL >> 1
  cp 96*2 ; HL >= 96.0 ?
  jp nc,DrawTriExit ; Triangle beyond screen
  
MinXDone:
  ld a,l \ and $FC \ ld l,a ; round
  ld (minX),hl ; MinX = round(minX)

  ; Rounding & Bounds checking on MaxX
  ld h,b \ ld l,c ; hl = BC
  inc hl \ inc hl \ inc hl ; round MaxX up -- smaller and faster than ld de,3 \ add hl,de
  bit 7,h ; MaxX < 0?
  jp nz,DrawTriExit ; Triangle beyond screen
  
  srl h ; carry = bit 0,h
  jr nz,MaxXOF ; HL > 128.0
  ld a,l \ rra \ srl a ; a = round(maxX / 4)
  cp 96 ; max > 96 ?
  jr c,MaxXDone
MaxXOF:
  ld a,$60 ; MaxX = 96
MaxXDone:
  dec a
  ld (maxX),a ; MaxX = max(maxX / 4, 96)
  
  
  ; Calc MinY/MaxY
  ld bc,(TriA+2) ; Y1
  ld de,(TriB+2) ; Y2
  ld hl,(TriC+2) ; Y3
  call MinMax ; HL < DE < BC
  
  ; Rounding & Bounds checking on MinY
;  ld de,$0002
;  add hl,de ; round MinY\
  bit 7,h ; MinY < 0 ?
  jr nz,MinYDone ; allow negative minY
  
  or a ; clear carry
  ld a,h \ rr a ; carry = bit 0,h
  jp nz,DrawTriExit ; HL >= 128.0 -- Triangle beyond screen
  ld a,l \ rra ; a = HL >> 1
  cp 64*2 ; HL >= 64.0 ?
  jp nc,DrawTriExit ; HL >= 64.0 -- Triangle beyond screen
  
MinYDone:
  ld a,l \ and $FC \ ld l,a ; round(minY)
  ld (minY),hl ; MinY = round(minY)

  ; Rounding & Bounds checking on MaxY
  ld h,b \ ld l,c ; hl = BC
  inc hl \ inc hl \ inc hl ; round MaxY up -- smaller and faster than ld de,3 \ add hl,de
  bit 7,h ; MaxY < 0?
  jp nz,DrawTriExit ; Triangle beyond screen
  
  srl h ; carry = bit 0,h
  jr nz,MaxYOF ; HL > 64.0
  ld a,l \ rra \ srl a ; a = round(max / 4)
  cp 64 ; max > 64 ?
  jr c,MaxYDone
MaxYOF:
  ld a,64 ; MaxY = 64
MaxYDone:
  dec a
  ld (maxY),a ; MaxY = Max(max / 4, 64)
  
  
  ld ix,DX12
  ld iy,DY12
  ld hl,TriA ; X1
  ld a,3 ; TriA,TriB,TriC
DrawTriSub:
  ex af,af' ; Save counter
  ;{DrawTriSub}
  push hl ; &X
  ld c,(hl) \ inc hl
  ld b,(hl) \ inc hl ; BC = X
  ld e,(hl) \ inc hl
  ld d,(hl) ; DE = Y1
  ex de,hl ; HL = Y
  ld e,(iy+0) \ ld d,(iy+1) ; DE = DY
  push de ; Stack={&X,DY}
  push hl ; Stack={&X,DY,Y}
  ld h,b \ ld l,c ; HL = X
  call sMultFixed ; DE = X * DY
  ld l,(ix+0) \ ld h,(ix+1) ; HL = DX
  ld b,h \ ld c,l ; BC = DX
  ex (sp),hl ; HL = Y, (sp) = DX
  push de ; Stack={&X,DY,DX,X*DY}
  ld d,b \ ld e,c ; HL = DX
  call sMultFixed ; DE = Y*DX
  ex de,hl ; HL = Y*DX
  call NegHL ; HL = -Y*DX
  pop de ; X*DY, Stack={&X,DY,DX}
  add hl,de ; X*DY-Y*DX
  pop bc ; DX
  pop de ; DY, Stack={&X}
  bit 7,d ; DY < 0?
  jr nz,DrawTriSubInc ; DY < 0
  ld a,d \ or e ; DY == 0 ?
  jr nz,DrawTriSubNoInc ; DY != 0
  or b ; DX > 0 ?
  jp m,DrawTriSubNoInc ; DX < 0
  or c ; DX != 0 ?
  jr z,DrawTriSubNoInc ; DX == 0
DrawTriSubInc:
  inc hl ; X++
DrawTriSubNoInc:
  push hl \ push bc ; Stack={&X,X',DX}
  call NegDE
  push de ; Stack={&X,X',DX,-DY}
  ld hl,(minX) ; minX << 2
  call sMultFixed ; DE = -DY * minX
  ex de,hl ; HL = -DY * minX
  ex (sp),hl ; hl = -DY, (sp) = -DY*minX
  ld (IY+0),l \ ld (IY+1),h ; DY = -(DY<<2)
  pop hl ; -DY*minX
  pop de ; DX
  push hl \ push de ; Stack={&X,X',-DY*minX,DX}
  ld hl,(minY) ; minY << 2
  call sMultFixed ; DE = DX*minY
  ex de,hl ; HL = DX*minY
  ex (sp),hl ; hl = DX, (sp) = DX*minY
  ld (IX+0),l \ ld (IX+1),h ; DX <<= 2
  pop hl ; DX*minY
  pop de ; -DY*minX
  pop bc ; X'
  add hl,de \ add hl,bc ; Y' = X' + DX*minY - DY*minX
  ex de,hl ; DE = Y'
  pop hl ; &X
  ld (hl),c \ inc hl
  ld (hl),b \ inc hl ; &X = X'
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; &Y = Y'
  ;{/DrawTriSub}
  inc ix \ inc ix
  inc iy \ inc iy
  ex af,af' ; Restore
  dec a;
  jp nz,DrawTriSub
  
;DrawInit:
  exx \ ex af,af' ; Load Pixel Regs
  ; HL' = PixBuffer ; current pixel(s)
  ; DE' = LineBuffer ; start of current line
  ; B' = BitPos
  ; A' = BitMask
  ; C' = BitPosInit
 
  ; minY >>= 2
;  ld de,$0002
  ld hl,(minY)
;  add hl,de ; round
  ld a,l
  rr h \ rra
  rr h \ rra
  ld (minY),a
  
  or a ; minY < 0 ?
  jp p,DrawTriPosMinY
  xor a ; buffer calcs start at 0
DrawTriPosMinY:
  ld b,a ; minY
  
;   minX >>= 2
  ld hl,(minX)
;  add hl,de ; round
  ld a,l
  rr h \ rra
  rr h \ rra
  ld (minX),a
  
  or a ; minX < 0 ?
  jp p,DrawTriPosMinX
  xor a ; buffer calcs start at 0
DrawTriPosMinX:
  
  
  ld h,0
  ld l,b ; HL = minY
  ld d,h
  ld e,l ; DE = minY
  add hl,hl \ add hl,de ; HL = minY*3
  add hl,hl \ add hl,hl ; HL = minY*12
  ld c,a ; c = minX
  rra \ rra \ rra ; a = minX >> 3
  and $1F ; a = (minX >> 3) & a
  ld e,a ; DE = minX
  add hl,de ; HL = minY*12+(minX/8)
  ld de,GBuffB ; DE = GraphBuff
  add hl,de ; HL = PixBuffer = GBuff + minY*12+(minX/8)
  ld d,h \ ld e,l ; LineBuffer = PixBuffer
  
  ld a,c ; a = minX
  and 7 ; minX & 7 - pixel block alignment
  ld c,a \ ld a,8 \ sub c ; 8-(minX&7)
  ld c,a ; BitPosInit = 8-(minX&7)
;LineInit:
  ld b,c ; BitPos = BitPosInit
  ld a,$FF ; BitMask = $FF ; all pixels cleared
  exx \ ex af,af' ; Restore Regs
  
  
  pop hl \ push hl ; hl = ShadePtr
  ld a,(minY) \ and 3 ; minY % 4
  add a,l \ ld l,a ; l += minY % 4
  ld h,(hl) ; Shade = *(ShadePtr + minY%4)
  push hl ; Shade
  
  
;DrawTri Cont
  ld a,(minY) \ ld c,a ; c = MinY
  or a ; y < 0 ?
  jp m,DrawTriYSkipNeg ; skip lines until y >= 0
  
  ld hl,(TriA+2) \ ld (TriA),hl ; X1 = Y1
  ld hl,(TriB+2) \ ld (TriB),hl ; X2 = Y2
  ld hl,(TriC+2) \ ld (TriC),hl ; X3 = Y3
DrawTriYLoop:
  ld a,(minX) \ ld b,a ; b = MinX
  or a ; x < 0 ?
  jp m,DrawTriXSkipNeg ; skip pixels until x >= 0
  
DrawTriXLoop:
  ;if(X3 > 0 && ...
  ld hl,(TriC) ; hl = X3
  ld a,h \ or a ; h < 0?
  jp m,DrawTriXCont ; X3 < 0
  or l ; h | l
  jr z,DrawTriXCont ; X3 == 0
  
  ;... X2 > 0 && ...
  ld hl,(TriB) ; HL = X2
  ld a,h \ or a ; h < 0?
  jp m,DrawTriXCont ; X2 < 0
  or l ; h | l
  jr z,DrawTriXCont ; X2 == 0
  
  ;... X1 > 0)
  ld hl,(TriA) ; HL = X1
  ld a,h \ or a ; h < 0?
  jp m,DrawTriXCont ; X1 < 0
  or l ; h | l
  jr z,DrawTriXCont ; X1 == 0
  
;{SetPixel}
  exx \ ex af,af' ; Load Pixel Regs.
  or a ; mask bit = 0
  jr DrawTriPutPixel


DrawTriXCont:
;{SkipPixel}
  exx \ ex af,af' ; Load Pixel Regs.
  scf ; mask bit = 1
DrawTriPutPixel:
  rla ; push mask bit on to mask
  djnz PixelDone ; BitPos--, Bits remaining?
;--Put Pixel Data
  ld b,a ; b = Mask
  and (hl) \ ld (hl),a ; *PixBuffer &= BitMask
  ld a,b \ cpl ; a = ~BitMask
  
  ; a & Shading
  ex (sp),hl ; hl = Shade
  and h ; a = ~BitMask & Shade
  ex (sp),hl ; hl = PixBuffer
  
  or (hl) \ ld (hl),a ; *PixBuffer |= (BitMask & Shading)
;--End Put Pixel Data
  inc hl ; PixBuffer++
  ld a,$FF ; BitMask = $FF ; all pixels cleared
  ld b,8 ; BitPos = 8 ; whole block
PixelDone:
  exx \ ex af,af' ; Restore regs.
  
;DrawTriCont:
DrawTriXSkipNeg:
  ld hl,(TriA) ; X1
  ld de,(DY12) ; -DY12
  add hl,de
  ld (TriA),hl ; X1 -= DY12
  
  ld hl,(TriB) ; X2
  ld de,(DY23) ; -DY23
  add hl,de
  ld (TriB),hl ; X2 -= DY23
  
  ld hl,(TriC) ; X3
  ld de,(DY31) ; -DY31
  add hl,de
  ld (TriC),hl ; X3 -= DY31
  
  ;Count down?: djnz DTXLoop
  inc b ; X++
  jp m,DrawTriXSkipNeg ; skip pixels until x >= 0
  
  ld a,(maxX)
  cp b ; X < MaxX?
  jr nc,DrawTriXLoop ; X != MaxX
  
;{LineFinish}
  exx \ ex af,af' ; Load Pixel Regs.
  bit 3,b ; b == 8?
  jr nz,DrawTriNoPad
DrawTriPadLoop:
  scf ; mask bit = 1
  rla ; push mask bit
  djnz DrawTriPadLoop
;--Put Pixel Data
  ld b,a ; b = Mask
  and (hl) \ ld (hl),a ; *PixBuffer &= BitMask
  ld a,b \ cpl ; a = ~BitMask
  
  ; a & Shading
  ex (sp),hl ; hl = Shade
  and h ; a = ~BitMask & Shade
  ex (sp),hl ; hl = PixBuffer
  
  or (hl) \ ld (hl),a ; *PixBuffer |= (BitMask & Shading)
;--End Put Pixel Data
DrawTriNoPad:
  ld hl,12 ; Buffer Stride
  add hl,de ; LineBuffer += Stride (y++)
  ld d,h \ ld e,l ; PixelBuffer = LineBuffer
  ld b,c ; BitPos = BitPosInit
  ld a,$FF ; BitMask = $FF
  exx \ ex af,af' ; Restore Regs.
  
DrawTriYSkipNeg:
  ld hl,(TriA+2) ; Y1
  ld de,(DX12) \ add hl,de
  ld (TriA+2),hl ; Y1 += DX12 
  ld (TriA),hl ; X1 = Y1
  
  ld hl,(TriB+2) ; Y2
  ld de,(DX23) \ add hl,de
  ld (TriB+2),hl ; Y2 += DX23
  ld (TriB),hl ; X2 = Y2
  
  ld hl,(TriC+2) ; Y3
  ld de,(DX31) \ add hl,de
  ld (TriC+2),hl ; Y3 += DX31
  ld (TriC),hl ; X3 = Y3
  
  inc c ; Y++
  jp m,DrawTriYSkipNeg ; skip rows until y >= 0
  
  ld a,c \ and 3 ; a = y % 4
  pop hl ; throw away Shade
  pop hl \ push hl ; ShadePtr
  add a,l \ ld l,a ; ShadePtr += y%4
  ld h,(hl) ; Shade = *ShadePtr
  push hl ; Shade
  
  ld a,(maxY)
  cp c ; Y < MaxY?
  jp nc,DrawTriYLoop ; Y != MaxY
  pop hl ; throw away shade
DrawTriExit:
  pop hl ; ShadePtr
  pop hl ; TListPtr+1
  ret


; hl = min(hl, de, bc); bc = max(hl, de, bc)
; out: hl <= de <= bc (signed)
MinMax:
  ; cmp bc,de
  ld a,b \ cp d
  jr z,CheckBCDElow ; b == d
  jp m,CheckBCDEsign
  jp pe,SwapBCDE ; sign != overflow --> BC < DE
  jr Check2 ; sign == overflow --> BC >= DE
  
CheckBCDEsign:
  jp po,SwapBCDE ; sign != overflow --> BC < DE
  jr Check2 ; sign == overflow --> BC >= DE
  
CheckBCDElow:
  ld a,c \ cp e
  jr nc,Check2 ; BC >= DE
SwapBCDE:
  ld a,b \ ld b,d \ ld d,a ; swap b d
  ld a,c \ ld c,e \ ld e,a ; swap c e
  
Check2: ; cmp bc,hl
  ld a,b \ cp h
  jr z,CheckBCHLlow ; b == h
  jp m,CheckBCHLsign
  jp pe,SwapBCHL ; sign != overflow --> BC < HL
  jr Check3 ; sign == overflow --> BC >= HL
  
CheckBCHLsign:
  jp po,SwapBCHL ; sign != overflow --> BC < HL
  jr Check3 ; sign == overflow --> BC >= HL
  
CheckBCHLlow:
  ld a,c \ cp l
  jr nc,Check3 ; BC >= HL
SwapBCHL:
  ld a,b \ ld b,h \ ld h,a ; swap b h
  ld a,c \ ld c,l \ ld l,a ; swap c l
  
Check3: ; cmp de,hl
  ld a,d \ cp h
  jr z,CheckDEHLlow
  jp m,CheckDEHLsign
  ret po ; sign == overflow --> DE >= HL
  ex de,hl ; SwapDEHL
  ret

CheckDEHLsign:
  ret pe ; sign == overflow --> DE >= HL
  ex de,hl ; swapDEHL
  ret

CheckDEHLlow:
  ld a,e \ cp l
  ret nc
  ex de,hl ; swapDEHL
  ret


; Old "MinMax" function -- needed to be signed
; sorts hl, de, bc
; out: hl <= de <= bc (unsigned)
;MinMax:
;  ld a,b \ cp d
;  jr c,SwapBCDE ; BC < DE
;  jr nz,Check2 ; BC > DE
;  ld a,c \ cp e
;  jr nc,Check2 ; BC >= DE
;SwapBCDE:
;  ld a,b \ ld b,d \ ld d,a ; swap b d
;  ld a,c \ ld c,e \ ld e,a ; swap c e
;Check2:
;  ld a,b \ cp h
;  jr c,SwapBCDE ; BC < HL
;  jr nz,Check3 ; BC > HL
;  ld a,c \ cp l
;  jr nc,Check3 ; BC >= HL
;SwapBCHL:
;  ld a,b \ ld b,h \ ld h,a ; swap b h
;  ld a,c \ ld c,l \ ld l,a ; swap c l
;Check3:
;  ld a,d \ cp h
;  jr c,SwapDEHL ; DE < HL
;  ret nz ; BC > HL
;  ld a,d \ cp l
;  ret nc ; BC >= HL
;SwapDEHL:
;  ex de,hl
;  ret
