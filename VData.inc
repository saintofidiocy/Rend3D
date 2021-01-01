; Vertex Data Program Header


.nolist
#define  ProgStart   $9D95
#define  ProtProgObj 6

; Rend3D.z80 exports for VObj file linking
#include "Rend3D.exp"

.org ProgStart-2
.db $BB,$6D
.list
  jr _main

; Float helper -- use with .db
#define float(f)  f&$FF0000>>16,f&$FF,f&$FF00>>8


#define VCount (TList-VList)/9 ; auto-count
#define TCount (NList-TList)/3 ; auto-count

CameraPos:
#ifdef CAM_X
  .db float(CAM_X)
#else
  .db $00,$00,$00 ; X
#endif
#ifdef CAM_Y
  .db float(CAM_Y)
#else
  .db $00,$00,$00 ; Y
#endif
#ifdef CAM_Z
  .db float(CAM_Z)
#else
  .db $42,$00,$C0 ; Z = 12
#endif
#ifdef CAM_W
  .db float(CAM_W)
#else
  .db $00,$00,$00 ; W
#endif

VertexCount:
  .db VCount ; auto-count
TriangleCount:
  .db TCount ; auto-count

VListPtr:
  .dw VList-ProgStart+4
TListPtr:
  .dw TList-ProgStart+4
NListPtr:
  .dw NList-ProgStart+4
VListCalcPtr:
  .dw VListCalc-ProgStart+4
TSortPtr:
  .dw TSort-ProgStart+4

_main:
  call getVData
  ld de,Rend3D.VDatName
  add hl,de
  ex de,hl ; de = Rend3D.VDatName
  ld hl,SelfName
  ld bc,9
  ldir  ; Rend3D.VDatName = SelfName
  ld hl,textDone
  jr vDataExit

#include "findsym.inc"

textDone:
  .db "Vertex Data Set",0

VDatName:
  .db ProtProgObj,"REND3D",0,0,0

