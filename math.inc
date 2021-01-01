; General Math Functions

; I don't remember the exact sources for the original log, exp, sin, cos and acos algorithms, apart from being available somewhere on the web
; If you know of the sources, let me know so that I may give credit


; returns 1/sqrt(x)
; Inputs:
;  AHL = x
; adapted from: https://en.wikipedia.org/wiki/Fast_inverse_square_root
InvSqrt:
  ld c,a
  call fDiv2CDE ;dec c ; CHL / 2
  push hl ; CHL / 2
  ld de,$6EB5
  rl h \ srl a \ rr h ; (HL << 1), AHL >> 1
  cpl ; -a
  ld b,a \ call NegHL ; -HL, preserve a
  ld a,h \ or l ; hl == 0 ? -- carried
  jr nz,InvSqrtNC
  inc b ; add carry
InvSqrtNC:
  ld a,b ; restore A
  add hl,de \ adc a,$5E ; $5E6EB5 - *(int*)AHL
  scf \ rr h \ rr l ; y = *(float*)i
  pop de \ push hl \ push af ; pop x/2, push y
  push de \ push bc ; push x/2
  call fSquare ; y*y
  pop bc \ pop de ; x/2
  call fMult ; x/2*y*y
  xor $80 ; -(x/2*y*y)
  ld c,$3F \ ld de,$C000 ; CDE = 1.5
  call fAdd ; 1.5 - (x/2*y*y)
  pop bc \ pop de \ ld c,b ; y
  call fMult ; y * (1.5 - (x/2*y*y))
  ret


; returns sqrt(1+x)
; Inputs:
;  AHL = x
Sqrt1plusX:
  ld c,$3F \ ld de,$8000 ; CDE = 1.0
  call fAdd ; x + 1
  ;jr Sqrt
  ; fall through to sqrt

; returns sqrt(x)
; Inputs:
;  AHL = x
Sqrt:
  push af \ push hl ; x
  call InvSqrt
  pop de \ pop bc \ ld c,b
  call fMult ; AHL^(-1/2+1)
  ret



; returns ln(x)
; Inputs:
;  AHL = x
Log:
 ;call log2
 ;ld c,$3E \ ld de,$B172 ; CDE = ln(2)
 ;call fMult ; log2(x) * ln(2)
 ;ret

Log2:
  ld b,a
  and $7F \ sub 64
  ld c,a ; log_2
  push bc ; log_2
  
  ld a,b
  and $80 \ or $3F ; exp(val) = 0
  push af \ push hl ; val
  ld c,$C0 \ ld de,$C000 ; CDE = -3
  call fDiv ; val / -3
  ld c,$40 \ ld de,$8000 ; CDE = 2
  call fAdd ; (val / -3) + 2
  pop de \ pop bc \ ld c,b ; CDE = val
  call fMult ; ((val / -3) + 2) * val
  call fTrunc
  ld c,$BE \ ld de,$AAAB ; CDE = -2/3
  call fAdd ; ((val / -3) + 2) * val - 2/3
  pop bc ; c = log_2
  push af \ push hl ; val
  ld a,c
  call s8tof ; AHL = A
  pop de \ pop bc \ ld c,b ; CDE = val
  call fAdd ; val + float(log_2)
  ;ret

;LogCont:
  ld c,$3E \ ld de,$B172 ; CDE = ln(2)
  call fMult ; log2(x) * ln(2)
  ret



; returns e^(x)
; Inputs:
;  AHL = x
Exp:
  ld c,$3F \ ld de,$B8AA ; CDE = 1.442...
  call fMult ; x*1.442...
  ;jr pow2
Pow2: ; 2^x; AHL = x
  ld c,a
  and $80 ; sgn(x)
  ld b,a ; offset
  ex af,af' \ ld a,b \ ex af,af' ; a' = offset
;x < -126 ? -126 : x
  jr z,Pow2NoClip ; x > 0
  ld a,c \ xor b ; exp(x)
  cp $45 ; exp - exp(-126)
  jr c,Pow2NoClip ; exp(x) < exp(-126)
  jr nz,Pow2Clip ; exp(x) > exp(-126)
  ld a,h
  cp $FC
  jr nc,Pow2NoClip ; x >= -126
Pow2Clip:
  ld c,$C5 ; c = exp(-126)
  ld hl,$FC00 ; AHL = -126
Pow2NoClip:
  ld a,c ; restore exp(AHL)
  push af \ push hl ; P
  call fInt ; w = int(P), no rounding
  
  pop de \ pop bc ; P
  push bc \ push de ; P
  ;ld c,b ; CDE = P
  ld c,a
  ld a,b
  ex de,hl ; CDE = W, AHL = P
  ;pop hl \ pop af ; P
  ;push af \ push hl ; P
  call fSub ; P-W
  
  ex af,af' ; a = offset
  or a
  jr z,Pow2NoOffs ; Offset == 0
  ex af,af' ; restore AHL
  ld c,$3F \ ld de,$8000 ; CDE = 1.0
  call fAdd ; P - W + offset
  jr Pow2NoOffs+1
Pow2NoOffs:
  ex af,af' ; restore AHL
;Pow2NoOffs+1:
  push af \ push hl ; Z
  ld c,$3F \ ld de,$BEBD ; CDE = 1.49
  call fMult ; Z*1.49
  call fTrunc
  pop de \ pop bc \ ld c,b ; CDE = Z
  push af \ push hl ; z*1.49
  ld a,$41 \ ld hl,$9AF6 ; AHL = 4.842
  call fSub ; 4.842-Z
  ex de,hl \ ld c,a ; CDE = 4.842-Z
  ld a,$43 \ ld hl,$DDD3 ; AHL = 27.73
  call fDiv ; 27.73/(4.8-z)
  pop de \ pop bc \ ld c,b ; CDE = z*1.49
  call fSub ; 27.73/(4.8-z) - z*1.49
  ld c,$45 \ ld de,$F28C ; CDE = 121.27
  call fAdd ; 121.27 + 27.73/(4.8-z) - z*1.49
  pop de \ pop bc \ ld c,b ; CDE = P
  call fAdd ; p + 121.27 + 27.73/(4.8-z) - z*1.49
  add a,15 ; AHL *= 2^15
  call ftou24
  sla h ; carry = bit7
  adc a,a ; a = (a<<1) + carry
  sub 64 ; exp(AHL) - 127 + 63
  scf ; carry = 1
  rr h ; bit7 = 1
  ret


; -pi to pi
Sin:
  ld c,$3E \ ld de,$A2FA ; CDE = 2/pi
  call fMult ; AHL * 2/pi
CosFinish:
  push af \ push hl ; AHL * 2/pi
  or $80 ; -|x*2/pi|
  ld c,$40 \ ld de,$8000 ; CDE = 2
  call fAdd ; 2-|x*2/pi|
  pop de \ pop bc \ ld c,b ; CDE = x*2/pi
  call fMult ; (2-|x*2/pi|)(x*2/pi)
  ld c,a \ ld d,h \ ld e,l ; CDE = AHL ; y
  push bc \ push de ; y
  push bc \ push de ; y
  and $7F ; |y|
  call fMult ; y*|y|
  pop de \ pop bc ; y
  call fSub ; y*|y|-y
  ld c,$3C \ ld de,$E666 ; CDE = .225 ; p
  call fMult ; .225 * (y*|y|-y)
  pop de \ pop bc ; y
  call fAdd ; .225 * (y*|y|-y) + y
  ret

Cos:
  ld c,$3E \ ld de,$A2FA ; CDE = 2/pi
  call fMult ; AHL * 2/pi
  
  or a
  jp m,CosAdd+1 ; AHL < 0 < 1.00
  cp $3F ; a-$3F
  jr c,CosAdd+1 ; AHL < 1.00
  jr nz,CosSub ; AHL > 1.00
  ld b,a
  xor a
  cp h ; $00-h
  jp m,CosAdd ; AHL < 1.00
  jr nz,CosSub ; AHL > 1.00
  cp l ; $00-l
  jr nz,CosSub ; AHL > 1.00
 
 
  ;ld b,a
  ;ld a,$3F
  ;cp b ; $3F-b
  ;jr c,CosSub ; b > $3F
  ;xor a
  ;cp h ; $00-h
  ;jp p,CosSub ; h > $80
  ;cp l ; $00-l
  ;jr nz,CosSub ; l > $00
 
CosAdd:
  ld a,b
;CosAdd+1:
  ld c,$3F \ ld de,$8000 ; CDE = 1
  
  call fAdd
  jr CosFinish

CosSub:
  ld c,$C0 \ ld de,$C000 ; CDE -3
  ld a,b
  call fAdd
  jr CosFinish



; returns arccos(AHL)
ACos:
  ld c,a
  and $80 ; sgn(AHL)
  ld b,a ; b = negate
  xor c ; AHL = |AHL|
  push bc ; neg
  ld c,a
  push bc \ push hl ; X
  push bc \ push hl ; X
  push bc \ push hl ; X
  ld c,$B9 \ ld de,$996E ; CDE = -.018
  call fMult ; |X| * -.018
  ld c,$3B \ ld de,$9816 ; CDE = .074
  call fAdd ; |X| * -.018 + .074
  pop de \ pop bc ; BCE = X
  call fMult
  ld c,$BC \ ld de,$D935
  call fAdd
  pop de \ pop bc ; BCE = X
  call fMult
  ld c,$3F \ ld de,$C90E
  call fAdd
  pop de \ pop bc ; CDE = X
  push af \ push hl ; ret
  ld a,$3F \ ld hl,$8000
  call fSub ; 1-X
  call Sqrt ; sqrt(1-X)
  pop de \ pop bc \ ld c,b ; CDE = ret
  call fMult ; ret * sqrt(1-X)
  pop bc ; b = sgn
  ld c,a ; store
  ld a,b ; a = sgn
  or a ; test a
  ld a,c ; restore
  ret z ; sgn == 0
  ex de,hl ; CDE = ret
  ld a,$40 \ ld hl,$C910 ; AHL = pi
  call fSub ; pi - ret
  ret



; Negate DE, only destroying A
NegDE:
  ld a,d \ cpl \ ld d,a
  ld a,e \ cpl \ ld e,a
  inc de
  ret


; Negate HL, only destroying A
NegHL:
  ld a,h \ cpl \ ld h,a
  ld a,l \ cpl \ ld l,a
  inc hl
  ret

