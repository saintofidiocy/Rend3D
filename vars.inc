; Defines Global Variables


; Float helper -- use with .db
#define float(f)  f&$FF0000>>16,f&$FF,f&$FF00>>8


; exit value used if key detection is in interrupt handler
;Exit:
;  .db 00

; Maximum number of main loop iterations, just so it doesn't go forever
RotCount:
  .db 03


; Move these variables to saveSScreen/plotSScreen?

ProjectionM:
  .db float($3FCE0A),float($000000),float($000000),float($000000)
  .db float($000000),float($409A84),float($000000),float($000000)
  .db float($000000),float($000000),float($BF8006),float($BFCCD2)
  .db float($000000),float($000000),float($BF8000),float($000000)
  ;.db $3F,$0A,$CE, $00,$00,$00, $00,$00,$00, $00,$00,$00
  ;.db $00,$00,$00, $40,$84,$9A, $00,$00,$00, $00,$00,$00
  ;.db $00,$00,$00, $00,$00,$00, $BF,$06,$80, $BF,$D2,$CC
  ;.db $00,$00,$00, $00,$00,$00, $BF,$00,$80, $00,$00,$00

; S = 1/tan(33deg / 2)
; A = 96/64
; N = .1
; F = 1000
; [S/A, 0, 0          , 0        ]
; [0  , S, 0          , 0        ]
; [0  , 0, (F+N)/(N-F), 2FN/(N-F)]
; [0  , 0, -1         , 0        ]
;  .db $40,$0A,$90, $00,$00,$00, $00,$00,$00, $00,$00,$00
;  .db $00,$00,$00, $40,$10,$D8, $00,$00,$00, $00,$00,$00
;  .db $00,$00,$00, $00,$00,$00, $BF,$07,$80, $BC,$D2,$CC
;  .db $00,$00,$00, $00,$00,$00, $BF,$00,$80, $00,$00,$00


RotM:
  .db float($3F8000),float($000000),float($000000)
  .db float($000000),float($3F8000),float($000000)
  .db float($000000),float($000000),float($3F8000)

; Matrix element offsets
RotM00 .equ 0
RotM01 .equ 3
RotM02 .equ 6
RotM10 .equ 9
RotM11 .equ 12
RotM12 .equ 15
RotM20 .equ 18
RotM21 .equ 21
RotM22 .equ 24


Q0:
  .db float($3F8000)
  .db float($000000)
  .db float($000000)
  .db float($000000)

; Quaternion element offsets
Q_A .equ 0
Q_B .equ 3
Q_C .equ 6
Q_D .equ 9

Q1:
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

QA:
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

QB:
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

QC:
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

VTmp1: ; Same as QA?
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

VTmp2: ; Same as QB?
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00
  .db $00,$00,$00

; Vector element offsets
VX .equ 0
VY .equ 3
VZ .equ 6
VW .equ 9

TriA: .dw $0000,$0000
TriB: .dw $0000,$0000
TriC: .dw $0000,$0000
DX12: .dw $0000
DX23: .dw $0000
DX31: .dw $0000
DY12: .dw $0000
DY23: .dw $0000
DY31: .dw $0000
minX: .dw $0000
maxX: .dw $0000
minY: .dw $0000
maxY: .dw $0000

tmpA: .db $00,$00,$00
tmpB: .db $00,$00,$00
tmpC: .db $00,$00,$00
tmpD: .db $00,$00,$00
tmpE  .equ RotM+RotM20
tmpF: .db $00,$00,$00

T:
  .db $00,$00,$00
QtR_S:
  .db $00,$00,$00

CPos:
CPosX: .db $00,$00,$00
CPosY: .db $00,$00,$00
CPosZ: .db $00,$00,$00
CPosW: .db $00,$00,$00

VertexCount:
  .db 0
TriangleCount:
  .db 0

VertexList:
  .dw $0000 ; none loaded
TriangleList:
  .dw $0000 ; none loaded
NormalList:
  .dw $0000 ; none loaded
VertexListCalc:
  .dw $0000 ; none loaded
TriangleSort:
  .dw $0000 ; none loaded

  align(4) ; align to 4 bytes
ShadeTable:
  .db $00,$00,$00,$00 ; 0
  .db $80,$00,$08,$00 ; 1
  .db $92,$00,$08,$00 ; 2
  .db $92,$00,$29,$00 ; 3
  .db $92,$04,$69,$00 ; 4
  .db $92,$24,$69,$02 ; 5
  .db $D6,$24,$69,$02 ; 6
  .db $D6,$A4,$69,$82 ; 7
  .db $D6,$AC,$69,$92 ; 8
  .db $D6,$AE,$6D,$92 ; 9
  .db $D6,$AE,$6D,$DA ; 10
  .db $D6,$EE,$7D,$DA ; 11
  .db $D7,$EF,$7D,$DA ; 12
  .db $FF,$EF,$7D,$DA ; 13
  .db $FF,$EF,$FD,$FA ; 14
  .db $FF,$EF,$FF,$FE ; 15
  .db $FF,$FF,$FF,$FF ; 16

; old ugly shading
;  .db $00,$00,$00,$00 ; 0
;  .db $11,$00,$00,$00 ; 1
;  .db $11,$00,$44,$00 ; 2
;  .db $55,$00,$44,$00 ; 3
;  .db $55,$00,$55,$00 ; 4
;  .db $55,$22,$55,$00 ; 5
;  .db $55,$22,$55,$88 ; 6
;  .db $55,$AA,$55,$88 ; 7
;  .db $55,$AA,$55,$AA ; 8
;  .db $77,$AA,$55,$AA ; 9
;  .db $77,$AA,$DD,$AA ; 10
;  .db $FF,$AA,$DD,$AA ; 11
;  .db $FF,$AA,$FF,$AA ; 12
;  .db $FF,$BB,$FF,$AA ; 13
;  .db $FF,$BB,$FF,$EE ; 14
;  .db $FF,$FF,$FF,$EE ; 15
;  .db $FF,$FF,$FF,$FF ; 16
