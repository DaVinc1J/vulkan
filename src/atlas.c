#include "headers/atlas.h"

void split_division(_packer *packer, u16 tex_w, u16 tex_h, _division *division) {
	u16 x = division->x;
	u16 y = division->y;
	u16 w = division->w;
	u16 h = division->h;

	u16 swap_split = 0;

	if (h <= 256) {
		swap_split = 1;
	}

	if (h > tex_h) {
		_division bottom_subdiv = { 
			x, 
			y + tex_h, 
			swap_split ? tex_w : w,
			h - tex_h 
		};

		packer->divisions = realloc(packer->divisions, 
															sizeof(_division) * (packer->division_count + 1));
		packer->divisions[packer->division_count++] = bottom_subdiv;
	}

	if (w > tex_w) {
		_division right_subdiv = { 
			x + tex_w, 
			y, 
			w - tex_w, 
			swap_split ? h : tex_h
		};

		packer->divisions = realloc(packer->divisions, sizeof(_division) * (packer->division_count + 1));
		packer->divisions[packer->division_count++] = right_subdiv;
	}
}

void merge_divisions(_packer *packer, u16 f, u16 s) {
	_division *first = &packer->divisions[f];
	_division *second = &packer->divisions[s];

	_division merged = {
		.x = (first->x < second->x) ? first->x : second->x,
		.y = (first->y < second->y) ? first->y : second->y,
		.w = (first->x == second->x) ? first->w : first->w + second->w,
		.h = (first->y == second->y) ? first->h : first->h + second->h,
	};

	packer->divisions[f] = merged;
	packer->divisions[s] = packer->divisions[--packer->division_count];
	packer->divisions = realloc(packer->divisions, sizeof(_division) * packer->division_count);
}

u16 can_merge(_division *a, _division *b) {
	if (a->y == b->y && a->h == b->h && (a->x + a->w == b->x || b->x + b->w == a->x)) {
		return 1;
	}

	if (a->x == b->x && a->w == b->w && (a->y + a->h == b->y || b->y + b->h == a->y)) {
		return 1;
	}

	return 0;
}

u16 place_texture(_packer *packer, u16 tex_w, u16 tex_h, u16 original_index) {
retry:

	for (u32 i = 0; i < packer->division_count; i++) {
		_division *division = &packer->divisions[i];


		if (tex_w <= division->w && tex_h <= division->h) {
			packer->textures = realloc(packer->textures, sizeof(_texture) * (packer->texture_count + 1));
			packer->textures[packer->texture_count] = (_texture){
				.index = original_index,
				.x = division->x,
				.y = division->y,
				.w = tex_w,
				.h = tex_h,
				.flags = 0
			};

			packer->texture_count++;

			split_division(packer, tex_w, tex_h, division);
			packer->divisions[i] = packer->divisions[--packer->division_count];
			i--;

			return 1;
		} else if (tex_h <= division->w && tex_w <= division->h) {

			packer->textures = realloc(packer->textures, sizeof(_texture) * (packer->texture_count + 1));
			packer->textures[packer->texture_count] = (_texture){
				.index = original_index,
				.x = division->x,
				.y = division->y,
				.w = tex_h,
				.h = tex_w,
				.flags = TEXTURE_FLAG_IS_ROTATED
			};

			packer->texture_count++;

			split_division(packer, tex_h, tex_w, division);
			packer->divisions[i] = packer->divisions[--packer->division_count];
			i--;

			return 1;
		}


	}

	if (packer->division_count > 1) {
		for (u16 first = 0; first < packer->division_count - 1; first++) {
			for (u16 second = first + 1; second < packer->division_count; second++) {
				if (can_merge(&packer->divisions[first], &packer->divisions[second])) {
					merge_divisions(packer, first, second);
					goto retry;
				}
			}
		}
	}

	return 0;
}

int compare_entries(const void *a, const void *b) {
	const _texture_entry *ta = (const _texture_entry *)a;
	const _texture_entry *tb = (const _texture_entry *)b;
	if (ta->w > tb->w) return -1;
	if (ta->w < tb->w) return 1;
	if (ta->h > tb->h) return -1;
	if (ta->h < tb->h) return 1;
	return 0;
}

void generate_atlas(const char *filename, _packer *packer, _textures *textures, u16 *remap) {
	u32 width = packer->scale;
	u32 height = packer->scale;
	u32 channels = 4;
	u8 *atlas = calloc(width * height * channels, 1);

	for (u16 i = 0; i < packer->texture_count; i++) {
		_texture *tex = &packer->textures[i];

		u16 original_index = tex->index;
		assert(original_index < textures->count);

		u16 sorted_index = remap[original_index];
		assert(sorted_index < textures->count);

		_texture_entry *entry = &textures->entries[sorted_index];
		assert(entry->pixels != NULL);

		for (u16 y = 0; y < tex->h; y++) {
			for (u16 x = 0; x < tex->w; x++) {
				u32 dst_x = tex->x + x;
				u32 dst_y = tex->y + y;
				u32 dst_index = (dst_y * width + dst_x) * channels;

				u32 src_index = (y * tex->w + x) * channels;
				memcpy(&atlas[dst_index], &entry->pixels[src_index], channels);
			}
		}
	}

	stbi_write_png(filename, width, height, channels, atlas, width * channels);
	free(atlas);
}

void resize_packer(_packer *packer) {
	_division bottom_div = {0, packer->scale, packer->scale, packer->scale};
	_division right_div = {packer->scale, 0, packer->scale, 2 * packer->scale};
	packer->divisions = realloc(packer->divisions, sizeof(_division) * (packer->division_count + 2));
	packer->divisions[packer->division_count++] = bottom_div;
	packer->divisions[packer->division_count++] = right_div;
	packer->scale *= 2;
}

u16 *build_remap_table(_textures *textures) {
	u16 *table = malloc(sizeof(u16) * textures->count);
	for (u16 i = 0; i < textures->count; i++) {
		table[textures->entries[i].index] = i;
	}
	return table;
}

void pack_atlas(_packer *packer, _textures *textures) {
	_division initial_division = { 0, 0, packer->scale, packer->scale};
	packer->divisions = malloc(sizeof(_division));
	packer->divisions[0] = initial_division;
	packer->division_count = 1;

	srand(time(NULL));

	qsort(textures->entries, textures->count, sizeof(_texture_entry), compare_entries);

	for (u16 i = 0; i < textures->count; i++) {
		u16 tex_w = textures->entries[i].w;
		u16 tex_h = textures->entries[i].h;
		if (!place_texture(packer, tex_w, tex_h, textures->entries[i].index)) {
		resize:
			resize_packer(packer);
			if (!place_texture(packer, tex_w, tex_h, textures->entries[i].index)) {
				goto resize;
			}
		}
	}
}
