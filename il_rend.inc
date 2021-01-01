; Inline; isn't a called function

; Macro to adjust matrix dimension so a duplicate function doesn't need to exist -- won't work in a flash program
#define SetMDim(x)	ld a,x \ ld (SetMDim1),a \ ld (SetMDim2),a

; calc vertex data
VertexCalc:
  ld a,(VertexCount)
  ld b,a
  ld iy,(VertexList)
  ld hl,(VertexListCalc)
VertexLoop:
  push bc ; VertexCount
  push hl ; VLCalc[v]
  ld ix,RotM
  ld hl,VTmp1
  call MultMV ; Vtmp1 = RotM*VList[v]
  ld de,9 \ add iy,de ; VList[v++]
  push iy ; VList
  ld ix,VTmp1
  ld iy,CPos
  ld hl,VTmp2
  call VSub ; Vtmp2 = Vtmp1 - CPos
  SetMDim(4) ; MDim = 4
  
  ; VTmp2.W = 1.0
  ld a,$3F \ ld hl,$8000
  ld (VTmp2+VW),a \ ld (VTmp2+VW+1),hl
  
  ld ix,ProjectionM
  ld iy,VTmp2
  ld hl,VTmp1
  call MultMV ; Vtmp1 = ProjM*Vtmp2
  SetMDim(3) ; MDim = 3
  
  pop iy ; VList
  pop hl ; hl = VLCalc[V]
  
  ld a,(VTmp1+VZ)
  ld c,a
  and $7F ; exp(VTmp1.W)
  jr z,VertexWZero ; VTmp1.W == 0
  ld a,c ; (VTmp1+VW)
  
  exx ; hl' = VLCalc[V]
  ld c,a ; (VTmp1+VW)
  
  ld a,(VTmp1+VZ)
  ld hl,(VTmp1+VZ+1)
  push af \ push hl ; Z
  
  ;ld a,c ; (VTmp1+VW)
  ld de,(VTmp1+VZ+1) ; CDE = VTmp1.W
  push bc \ push de ; W
  
  ld a,(VTmp1+VX)
  ld hl,(VTmp1+VX+1) ; AHL = VTmp1.X
  call fDiv ; AHL = VTmp1.X / VTmp1.W
  ld c,$44 \ ld de,$C000 ; 96.0
  call fMult \ call fTrunc ; AHL = VTmp1.X / VTmp1.W * 96.0
  ld c,$44 \ ld de,$C000 ; 48.0
  call fAdd
  
  pop de \ pop bc ; CDE = W
  push HL ; X/W
  ex af,af' ; a' = exp(X/W)
  
  ld a,(VTmp1+VY)
  ld hl,(VTmp1+VY+1) ; AHL = VTmp1.Y
  call fDiv ; AHL = VTmp1.Y / VTmp1.W
  ld c,$44 \ ld de,$8000 ; 64.0
  call fMult \ call fTrunc ; AHL = VTmp1.Y / VTmp1.W * 64.0
  ld c,$44 \ ld de,$8000 ; 32.0
  call fAdd
  
  
  ex (sp),hl ; hl = X/W, (sp) = Y/W
  push hl ; X/W
  exx ; hl = VLCalc[V]
  pop de ; ADE = X/W
  ex af,af' ; a = exp(X/W), a' = exp(Y/W)
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; VLCalc[V].X = X/W
  ex af,af' ; a = exp(Y/W), a' = exp(X/W)
  pop de ; ADE = Y/W
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; VLCalc[V].Y = Y/W
  pop de \ pop af ; Z
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; VLCalc[V].Z = Z
  pop bc ; b = VertexCount
  ;djnz VertexLoop ; Range of relative branch exceeded. :(
  dec b \ jp nz,VertexLoop
  jr VertexExit

VertexWZero:
  ld b,6 ; sizeof({X,Y})
VertexWZeroLoop: ; a == 0
  ld (hl),a \ inc hl
  djnz VertexWZeroLoop
  pop bc ; b = VertexCount
  ;djnz VertexLoop ; Range of relative branch exceeded.
  dec b \ jp nz,VertexLoop
VertexExit:



;Populate Sort List
  ld hl, (TriangleSort)
  ex de,hl ; de = SPtr
  ld hl, (TriangleList) ; hl = TPtr
  ld b,0 ; i = 0
  push de ; SPtr, S={SPtr}
ListLoop:
  push bc ; i, S={SPtr, i}
  ld a,$FF \ ld d,a \ ld e,a ; ADE = -inf
  ld b,3 ; j = 3
ListMinLoop:
  push bc ; j, S={SPtr, i, j}
  ld c,(hl) ; c = *TPtr
  inc hl ; TPtr++
  push hl ; TPtr, S={SPtr, i, j, TPtr}
  
  ; Get &VLC.Z
  ld b,0 \ ld h,b \ ld l,c ; hl = index, bc = index
  add hl,hl \ add hl,hl \ add hl,hl \ add hl,bc ; hl *= 9
  ld c,6 \ add hl,bc ; hl += 6
  ld b,h \ ld c,l ; bc = hl
  ld hl,(VertexListCalc)
  add hl,bc ; hl = &VLCalc[index].Z
  ; Got &VLC.Z
  
  push de ; Z bits, S={SPtr, i, j, TPtr, Z}
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) ;\ inc hl ; CDE = A
  pop hl ; AHL = Z, S={SPtr, i, j, TPtr}
  call fCmp ; Z-A
  jr nc,ListMinNoSwap ; Z >= A
  ex de,hl \ ld a,c ; Z = A
ListMinNoSwap:
  ex de,hl ; ADE = Z
  pop hl ; TPtr
  pop bc ; j, S={SPtr, i}
  djnz ListMinLoop
  pop bc ; i, S={SPtr}
  ex (sp),hl ; hl = SPtr, S={TPtr}
  ld (hl),b \ inc hl ; SPtr->index = i
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; SPtr->Z = Z; SPtr++
  ex (sp),hl ; hl=TPtr, S={SPtr}
  ld a,(TriangleCount)
  inc b ; i++
  cp b ; a-b
  jr nz,ListLoop ; i < TriangleCount
  pop hl ; throw away, S={}
  
;Sort List
  push af ; TriangleCount, S={TriangleCount}
  ld hl,(TriangleSort) ; SPtr
  ld b,a ; i = TriangleCount
SortLoop:
  push bc ; i, S={TCt, i}
  dec b ; j = i-1
  jr z,SortExit
  push hl ; SPtrCur
  push hl ; SPtrSwap, S={TCt, i, SPtrCur, SPtrSwap}
  inc hl
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) \ inc hl ; Z = SPtr->Z
  ; SPtr2 = ++SPtr
SortSubLoop:
  push bc ; j
  push hl ; SPtr2
  push de ; Z, S={TCt, i, SPtrCur, SPtrSwap, j, SPtr2, Z}
  inc hl
  ld a,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) \ inc hl ; A = SPtr2->Z
  ; SPtr2Next = SPtr2 + 1
  ex (sp),hl ; hl = Z, S={TCt, i, SPtrCur, SPtrSwap, j, SPtr2, SPtr2Next}
  ex de,hl ; AHL = A, CDE = Z
  call fCmp ; A-Z
  jr c,SortNewSwap ; A < Z
  ; Don't update
  pop hl ; SPtr2Next
  pop af ; throw away
  pop bc ; j, S={TCt, i, SPtrCur, SPtrSwap}
  djnz SortSubLoop
  jr SortSubFinish

SortNewSwap:
  ex de,hl \ ld c,a ; CDE = A
  pop hl ; SPtr2Next, S={TCt, i, SPtrCur, SPtrSwap, j, SPtr2}
  exx ; shadow to work
  pop hl ; SPtr2
  pop af ; a = j, S={TCt, i, SPtrCur, SPtrSwap}
  ex (sp),hl ; hl = SPtrSwap, S={TCt, i, SPtrCur, SPtr2}
  exx ; restore
  ld b,a ; b = j
  djnz SortSubLoop
SortSubFinish:
  pop hl ; SPtrSwap
  pop de ; SPtrCur, S={TCt, i}
  push de ; SPtrCur
  push hl ; SPtrSwap
  push de ; SPtrCur, S={TCt, i, SPtrCur, SPtrSwap, SPtrCur}
  ld b,(hl) \ inc hl
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) ;\ inc hl ; BCDE = *SPtrSwap
  exx
  pop hl ; SPtrCur, S={TCt, i, SPtrCur, SPtrSwap}
  ld b,(hl) \ inc hl
  ld c,(hl) \ inc hl
  ld e,(hl) \ inc hl
  ld d,(hl) ;\ inc hl ; BCDE = *SPtrCur
  pop hl ; SPtrSwap, S={TCt, i, SPtrCur}
  ld (hl),b \ inc hl
  ld (hl),c \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d ;\ inc hl ; *SPtrSwap = BCDE
  exx
  pop hl ; SPtrCur, S={TCt, i}
  ld (hl),b \ inc hl
  ld (hl),c \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl ; *SPtrCur = BCDE; SPtr = SPtrCur+1
  pop bc ; b = i, S={TriangleCount}
  djnz SortLoop
  push bc ; safety if, somehow, it gets to this point
SortExit:
  pop bc ; throw away i, S = {TriangleCount}



;Render Triangles
  ld hl,(TriangleSort) ; SPtr
  pop bc ; b = TriangleCount, S={}
RenderLoop:
  push bc ; i, S={i}
  ld c,(hl) ; e = SPtr->Index
  ld de,4 ; de = $0004
  add hl,de ; SPtr++
  push hl ; SPtrNext, S={i, SPtr}
  ld e,c ; de = index
  ld h,d \ ld l,e ; hl = de
  add hl,hl \ add hl,de ; hl = index * 3
  push hl ; TListOffset, S={i, SPtr, TListOffs}
  ld d,h \ ld e,l ; de = hl
  add hl,hl \ add hl,de ; hl = index * 9
  ex de,hl ; de = NListOffset
  ld iy,(NormalList)
  add iy,de ; NList[index]
  ld ix,RotM
  ld hl,VTmp1
  call MultMV ; VTmp1 = RotM * NList[index]
  ld a,(VTmp1 + VZ) ; exp(VTmp1.Z)
  or a
  jr z,RenderSkip ; VTmp1.Z == 0
  jp m,RenderSkip ; VTmp1.Z < 0
  ld hl,(VTmp1 + VZ + 1)
  ld c,$C3 \ ld de,$8800 ; CDE = -17, # Shades
  call fMult ; VTmp1.Z*-17
  call ftos16 ; S = (int)(VTmp1.Z*-17)
  ld de,17 \ add hl,de ; S = 17 - VTmp1.Z*17
  add hl,hl \ add hl,hl ; S *= 4
  ld de,ShadeTable
  add hl,de ; ShadeTable + S
  ex de,hl ; de = ShadePtr
  ld hl,(TriangleList)
  pop bc ; TListOffs, S={i, SPtr}
  add hl,bc ; TList[index]
  call DrawTri
  push hl ; smaller and faster than a jr $+1
RenderSkip:
  pop hl ; throw away TListOffs
  pop hl ; SPtr
  pop bc ; b = i, S={}
  djnz RenderLoop
  
; Finish

 