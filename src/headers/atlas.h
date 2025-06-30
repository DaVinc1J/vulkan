#ifndef ATLAS_H
#define ATLAS_H

#include "define.h"

void cleanup_atlas(_packer *packer, _infos *infos, u16 *remap);
void init_atlas(_packer *packer, _infos *infos, u16 **remap);
void pack_atlas(_packer *packer, _infos *infos, _texture **textures, u16 *remap);
static void draw_digit(u8 *img, int w, int h, int x, int y, char digit, u32 color);
void generate_debug_atlas(const char *filename, _packer *packer, _infos *infos, _texture *textures);
void print_atlas(_packer *packer, _infos *infos, _texture *textures, u16 *remap);
void generate_atlas(const char *filename, _packer *packer, _infos *infos, _texture *textures, u16 *remap);
void split_division(_packer *packer, u16 tex_w, u16 tex_h, _division *division);
void merge_divisions(_packer *packer, u16 f, u16 s);
u16 can_merge(_division *a, _division *b);
u16 place_texture(_packer *packer, _infos *infos, _texture *textures, u16 index);
int compare_entries(const void *a, const void *b);
void resize_packer(_packer *packer);
int pack_textures(_app *p_app);

#endif
