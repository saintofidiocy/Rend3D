; Matrix & Vector Routines

; Default MDim = 3
#define MDim 3
; SetMDim(a)
; Changes global matrix/vector size for MultMV and MIdentity
; Destroys: none
;SetMDim:
;  ld (SetMDim1),a
;  ld (SetMDim2),a
  ;ld (SetMDim3),a
  ;ld (SetMDim4),a
  ; more?
;  ret
 

; MultMM(matrix *Ma, matrix *Mb, matrix *Mout)
; (HL) = (IX) * (IY)
; Multiplies matrix pointed to by IX by matrix pointed to by IY and stores in to matrix pointed by HL
; Destroys:
MultMM:
  ; todo?
  ret


; MultMV(matrix *M, vector *V, vector *Vout)
; (HL) = (IX) * (IY)
; Multiplies matrix pointed to by IX by vector pointed to by IY and stores in vector pointed to by HL
; Destroys: AF', BC', DE', HL', AF, BC ($00), DE, IX (&M[4][0])
; 
MultMV:
  push hl ; Vout
  ld c,MDim ; i = 4
SetMDim1 .equ $-1

MMV_iLoop:
  push IY ; V[0]
  push hl ; Vout[i]
  xor a
  ld h,a \ ld l,a \ ld d,a \ ld e,a ; AHLDE = 0
  ld b,MDim ; k = 4
SetMDim2 .equ $-1

MMV_kLoop:
  push bc ; ik
  exx \ ex af,af' ; AHLDE' = acc
  ld a,(IX+0)
  ld l,(IX+1)
  ld h,(IX+2) ; AHL = M[i*dim+k]
  ld c,(IY+0)
  ld e,(IY+1)
  ld d,(IY+2) ; CDE = V[k]
  
  exx ; switch to unused bc
  ld bc,3
  add ix,bc
  add iy,bc
  exx ; switch to proper bc
  
  call fMult ; M[i*dim+k] * V[k]
  call fAddAcc ; acc += M[i*dim+k] * V[k]
  pop bc ; ik
  djnz MMV_kLoop
  call fTrunc
  ex de,hl
  pop hl ; Vout[i]
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  pop IY ; V[0]
  dec c
  jr nz,MMV_iLoop
  pop hl ; Vout
  ret


; MLoadIdentity(matrix *M)
; (HL) = identity matrix
; Loads the matrix pointed to by HL with identity
; Destroys: A ($00),BC ($0001),DE ($3F80)
; TStates: 1238 / 35 Bytes
MLoadIdentity:
  push hl ; push M
  ld de,$3F80
  ld c,MDim
SetMDim3 .equ $-1
MLI_cLoop:
  ld b,MDim
SetMDim4 .equ $-1
MLI_bLoop:
  ld a,b
  xor c ; if(b == c)?
  jr z,MLI_LDOne ; yes
  ; Load 0
  xor a
  ld (hl),a \ inc hl
  ld (hl),a \ inc hl
  ld (hl),a \ inc hl
  djnz MLI_bLoop
  dec c
  jp MLI_cLoop ; Always nz -- jp +1 byte, -2*3 TStates
MLI_LDOne:
  ld (hl),d \ inc hl
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  djnz MLI_bLoop
; dec c
; jr nz,MLI_cLoop ; This will never happen
  pop hl ; pop M
  ret



; Randomize rotation matrix
RotXYZ:
  call fRand ; X
  push AF \ push HL
  call Cos ; cosX
  ld (tmpA),a
  ld (tmpA+1),hl
  pop HL \ pop AF
  call Sin ; sinX
  ld (tmpD),a
  ld (tmpD+1),hl
  
  call fRand ; Y
  push AF \ push HL
  call Cos ; cosY
  ld (tmpB),a
  ld (tmpB+1),hl
  pop HL \ pop AF
  call Sin ; sinY
  ld (tmpE),a
  ld (tmpE+1),hl
  
  call fRand ; Z
  push AF \ push HL
  call Cos ; cosZ
  ld (tmpC),a
  ld (tmpC+1),hl
  pop HL \ pop AF
  call Sin ; sinZ
  ld (tmpF),a
  ld (tmpF+1),hl
  
  ld hl,RotM ; M00
  push hl
  
  ld a,(tmpC)
  ld hl,(tmpC+1)
  ex de,hl \ ld c,a ; CDE = C
  ld a,(tmpB)
  ld hl,(tmpB+1) ; AHL = B
  call fMult ; BC
  call fTrunc
  ex de,hl \ pop hl ; ADE, hl = M00
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl
  
  ld a,(tmpF)
  ld hl,(tmpF+1)
  ex de,hl \ ld c,a ; CDE = F
  ld a,(tmpA);
  ld hl,(tmpA+1) ; AHL = A
  call fMult ; AF
  pop bc
  push AF \ push HL \ push DE ; AHLDE, AF
  push bc
  exx \ ex af,af' ; acc' = AF
  ld a,(tmpC)
  ld hl,(tmpC+1)
  ex de,hl \ ld c,a ; CDE = C
  ld a,(tmpD)
  ld hl,(tmpD+1) ; AHL = D
  call fMult ; DC
  pop bc
  push AF \ push HL \ push DE ; AHLDE, CD
  push bc
  call fTrunc
  ex de,hl \ ld c,a ; CDE = CD
  ld a,(tmpE)
  ld hl,(tmpE+1) ; AHL = E
  call fMult ; CDE
  call fAddAcc ; AF+CDE
  call fTrunc
  ex de,hl \ pop hl ; m01
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl
  
  ld a,(tmpF)
  ld hl,(tmpF+1)
  ex de,hl \ ld c,a ; CDE = F
  ld a,(tmpD);
  ld hl,(tmpD+1) ; AHL = D
  call fMult ; DF
  pop bc
  push AF \ push HL \ push DE ; AHLDE, DF
  push bc
  exx \ ex af,af' ; acc' = DF
  ld a,(tmpC)
  ld hl,(tmpC+1)
  ex de,hl \ ld c,a ; CDE = C
  ld a,(tmpA)
  ld hl,(tmpA+1) ; AHL = A
  call fMult ; AC
  pop bc
  push AF \ push HL \ push DE ; AHLDE, AC
  push bc
  call fTrunc
  ex de,hl \ ld c,a ; CDE = AC
  ld a,(tmpE)
  ld hl,(tmpE+1) ; AHL = E
  xor $80 ; -E
  call fMult ; -ACE
  call fAddAcc ; DF-ACE
  call fTrunc
  ex de,hl \ pop hl ; m02
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl
  
  ld a,(tmpF)
  ld hl,(tmpF+1)
  ex de,hl \ ld c,a ; CDE = F
  ld a,(tmpB)
  ld hl,(tmpB+1) ; AHL = B
  call fMult ; BF
  call fTrunc
  xor $80 ; -BF
  ex de,hl \ pop hl ; m10
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  ld b,h \ ld c,l ; push hl \ pop bc
  
  pop de \ pop hl \ pop af ; AHLDE = AC
  push bc ; m11
  exx \ ex af,af' ; acc' = AC
  pop bc ; m11
  pop de \ pop hl \ pop af ; AHLDE = DF
  push bc ; m11
  call fTrunc
  ex de,hl \ ld c,a ; CDE = DF
  ld a,(tmpE)
  ld hl,(tmpE+1) ; AHL = E
  xor $80 ; -E
  call fMult ; -DFE
  call fAddAcc ; AC-DFE
  call fTrunc
  ex de,hl \ pop hl ; m11
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  ld b,h \ ld c,l ; push hl \ pop bc
  
  pop de \ pop hl \ pop af ; AHLDE = DC
  push bc ; m12
  exx \ ex af,af' ; acc' = DC
  pop bc
  pop de \ pop hl \ pop af ; AHLDE = AF
  push bc ; m12
  call fTrunc
  ex de,hl \ ld c,a ; CDE = AF
  ld a,(tmpE)
  ld hl,(tmpE+1) ; AHL = E
  call fMult ; AFE
  call fAddAcc ; DC+AFE
  call fTrunc
  ex de,hl \ pop hl
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  ;ex de,hl ; push hl
  
  ld de,(tmpE+1)
  ;ex de,hl ; m20
  inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl
  
  ld a,(tmpB)
  ld hl,(tmpB+1)
  ex de,hl \ ld c,a ; CDE = B
  ld a,(tmpD)
  ld hl,(tmpD+1) ; AHL = D
  call fMult ; DB
  call fTrunc
  xor $80 ; -DB
  ex de,hl \ pop hl ; m21
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  push hl
  
  ld a,(tmpB)
  ld hl,(tmpB+1)
  ex de,hl \ ld c,a ; CDE = B
  ld a,(tmpA)
  ld hl,(tmpA+1)
  call fMult ; AB
  call fTrunc
  ex de,hl \ pop hl ; m22
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d
  ret



; Vout = Va - Vb
; Va = IX
; Vb = IY
; Vout = HL
VSub:
  ;push hl ; Vout
  ld b,MDim
VSubLoop:
  push bc ; i
  push hl ; Vout[i]
  ld a,(iy+0) \ xor $80
  ld l,(iy+1)
  ld h,(iy+2)
  ld c,(ix+0)
  ld e,(ix+1)
  ld d,(ix+2)
  call fAdd ; Va[i] - Vb[i]
  ex de,hl
  pop hl ; Vout[i]
  ld (hl),a \ inc hl
  ld (hl),e \ inc hl
  ld (hl),d \ inc hl
  ld de,3
  add ix,de ; ix += 3
  add iy,de ; iy += 3
  pop bc ; i
  djnz VSubLoop
  ;pop hl ; Vout
  ret
