#include "headers/object.h"
#include "headers/atlas.h"

void load_gltf(_app *p_app) {
	u32 file_count = p_app->tex.file.count;
	p_app->obj.data = malloc(sizeof(cgltf_data*) * file_count);
	p_app->obj.flags = malloc(sizeof(u8) * file_count);

	for (u32 i = 0; i < file_count; i++) {
		p_app->obj.flags[i] = OBJECT_FLAG_CENTRE_AT_ZEO;

		const char *dir = p_app->config.dir.objects;
		const char *file = p_app->tex.file.names[i];
		size_t len = strlen(dir) + strlen(file) + 1;
		char *path = malloc(len);
		strcpy(path, dir);
		strcat(path, file);

		cgltf_options options = {0};
		cgltf_parse_file(&options, path, &p_app->obj.data[i]);
		cgltf_load_buffers(&options, p_app->obj.data[i], path);
		free(path);

		cgltf_data* data = p_app->obj.data[i];
		for (u32 m = 0; m < data->meshes_count; m++) {
			p_app->obj.primitive_count += data->meshes[m].primitives_count;
		}

		u16 count = p_app->obj.data[i]->textures_count;
		_packer packer;
		packer.scale = p_app->atlas.scale;
		packer.max_scale = p_app->atlas.max_scale;
		packer.flags = p_app->atlas.flags;
		_infos infos;
		infos.count = count;
		u16 *remap;
		_texture *textures = malloc(sizeof(_texture) * count);
		init_atlas(&packer, &infos, &remap);

		for (u32 j = 0; j < infos.count; j++) {
			const char *directory = p_app->config.dir.textures;
			const char *texture_file = p_app->obj.data[i]->textures[j].image->uri;

			size_t path_length = strlen(directory) + strlen(texture_file) + 1;
			char *texture_path = malloc(path_length);
			strcpy(texture_path, directory);
			strcat(texture_path, texture_file);

			int tex_w, tex_h, channels;
			infos.entries[j].pixels = stbi_load(texture_path, &tex_w, &tex_h, &channels, STBI_rgb_alpha);
			free(texture_path);

			infos.entries[j].w = (u16)tex_w;
			infos.entries[j].h = (u16)tex_h;
			infos.entries[j].index = j;
		}

		pack_atlas(&packer, &infos, &textures, remap);

		char atlas_file[32];
		snprintf(atlas_file, sizeof(atlas_file), "atlas_%u.png", p_app->tex.atlas.count);
		p_app->tex.atlas.names = realloc(p_app->tex.atlas.names, 
																	 sizeof(char *) * (p_app->tex.atlas.count + 1));
		p_app->tex.atlas.names[p_app->tex.atlas.count] = strdup(atlas_file);
		const char *atlas_dir = p_app->config.dir.atlases;
		size_t atlas_len = strlen(atlas_dir) + strlen(atlas_file) + 1;
		char *atlas_path = malloc(atlas_len);
		strcpy(atlas_path, atlas_dir);
		strcat(atlas_path, atlas_file);

		generate_atlas(atlas_path, &packer, &infos, textures, remap);
		p_app->atlas.textures = textures;
		p_app->atlas.texture_count = infos.count;
		//print_atlas(&packer, &infos, textures, remap);
		//generate_debug_atlas("debug_atlas.png", &packer, &infos, textures);
		p_app->tex.atlas.count++;

		cleanup_atlas(&packer, &infos, remap);
	}
}
