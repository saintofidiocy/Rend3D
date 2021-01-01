; Vertex Data Program Footer

; Buffers for translated vertexes and depth sort
VListCalc:
  .fill VCount*3*3 ; same size as VList
TSort:
  .fill TCount*4 ; {char Index, float Z}


; Throws an error if program is too big
#include "fsizechk.inc"
