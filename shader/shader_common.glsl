#define MAX_NUM_SKELETAL_NODES 96
#define MAX_MORPH_LAYERS       3

#define L_Diffuse  0
#define L_Shadow0  1
#define L_Shadow1  2
#define L_Scene    3
#define L_Skinning 4
#define L_Material 5
#define L_GDiffuse 6
#define L_GDepth   7
#define L_MorphId  8
#define L_Morph    9

#if defined(VERTEX) && (defined(OBJ) || defined(SKINING) || defined(MORPH))
#define MAT_ANIM 1
#endif

struct Light {
  vec4  pos;
  vec3  color;
  float range;
  };

struct MorphDesc {
  uint  indexOffset;
  uint  sample0;
  uint  sample1;
  float alpha;
  };

#if defined(OBJ) || defined(SKINING) || defined(MORPH)
layout(push_constant, std140) uniform UboPush {
  mat4      obj;
  float     fatness;
#if defined(MORPH)
  MorphDesc morph[MAX_MORPH_LAYERS];
#endif
  } push;
#endif

#if defined(FRAGMENT) && !(defined(SHADOW_MAP) && !defined(ATEST))
layout(binding = L_Diffuse) uniform sampler2D textureD;
#endif

#if defined(FRAGMENT) && !defined(SHADOW_MAP)
layout(binding = L_Shadow0) uniform sampler2D textureSm0;
layout(binding = L_Shadow1) uniform sampler2D textureSm1;
#endif

layout(binding = L_Scene,    std140) uniform UboScene {
  vec3  ldir;
  float shadowSize;
  mat4  mv;
  mat4  modelViewInv;
  mat4  shadow[2];
  vec3  ambient;
  vec4  sunCl;
  } scene;

#if defined(SKINING) && defined(VERTEX)
layout(binding = L_Skinning, std140) uniform UboAnim {
  mat4 skel[MAX_NUM_SKELETAL_NODES];
  } anim;
#endif

#if defined(VERTEX) && defined(MAT_ANIM)
layout(binding = L_Material, std140) uniform UboMaterial {
  vec2 texAnim;
  } material;
#endif

#if defined(FRAGMENT) && (defined(WATER) || defined(GHOST))
layout(binding = L_GDiffuse) uniform sampler2D gbufferDiffuse;
layout(binding = L_GDepth  ) uniform sampler2D gbufferDepth;
#endif

#if defined(VERTEX) && defined(MORPH)
layout(binding = L_MorphId, std140) readonly buffer SsboMorphId {
  ivec4 index[];
  } morphId;
layout(binding = L_Morph, std140) readonly buffer SsboMorph {
  vec4  samples[];
  } morph;
#endif
