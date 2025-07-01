#include "headers/atlas.h"

static const u8 digit_font[10][5] = {
	{0b111, 0b101, 0b101, 0b101, 0b111},
	{0b010, 0b110, 0b010, 0b010, 0b111},
	{0b111, 0b001, 0b111, 0b100, 0b111},
	{0b111, 0b001, 0b111, 0b001, 0b111},
	{0b101, 0b101, 0b111, 0b001, 0b001},
	{0b111, 0b100, 0b111, 0b001, 0b111},
	{0b111, 0b100, 0b111, 0b101, 0b111},
	{0b111, 0b001, 0b010, 0b100, 0b100},
	{0b111, 0b101, 0b111, 0b101, 0b111},
	{0b111, 0b101, 0b111, 0b001, 0b111},
};

void draw_digit(u8 *img, int w, int h, int x, int y, char digit, u32 color) {
	if (digit < '0' || digit > '9') return;
	const u8 *glyph = digit_font[digit - '0'];

	for (int row = 0; row < 5; row++) {
		for (int col = 0; col < 3; col++) {
			if (glyph[row] & (1 << (2 - col))) {
				int px = x + col;
				int py = y + row;
				if (px >= 0 && px < w && py >= 0 && py < h) {
					u8 *p = &img[(py * w + px) * 4];
					p[0] = color & 0xFF;
					p[1] = (color >> 8) & 0xFF;
					p[2] = (color >> 16) & 0xFF;
					p[3] = 255;
				}
			}
		}
	}
}

void generate_debug_atlas(const char *filename, _packer *packer, _infos *infos, _texture *textures) {
	u32 width = packer->scale;
	u32 height = packer->scale;
	u32 channels = 4;
	u8 *atlas = calloc(width * height * channels, 1);
	srand((u32)time(NULL));

	for (u16 i = 0; i < infos->count; i++) {
		_texture *t = &textures[i];
		u8 r = rand() % 200 + 40;
		u8 g = rand() % 200 + 40;
		u8 b = rand() % 200 + 40;

		for (u32 y = 0; y < t->h; y++) {
			for (u32 x = 0; x < t->w; x++) {
				u32 dx = t->x + x;
				u32 dy = t->y + y;
				u8 *p = &atlas[(dy * width + dx) * 4];
				p[0] = r; 
				p[1] = g; 
				p[2] = b; 
				p[3] = 255;
			}
		}

		char digits[6];
		snprintf(digits, sizeof(digits), "%u", i);
		for (int d = 0; digits[d]; d++) {
			draw_digit(atlas, width, height, t->x + d * 4, t->y, digits[d], 0xFFFFFF);
		}
	}

	for (u16 i = 0; i < packer->division_count; i++) {
		_division *div = &packer->divisions[i];
		
		for (u32 y = 0; y < div->h; y++) {
			for (u32 x = 0; x < div->w; x++) {
				u32 dx = div->x + x;
				u32 dy = div->y + y;
				u8 *p = &atlas[(dy * width + dx) * 4];
				if (x == div->w || x == 0 || y == div->h || y == 0) {
					p[0] = 0;
					p[1] = 0;
					p[2] = 0;
					p[3] = 255;
				} else {
					p[0] = 128;
					p[1] = 128;
					p[2] = 128;
					p[3] = 255;
				}
			}
		}

		char digits[6];
		snprintf(digits, sizeof(digits), "%u", i);
		for (int d = 0; digits[d]; d++) {
			draw_digit(atlas, width, height, div->x + d * 4, div->y, digits[d], 0xFFFFFF);
		}
	}

	stbi_write_png(filename, width, height, channels, atlas, width * channels);
	free(atlas);
}

void print_atlas(_packer *packer, _infos *infos, _texture *textures, u16 *remap) {
	printf("Atlas Scale: %u x %u\n", packer->scale, packer->scale);
	printf("Entries:\n");
	for (u16 i = 0; i < infos->count; i++) {
		_entry *e = &infos->entries[i];
		printf("  [%u] w=%u h=%u index=%u\n", i, e->w, e->h, e->index);
	}
	printf("Remap Table:\n");
	for (u16 i = 0; i < infos->count; i++) {
		printf("  remap[%u] = %u\n", i, remap[i]);
	}
	printf("Textures:\n");
	for (u16 i = 0; i < infos->count; i++) {
		_texture *t = &textures[i];
		printf("  tex[%u]: x=%u y=%u w=%u h=%u flags=0x%x\n", i, t->x, t->y, t->w, t->h, t->flags);
	}
	printf("Divisions:\n");
	for (u16 i = 0; i < packer->division_count; i++) {
		_division *d = &packer->divisions[i];
		printf("  [%u] x=%u y=%u w=%u h=%u\n", i, d->x, d->y, d->w, d->h);
	}
}

void split_division(_packer *packer, u16 tex_w, u16 tex_h, _division *division) {
	u16 x = division->x;
	u16 y = division->y;
	u16 w = division->w;
	u16 h = division->h;

	u16 swap_split = (h <= 256);
	u16 new_divisions = 0;

	if (h > tex_h) new_divisions++;
	if (w > tex_w) new_divisions++;

	if (new_divisions > 0) {
		packer->divisions = realloc(packer->divisions, 
															sizeof(_division) * (packer->division_count + new_divisions));
		u16 added = 0;

		if (h > tex_h) {
			_division bottom_subdiv = { 
				x, 
				y + tex_h, 
				swap_split ? tex_w : w,
				h - tex_h 
			};
			packer->divisions[packer->division_count + added++] = bottom_subdiv;
		}

		if (w > tex_w) {
			_division right_subdiv = { 
				x + tex_w, 
				y, 
				w - tex_w, 
				swap_split ? h : tex_h
			};
			packer->divisions[packer->division_count + added] = right_subdiv;
		}
		packer->division_count += new_divisions;
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

u16 place_texture(_packer *packer, _infos *infos, _texture *textures, u16 sorted_index) {
retry:
	for (u32 i = 0; i < packer->division_count; i++) {
		_division *division = &packer->divisions[i];
		u16 tex_w = infos->entries[sorted_index].w;
		u16 tex_h = infos->entries[sorted_index].h;
		u16 orig_index = infos->entries[sorted_index].index;
		u8 rotated = 0;

		if (tex_w <= division->w && tex_h <= division->h) {
		} else if (tex_h <= division->w && tex_w <= division->h) {
			rotated = 1;
			u16 tmp = tex_w;
			tex_w = tex_h;
			tex_h = tmp;
		} else {
			continue;
		}

		textures[orig_index] = (_texture){
			.x = division->x,
			.y = division->y,
			.w = tex_w,
			.h = tex_h,
			.flags = rotated ? TEXTURE_FLAG_IS_ROTATED : 0
		};

		split_division(packer, tex_w, tex_h, division);
		packer->divisions[i] = packer->divisions[--packer->division_count];
		i--;
		return 1;
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
	const _entry *ta = (const _entry *)a;
	const _entry *tb = (const _entry *)b;
	if (ta->w > tb->w) return -1;
	if (ta->w < tb->w) return 1;
	if (ta->h > tb->h) return -1;
	if (ta->h < tb->h) return 1;
	return 0;
}

void generate_atlas(const char *filename, _packer *packer, _infos *infos, _texture *textures, u16 *remap) {
	if (!(packer->flags & PACKER_FLAG_ALWAYS_REGENERATE)) {
		if (access(filename, F_OK) == 0) {
			printf("skipping atlas %s (already exists)\n", filename);
		}
	}
	u32 width = packer->scale;
	u32 height = packer->scale;
	u32 channels = 4;
	u8 *atlas = calloc(width * height * channels, 1);

	for (u16 orig_index = 0; orig_index < infos->count; orig_index++) {
		_texture *tex = &textures[orig_index];
		u16 sorted_index = remap[orig_index];
		_entry *entry = &infos->entries[sorted_index];

		for (u16 y = 0; y < tex->h; y++) {
			for (u16 x = 0; x < tex->w; x++) {
				u32 dst_x = tex->x + x;
				u32 dst_y = tex->y + y;
				u32 dst_idx = (dst_y * width + dst_x) * channels;
				u32 src_idx = (y * tex->w + x) * channels;

				if (entry->pixels[src_idx + 3] < 255) {
    			tex->flags |= TEXTURE_FLAG_HAS_ALPHA;
				}

				memcpy(&atlas[dst_idx], &entry->pixels[src_idx], channels);
			}
		}
	}

	stbi_write_png(filename, width, height, channels, atlas, width * channels);
	free(atlas);

}

void resize_packer(_packer *packer) {
	u16 old_scale = packer->scale;
	u16 new_scale = old_scale * 2;

	_division bottom_div = {0, old_scale, new_scale, old_scale};
	_division right_div = {old_scale, 0, old_scale, old_scale};

	packer->divisions = realloc(packer->divisions, 
														 sizeof(_division) * (packer->division_count + 2));
	packer->divisions[packer->division_count++] = bottom_div;
	packer->divisions[packer->division_count++] = right_div;
	packer->scale = new_scale;

	if (packer->division_count > 1) {
		for (u16 first = 0; first < packer->division_count - 1; first++) {
			for (u16 second = first + 1; second < packer->division_count; second++) {
				if (can_merge(&packer->divisions[first], &packer->divisions[second])) {
					merge_divisions(packer, first, second);
				}
			}
		}
	}

}

void init_atlas(_packer *packer, _infos *infos, u16 **remap) {
	packer->divisions = malloc(sizeof(_division));
	packer->divisions[0] = (struct _division){0, 0, packer->scale, packer->scale};
	packer->division_count = 1;

	infos->entries = malloc(sizeof(_entry) * infos->count);

	*remap = malloc(sizeof(u16) * infos->count);
	for (u16 i = 0; i < infos->count; i++) {
		(*remap)[i] = 0;
	}
}

void cleanup_atlas(_packer *packer, _infos *infos, u16 *remap) {
	if (packer && packer->divisions) {
		free(packer->divisions);
		packer->divisions = NULL;
		packer->division_count = 0;
	}

	if (infos && infos->entries) {
		for (u16 i = 0; i < infos->count; i++) {
			stbi_image_free(infos->entries[i].pixels);
		}
		free(infos->entries);
		infos->entries = NULL;
		infos->count = 0;
	}

	if (remap) {
		free(remap);
	}
}

void pack_atlas(_packer *packer, _infos *infos, _texture **textures, u16 *remap) {
	qsort(infos->entries, infos->count, sizeof(_entry), compare_entries);

	for (u16 sorted_index = 0; sorted_index < infos->count; sorted_index++) {
		u16 original_index = infos->entries[sorted_index].index;
		remap[original_index] = sorted_index;
	}

	for (u16 sorted_index = 0; sorted_index < infos->count; sorted_index++) {
		if (!place_texture(packer, infos, *textures, sorted_index)) {
		resize:
			resize_packer(packer);
			if (!place_texture(packer, infos, *textures, sorted_index)) {
				if (packer->scale > packer->max_scale) goto exit;
				goto resize;
			}
		}
	}
exit:
	return;
}
