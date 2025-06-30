#include "headers/object.h"
#include "headers/atlas.h"

void load_gltf(_app *p_app) {
	u32 file_count = p_app->tex.gltf.file_count;
	p_app->obj.data = malloc(sizeof(cgltf_data*) * file_count);

	for (u32 i = 0; i < file_count; i++) {
		const char *dir = p_app->config.dir.objects;
		const char *file = p_app->tex.gltf.file_names[i];
		size_t len = strlen(dir) + strlen(file) + 1;
		char *path = malloc(len);
		strcpy(path, dir);
		strcat(path, file);

		cgltf_options options = {0};
		cgltf_parse_file(&options, path, &p_app->obj.data[i]);
		cgltf_load_buffers(&options, p_app->obj.data[i], path);
		free(path);

		u16 count = p_app->obj.data[i]->textures_count;
		_packer packer;
		packer.scale = 1024;
		packer.max_scale = 16384;
		packer.flags = PACKER_FLAG_NONE;
		//packer.flags = PACKER_FLAG_ALWAYS_REGENERATE;
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
		snprintf(atlas_file, sizeof(atlas_file), "atlas_%u.png", p_app->tex.gltf.atlas_count);
		p_app->tex.gltf.atlas_names = realloc(p_app->tex.gltf.atlas_names, 
																				sizeof(char *) * (p_app->tex.gltf.atlas_count + 1));
		p_app->tex.gltf.atlas_names[p_app->tex.gltf.atlas_count] = strdup(atlas_file);
		const char *atlas_dir = p_app->config.dir.atlases;
		size_t atlas_len = strlen(atlas_dir) + strlen(atlas_file) + 1;
		char *atlas_path = malloc(atlas_len);
		strcpy(atlas_path, atlas_dir);
		strcat(atlas_path, atlas_file);

		generate_atlas(atlas_path, &packer, &infos, textures, remap);
		//print_atlas(&packer, &infos, textures, remap);
		//generate_debug_atlas("debug_atlas.png", &packer, &infos, textures);
		p_app->tex.gltf.atlas_count++;

		cleanup_atlas(&packer, &infos, remap);
		free(textures);
		textures = NULL;
	}
}

void read_obj_file(_app *p_app) {
	u32 file_count = p_app->tex.object.file_count;
	_app_objects *fillers = calloc(file_count, sizeof(_app_objects));

	p_app->obj.object_count = 0;
	p_app->obj.texture_count = 0;

	for (u32 i = 0; i < file_count; i++) {

		const char *dir = p_app->config.dir.objects;
		const char *file = p_app->tex.object.file_names[i];

		size_t len = strlen(dir) + strlen(file) + 1;
		char *path = malloc(len);

		strcpy(path, dir);
		strcat(path, file);	

		fillers[i].mesh = fast_obj_read(path);
		free(path);

		u32 oc = fillers[i].mesh->object_count;

		fillers[i].vertices = malloc(oc * sizeof(_vertex*));
		fillers[i].indices = malloc(oc * sizeof(u32*));
		fillers[i].face_indices = malloc(oc * sizeof(u32*));
		fillers[i].material_indices = malloc(oc * sizeof(u32*));
		fillers[i].group_indices = malloc(oc * sizeof(u32*));
		fillers[i].object_indices = malloc(oc * sizeof(u32*));
		fillers[i].texture_indices = malloc(oc * sizeof(u32*));

		fillers[i].vertices_count = malloc(oc * sizeof(u32));
		fillers[i].indices_count = malloc(oc * sizeof(u32));

		flatten(&fillers[i]);
		p_app->obj.object_count += oc;
		p_app->obj.texture_count += fillers[i].mesh->texture_count - 1;
	}

	_app_objects *dst = &p_app->obj;
	dst->mesh = NULL;
	dst->vertices = malloc(dst->object_count * sizeof(_vertex*));
	dst->indices = malloc(dst->object_count * sizeof(u32*));
	dst->face_indices = malloc(dst->object_count * sizeof(u32*));
	dst->material_indices = malloc(dst->object_count * sizeof(u32*));
	dst->group_indices = malloc(dst->object_count * sizeof(u32*));
	dst->object_indices = malloc(dst->object_count * sizeof(u32*));
	dst->texture_indices = malloc(dst->object_count * sizeof(u32*));
	dst->vertices_count = malloc(dst->object_count * sizeof(u32));
	dst->indices_count = malloc(dst->object_count * sizeof(u32));
	dst->textures = malloc(dst->texture_count * sizeof(fastObjTexture));

	u32 dst_i = 0;
	u32 tex_offset = 0;
	for (u32 i = 0; i < file_count; i++) {
		u32 oc = fillers[i].mesh->object_count;
		u32 tc = fillers[i].mesh->texture_count;

		for (u32 t = 1; t < tc; t++) {
			dst->textures[tex_offset + t - 1] = fillers[i].mesh->textures[t];
			if (fillers[i].mesh->textures[t].path)
				dst->textures[tex_offset + t - 1].path = strdup(fillers[i].mesh->textures[t].path);
			if (fillers[i].mesh->textures[t].name)
				dst->textures[tex_offset + t - 1].name = strdup(fillers[i].mesh->textures[t].name);
		}

		for (u32 j = 0; j < oc; j++, dst_i++) {
			u32 v_count = fillers[i].vertices_count[j];
			u32 i_count = fillers[i].indices_count[j];

			dst->vertices[dst_i] = malloc(sizeof(_vertex) * v_count);
			dst->indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->face_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->material_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->group_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->object_indices[dst_i] = malloc(sizeof(u32) * i_count);
			dst->texture_indices[dst_i] = malloc(sizeof(u32) * i_count);

			memcpy(dst->vertices[dst_i], fillers[i].vertices[j], sizeof(_vertex) * v_count);
			memcpy(dst->indices[dst_i], fillers[i].indices[j], sizeof(u32) * i_count);
			memcpy(dst->face_indices[dst_i], fillers[i].face_indices[j], sizeof(u32) * i_count);
			memcpy(dst->material_indices[dst_i], fillers[i].material_indices[j], sizeof(u32) * i_count);
			memcpy(dst->group_indices[dst_i], fillers[i].group_indices[j], sizeof(u32) * i_count);
			memcpy(dst->object_indices[dst_i], fillers[i].object_indices[j], sizeof(u32) * i_count);

			for (u32 k = 0; k < i_count; k++) {
				u32 tex_id = fillers[i].texture_indices[j][k];
				dst->texture_indices[dst_i][k] = tex_id > 0 ? (tex_offset + tex_id - 1) : UINT32_MAX;
			}

			dst->vertices_count[dst_i] = v_count;
			dst->indices_count[dst_i] = i_count;
		}

		fast_obj_destroy(fillers[i].mesh);
		free(fillers[i].vertices);
		free(fillers[i].indices);
		free(fillers[i].face_indices);
		free(fillers[i].material_indices);
		free(fillers[i].group_indices);
		free(fillers[i].object_indices);
		free(fillers[i].texture_indices);
		free(fillers[i].vertices_count);
		free(fillers[i].indices_count);

		tex_offset += tc - 1;
	}
	free(fillers);

	for (u32 o = 0; o < dst->object_count; o++) {
		for (u32 k = 0; k < dst->indices_count[o]; k++) {
			u32 idx = dst->indices[o][k];
			dst->vertices[o][idx].tex_index = dst->texture_indices[o][k];
		}
	}

	u32 tex_count = dst->texture_count;
	u32 *remap_table = malloc(sizeof(u32) * tex_count);
	u32 new_index = 0;

	for (u32 i = 0; i < tex_count; i++) {
		const char *path = dst->textures[i].path;
		if (path && strlen(path) > 0) {
			remap_table[i] = new_index++;
		} else {
			remap_table[i] = UINT32_MAX;
		}
	}

	for (u32 o = 0; o < dst->object_count; o++) {
		for (u32 v = 0; v < dst->vertices_count[o]; v++) {
			u32 old = dst->vertices[o][v].tex_index;
			dst->vertices[o][v].tex_index = (old < tex_count && remap_table[old] != UINT32_MAX) ? remap_table[old] : 0;
		}
	}
	free(remap_table);
}

void flatten(_app_objects* p_obj) {

	u32 idx_offset = 0;

	for (u32 o = 0; o < p_obj->mesh->object_count; ++o) {
		fastObjGroup* obj = &p_obj->mesh->objects[o];

		u32 index_count = 0;
		for (u32 f = obj->face_offset; f < obj->face_offset + obj->face_count; ++f) {
			uint32_t fv = p_obj->mesh->face_vertices[f];
			if (fv >= 3)
				index_count += (fv - 2) * 3;
		}

		p_obj->vertices[o] = malloc(index_count * sizeof(_vertex));
		p_obj->indices[o] = malloc(index_count * sizeof(u32));
		p_obj->face_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->material_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->group_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->object_indices[o] = malloc(index_count * sizeof(u32));
		p_obj->texture_indices[o] = malloc(index_count * sizeof(u32));

		p_obj->vertices_count[o] = 0;
		p_obj->indices_count[o] = 0;

		u32 table_size = 1024;
		while (table_size < index_count) table_size <<= 1;
		_node** table = calloc(table_size, sizeof(_node*));
		_node* n_pool = malloc(index_count * sizeof(_node));

		u32 pool_index = 0;

		for (u32 f = obj->face_offset; f < obj->face_offset + obj->face_count; ++f) {
			uint32_t fv = p_obj->mesh->face_vertices[f];
			if (fv < 3) continue;

			u32 mat = 0; 
			if (p_obj->mesh->face_materials) {
				mat = p_obj->mesh->face_materials[f];
			}

			for (u32 i = 1; i + 1 < fv; ++i) {
				fastObjIndex tri[3] = {
					p_obj->mesh->indices[idx_offset + 0],
					p_obj->mesh->indices[idx_offset + i],
					p_obj->mesh->indices[idx_offset + i + 1]
				};

				for (int t = 0; t < 3; ++t) {
					fastObjIndex idx = tri[t];
					_vertex v = {0};

					if (idx.p >= 0 && idx.p < p_obj->mesh->position_count) {
						v.pos[0] = p_obj->mesh->positions[3 * idx.p + 0];
						v.pos[2] = -p_obj->mesh->positions[3 * idx.p + 1];
						v.pos[1] = p_obj->mesh->positions[3 * idx.p + 2];
					}

					if (idx.t >= 0 && idx.t < p_obj->mesh->texcoord_count) {
						v.tex[0] = p_obj->mesh->texcoords[2 * idx.t + 0];
						v.tex[1] = p_obj->mesh->texcoords[2 * idx.t + 1];
					}

					if (idx.n >= 0 && idx.n < p_obj->mesh->normal_count) {
						v.norm[0] = p_obj->mesh->normals[3 * idx.n + 0];
						v.norm[2] = -p_obj->mesh->normals[3 * idx.n + 1];
						v.norm[1] = p_obj->mesh->normals[3 * idx.n + 2];
					}

					v.tex_index = p_obj->mesh->materials[mat].map_Kd - 1;

					u32 hash = 5381;
					unsigned char* bytes = (unsigned char*)&v;
					for (size_t j = 0; j < sizeof(_vertex); j++) {
						hash = ((hash << 5) + hash) + bytes[j];
					}
					hash &= (table_size - 1);

					_node* n = table[hash];
					u32 found_index = 0;
					int found = 0;

					while (n) {
						if (memcmp(&n->vertex, &v, sizeof(_vertex)) == 0) {
							found = 1;
							found_index = n->index;
							break;
						}
						n = n->next;
					}

					if (found) {
						p_obj->indices[o][p_obj->indices_count[o]] = found_index;
					} else {
						p_obj->vertices[o][p_obj->vertices_count[o]] = v;
						p_obj->indices[o][p_obj->indices_count[o]] = p_obj->vertices_count[o];

						_node* new_n = &n_pool[pool_index++];
						new_n->vertex = v;
						new_n->index = p_obj->vertices_count[o];
						new_n->next = table[hash];
						table[hash] = new_n;

						p_obj->vertices_count[o]++;
					}

					fastObjMaterial *material = &p_obj->mesh->materials[mat];

					u32 texture_index = material->map_Kd;

					if (texture_index > 0 && texture_index <= p_obj->mesh->texture_count) {
						p_obj->texture_indices[o][p_obj->indices_count[o]] = texture_index;
					} else {
						p_obj->texture_indices[o][p_obj->indices_count[o]] = 0;
					}

					p_obj->face_indices[o][p_obj->indices_count[o]] = f;
					p_obj->material_indices[o][p_obj->indices_count[o]] = mat;
					p_obj->group_indices[o][p_obj->indices_count[o]] = o;
					p_obj->object_indices[o][p_obj->indices_count[o]] = o;

					p_obj->indices_count[o]++;
				}
			}

			idx_offset += fv;
		}

		free(table);
		free(n_pool);
	}
}
