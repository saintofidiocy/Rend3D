// Quick & dirty program to convert *.stl model to VData code for this program
// When exporting rotate the model to face Z+ and invert across the Y axis to match the coordinate space
// No other data should be exported, as there is no support for materials, textures, lighting, etc.

#include <stdio.h>
#include <stdlib.h>

typedef   signed char  s8;
typedef unsigned char  u8;
typedef   signed short s16;
typedef unsigned short u16;
typedef   signed long  s32;
typedef unsigned long  u32;
typedef   signed long long s64;
typedef unsigned long long u64;
typedef enum { false, true } bool;


typedef struct {
  float normal[3];
  float v[3][3];
  u16 dataCount;
} __attribute__((packed)) stl_triangle;

typedef struct {
  float normal[3];
  u32 v[3];
} triangle;

typedef struct {
  float v[3];
  u32 left,right;
} vTree;

struct {
  u8 header[4];
  u8 cPos[12];
  u8 vCount;
  u8 tCount;
  u16 VListPtr,TListPtr,NListPtr,VCalcPtr,TSortPtr; // ftell+2
  u8 data[134];
  u8 name[9];
} __attribute__((packed)) bindata = { // compiled VData code
  {0xBB,0x6D,0x18,0x18},
  {0x00,0x00,0x00, 0x00,0x00,0x00, 0x42,0x00,0x80, 0x00,0x00,0x00},
  0,0,
  0,0,0,0,0,
  {0xCD,0xC4,0x9D,0x11,0x20,0x00,0x19,0xEB,0x21,0x34,0x9E,0x01,0x09,0x00,0xED,0xB0,
   0x21,0x1A,0x9E,0x18,0x21,0x21,0x2A,0x9E,0x7E,0xB7,0x28,0x0C,0xE7,0xEF,0xF1,0x42,
   0x38,0x0B,0x78,0xB7,0x20,0x0C,0xEB,0xC9,0x21,0xEC,0x9D,0x18,0x08,0x21,0xFD,0x9D,
   0x18,0x03,0x21,0x0C,0x9E,0xD1,0xEF,0x0A,0x45,0xEF,0x2E,0x45,0xC9,0x45,0x72,0x72,
   0x3A,0x20,0x4E,0x6F,0x6E,0x65,0x20,0x4C,0x6F,0x61,0x64,0x65,0x64,0x00,0x45,0x72,
   0x72,0x3A,0x20,0x4E,0x6F,0x74,0x20,0x66,0x6F,0x75,0x6E,0x64,0x00,0x45,0x72,0x72,
   0x3A,0x20,0x41,0x72,0x63,0x68,0x69,0x76,0x65,0x64,0x00,0x56,0x65,0x72,0x74,0x65,
   0x78,0x20,0x44,0x61,0x74,0x61,0x20,0x53,0x65,0x74,0x00,0x06,0x52,0x45,0x4E,0x44,
   0x33,0x44,0x00,0x00,0x00,0x06},
  0
};



u32 vTreeAdd(vTree* tree, u32* treeCount, float* v);
u32 floatConv(float f);


int main(int argc, char* argv[]){
  u8 name[260];
  u8 outname[260];
  
  printf("size == %d\n", sizeof(stl_triangle));
  if(argc < 2){
    puts("No file specified.");
    system("pause");
    return 0;
  }
  
  u32 triCount = 0;
  triangle* tris = NULL;
  vTree* tree = NULL;
  u32 treeCount = 0;
  stl_triangle loadTri;
  u32 i,j,out,exp;
  
  // Get filename
  
  puts(argv[1]);
  for(i = strlen(argv[1])-1; i > 0 && argv[1][i-1] != '\\'; i--);
  for(j = i; argv[1][j] != 0 && argv[1][j] != '.'; j++) name[j-i] = argv[1][j];
  name[j-i] = 0;
  puts(name);
  
  if(!(name[0] >= 'A' && name[0] <= 'Z') && !(name[0] >= 'a' && name[0] <= 'z')){
    sprintf(outname, "V%s", name);
    strcpy(name, outname);
  }
  for(i = 0; i < 8 && name[i] != 0; i++){
    bindata.name[i] = toupper(name[i]);
  }
  name[i] = 0;
  
  
  // Load STL
  
  FILE* f = fopen(argv[1], "rb");
  if(f == NULL){
    puts("Could not open file.");
    system("pause");
    return 0;
  }
  fseek(f, 80, SEEK_SET);
  fread(&triCount, 1, sizeof(u32), f);
  
  tris = malloc(triCount * sizeof(triangle));
  tree = malloc(triCount * 3 * sizeof(vTree));
  
  for(i = 0; i < triCount; i++){
    fread(&loadTri, 1, sizeof(stl_triangle), f);
    if(loadTri.dataCount > 0){
      printf("dataCount at %08X = %d\n", 84+triCount*sizeof(stl_triangle), loadTri.dataCount);
      fseek(f, loadTri.dataCount, SEEK_CUR);
    }
    for(j = 0; j < 3; j++){
      tris[i].normal[j] = -loadTri.normal[j];
      tris[i].v[j] = vTreeAdd(tree, &treeCount, loadTri.v[j]);
    }
  }
  fclose(f);
  
  
  // Save source file
  
  sprintf(outname, "%s.z80", name);
  f = fopen(outname, "w");
  fprintf(f, "#define CAM_X $000000\n#define CAM_Y $000000\n#define CAM_Z $428000\n#include \"VData.inc\"\nSelfName:\n .db ProtProgObj,\"%s\"", bindata.name);
  for(i = strlen(bindata.name); i < 9; i++){
    fputs(",0", f);
  }
  fputs("\n\nVList:\n", f);
  for(i = 0; i < treeCount; i++){
    out = 
    fprintf(f, " .db float($%06X),float($%06X),float($%06X)\n", floatConv(tree[i].v[0]), floatConv(tree[i].v[1]), floatConv(tree[i].v[2]));
  }
  
  fputs("\n\nTList:\n", f);
  for(i = 0; i < triCount; i++){
    fprintf(f, " .db %d,%d,%d\n", tris[i].v[0], tris[i].v[1], tris[i].v[2]);
  }
  
  fputs("\n\nNList:\n", f);
  for(i = 0; i < triCount; i++){
    fprintf(f, " .db float($%06X),float($%06X),float($%06X)\n", floatConv(tris[i].normal[0]), floatConv(tris[i].normal[1]), floatConv(tris[i].normal[2]));
  }
  
  fputs("\n#include \"VDataEnd.inc\"\n.end\n", f);
  fclose(f);
  
  
  // Save bin file
  
  sprintf(outname, "%s.bin", name);
  f = fopen(outname, "wb");
  bindata.vCount = treeCount;
  bindata.tCount = triCount;
  bindata.VListPtr = sizeof(bindata)+2;
  bindata.TListPtr = bindata.VListPtr + treeCount*9;
  bindata.NListPtr = bindata.TListPtr + triCount*3;
  bindata.VCalcPtr = bindata.NListPtr + triCount*9;
  bindata.TSortPtr = bindata.VCalcPtr + treeCount*9;
  j = bindata.TSortPtr + triCount*4 - 2;
  if(j > 8811) printf(" !!! WARNING: Filesize has exceeded maximum !\n");
  fwrite(&bindata, 1, sizeof(bindata), f);
  for(i = 0; i < treeCount; i++){
    for(j = 0; j < 3; j++){
      out = floatConv(tree[i].v[j]);
      exp = out >> 16;
      fwrite(&exp, 1, 1, f);
      fwrite(&out, 1, 2, f);
    }
  }
  for(i = 0; i < triCount; i++){
    for(j = 0; j < 3; j++){
      fwrite(&tris[i].v[j], 1, 1, f);
    }
  }
  for(i = 0; i < triCount; i++){
    for(j = 0; j < 3; j++){
      out = floatConv(tris[i].normal[j]);
      exp = out >> 16;
      fwrite(&exp, 1, 1, f);
      fwrite(&out, 1, 2, f);
    }
  }
  out = 0xFFFFFFFF;
  for(j = treeCount*9 + triCount*4; j >= 4; j -= 4){
    fwrite(&out, 1, 4, f);
  }
  if(j) fwrite(&out, 1, j, f);
  fclose(f);
  
  free(tree);
  free(tris);
  
  printf("%d vertices (%d B)\n%d triangles (%d B)\n%d normals (%d B)\n\nTOTAL: %d B\n", treeCount, treeCount * 9, triCount, triCount, triCount, triCount * 9, (treeCount + triCount) * 9 + triCount);
  
  system("pause");
  return 0;
}


u32 vTreeAdd(vTree* tree, u32* treeCount, float* v){
  u32 axis = 0;
  u32 id = 0;
  u32 i;
  bool same = true;
  bool add = false;
  
  if(*treeCount == 0){
    add = true;
  }
  while(!add && id != 0xFFFFFFFF){
    same = true;
    for(i=0; i<3; i++){
      if(v[i] != tree[id].v[i]){
        same = false;
        break;
      }
    }
    if(same) return id;
    if(v[axis] < tree[id].v[axis]){
      if(tree[id].left == 0xFFFFFFFF){
        add = true;
        tree[id].left = *treeCount;
        break;
      }
      id = tree[id].left;
    }else{
      if(tree[id].right == 0xFFFFFFFF){
        add = true;
        tree[id].right = *treeCount;
        break;
      }
      id = tree[id].right;
    }
    axis++;
    if(axis == 3) axis = 0;
  }
  if(add){
    id = *treeCount;
    (*treeCount)++;
    for(i=0; i<3; i++){
      tree[id].v[i] = v[i];
    }
    tree[id].left = 0xFFFFFFFF;
    tree[id].right = 0xFFFFFFFF;
    return id;
  }
  return 0xFFFFFFFF; // something went wrong
}

u32 floatConv(float f){
  s32 fi = *(u32*)(&f);
  bool sign = fi < 0;
  s32 exp = ((fi & 0x7F800000) >> 23) - 64;
  s32 mant;
  
  if(exp > 126){
    exp = 127;
    mant = 0;
  }else if(exp < 0){
    exp = 0;
    mant = 0;
    sign = false;
  }else{
    mant = (((fi & 0x7FFF80) + 0x80) >> 8) | 0x8000;
  }
  if(sign) exp |= 0x80;
  return (exp << 16) | mant;
}
