#ifndef ATLAS_H
#define ATLAS_H

#include "define.h"

void generate_atlas(const char *filename, _packer *packer, _textures *textures, u16 *remap);
u16 *build_remap_table(_textures *textures);
void pack_atlas(_packer *packer, _textures *textures);
void split_division(_packer *packer, u16 tex_w, u16 tex_h, _division *division);
void merge_divisions(_packer *packer, u16 f, u16 s);
u16 can_merge(_division *a, _division *b);
u16 place_texture(_packer *packer, u16 tex_w, u16 tex_h, u16 original_index);
int compare_entries(const void *a, const void *b);
void resize_packer(_packer *packer);
int pack_textures(_app *p_app);

#endif
