; Loads the data pointer to a different variable or program in RAM

.nolist
ChkFindSym    .equ $42F1
PutS          .equ $450A
NewLine       .equ $452E
#define bcall(x)  rst 28h \ .dw x
.list

getVData:
  ld hl,VDatName
  ld a,(hl)
  or a
  jr z,vDataNotSpecified
  rst 20h ; rMov9ToOp1
  bcall(ChkFindSym)
  jr c,vDataNotFound
  ld a,b
  or a
  jr nz,vDataArchived
  ex de,hl
  ret

vDataNotSpecified:
  ld hl,vDatErr1
  jr vDataError
vDataNotFound:
  ld hl,vDatErr2
  jr vDataError
vDataArchived:
  ld hl,vDatErr3
vDataError:
  pop de ; return past previous call to exit the program
vDataExit:
  bcall(PutS)
  bcall(NewLine)
  ret

vDatErr1:
  .db "Err: None Loaded",0
vDatErr2:
  .db "Err: Not found",0
vDatErr3:
  .db "Err: Archived",0
