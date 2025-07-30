#include "3d.h"

leaf_t *mapa_discoverLeaf(vetor3d_t *pos, mapa_t *mapa);
int mapa_canMoveTo(float px, float py, float pz, mapa_t *mapa);
bool mapa_face_solida(face_t *face);
byte *mapa_leafVIS (leaf_t *leaf, mapa_t *mapa);

int mapa_loadEntities (mapa_t *mapa);

void mostraMapa2D(mapa_t *mapa, camera_t *cam, byte *vis);

bool mapa_trace_bsp_visibilidade(mapa_t *mapa, vetor3d_t de, vetor3d_t para);
