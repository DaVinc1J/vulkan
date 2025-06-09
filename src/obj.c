#include "obj.h"

char* preprocess(const char* input, int *p_line_count) {
	if (!input) return NULL;

	size_t capacity = strlen(input) + 1;
	char* output = malloc(capacity);
	if (!output) return NULL;

	const char* in = input;
	char* out = output;
	size_t size = 0;
	int line_count = 0;
	bool in_space = false;

	while (*in) {
		if (*in == '\\') {
			const char* next = in + 1;
			while (*next && isspace(*next) && *next != '\n') next++;
			if (*next == '\n') {
				in = next + 1;
				continue;
			}
		}

		if (*in == '#') {
			while (*in && *in != '\n') in++;
			in_space = false;
			continue;
		}

		if (isspace(*in)) {
			if (*in == '\n') {
				if (out != output && *(out - 1) != '\n') {
					*out++ = '\n';
					line_count++;
					size++;
				}
			} else if (!in_space) {
				*out++ = ' ';
				size++;
				in_space = true;
			}
			in++;
			continue;
		}

		if (in_space) {
			in_space = false;
		}
		*out++ = *in++;
		size++;

		if (size + 1 >= capacity) {
			capacity *= 2;
			char* new_output = realloc(output, capacity);
			if (!new_output) {
				free(output);
				return NULL;
			}
			out = new_output + size;
			output = new_output;
		}
	}

	*out = '\0';
	*p_line_count = line_count;

	char* final = realloc(output, size + 1);
	return final ? final : output;
}

char* read_obj(const char* path) {

	FILE* file = fopen(path, "rb");
	if (!file) {
		fprintf(stderr, "error opening %s => %s\n", path, strerror(errno));
		return NULL;
	}

	if (fseek(file, 0, SEEK_END)) {
		fclose(file);
		return NULL;
	}
	long size = ftell(file);
	if (size < 0) {
		fclose(file);
		return NULL;
	}
	rewind(file);

	char* data = malloc(size + 1);
	if (!data) {
		fclose(file);
		return NULL;
	}

	size_t read = fread(data, 1, size, file);
	if (read != (size_t)size) {
		free(data);
		fclose(file);
		return NULL;
	}

	data[size] = '\0';
	fclose(file);
	return data;
}

void preparse(_parser *p_parser) {
	const char* line = p_parser->lines;

	for (int i = 0; i < p_parser->line_count; i++) {
		while (isspace(*line)) line++;

		if (strncmp(line, "v ", 2) == 0) {
			p_parser->max_counts.positions++;
		}

		else if (strncmp(line, "vt ", 3) == 0) {
			p_parser->max_counts.texcoords++;
		}

		else if (strncmp(line, "vn ", 3) == 0) {
			p_parser->max_counts.normals++;
		}

		else if (strncmp(line, "f ", 2) == 0) {
			p_parser->max_counts.faces++;

			int verts = 0;
			const char* start = line + 2;
			while (*start && *start != '\n') {
				if (!isspace(*start)) {
					verts++;
					while (*start && !isspace(*start)) start++;
				} else start++;
			}
			if (verts >= 3) {
				p_parser->max_counts.triangles += verts - 2;
				if (verts > p_parser->max_verts) p_parser->max_verts = verts;
				const char* line_end = strchr(line, '\n');
				int line_len = line_end ? (int)(line_end - line) : (int)strlen(line);
			}
		}

		while (*line && *line != '\n') line++;
		if (*line == '\n') line++;
	}
}

void parse(_parser parser, _mesh *p_mesh, _counts *p_count) {
	const char *line = parser.lines;

	for (int i = 0; i < parser.line_count; i++) {
		while (isspace(*line)) line++;

		if (strncmp(line, "v ", 2) == 0) {
			line += 2;

			char* end;
			errno = 0;
			double x = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in vertex x coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			errno = 0;
			double y = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in vertex y coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			errno = 0;
			double z = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in vertex z coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			p_mesh->positions[p_count->positions++] = (_vec3){x, y, z};
		} else if (strncmp(line, "vt ", 3) == 0) {
			line += 3;

			char *end;

			errno = 0;
			double u = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in texture u component\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			errno = 0;
			double v = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in texture v component\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			p_mesh->texcoords[p_count->texcoords++] = (_vec2){u, v};
		} else if (strncmp(line, "vn ", 3) == 0) {
			line += 3;

			char* end;
			errno = 0;
			double x = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in normal x coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			errno = 0;
			double y = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in normal y coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			errno = 0;
			double z = strtod(line, &end);
			if (end == line || errno == ERANGE) {
				printf("invalid double in normal z coordinate\n");
				while (*line && *line != '\n') line++;
				if (*line == '\n') line++;
				continue;
			}
			line = end;

			p_mesh->normals[p_count->normals++] = (_vec3){x, y, z};
		}  else if (strncmp(line, "f ", 2) == 0) {
			line += 2;
			_indexed_vertex face_verts[(parser.max_verts < MAX_FACE_VERTS ? parser.max_verts : MAX_FACE_VERTS)];
			int nverts = 0;
			int valid_face = 1;

			while (*line && *line != '\n' && nverts < MAX_FACE_VERTS) {
				while (isspace(*line)) line++;
				if (*line == '\n' || *line == '\0') break;

				_indexed_vertex v = {-1, -1, -1};

				char* end;
				errno = 0;
				long val = strtol(line, &end, 10);
				if (end == line || errno == ERANGE) {
					valid_face = 0;
					printf("Invalid position index in face\n");
					break;
				}

				if (val < 0) {
					v.pos = p_count->positions + val;
				} else {
					v.pos = val - 1;
				}

				if (v.pos < 0 || v.pos >= p_count->positions) {
					valid_face = 0;
					printf("pos index out of range: %ld (%d available)\n", 
						val, p_count->positions);
					break;
				}
				line = end;

				if (*line == '/') {
					line++;

					if (*line != '/' && *line != '\n' && !isspace(*line)) {
						errno = 0;
						val = strtol(line, &end, 10);
						if (end != line && errno != ERANGE) {
							if (val < 0) {
								v.tex = p_count->texcoords + val;
							} else {
								v.tex = val - 1;
							}
							if (v.tex < 0 || v.tex >= p_count->texcoords) {
								valid_face = 0;
								printf("Texture index out of range: %ld (%d available)\n", 
							 val, p_count->texcoords);
								break;
							}
						}
						line = end;
					}

					if (*line == '/') {
						line++;
						errno = 0;
						val = strtol(line, &end, 10);
						if (end != line && errno != ERANGE) {
							if (val < 0) {
								v.norm = p_count->normals + val;
							} else {
								v.norm = val - 1;
							}
							if (v.norm < 0 || v.norm >= p_count->normals) {
								valid_face = 0;
								printf("norm index out of range: %ld (%d available)\n", 
							 val, p_count->normals);
								break;
							}
						}
						line = end;
					}
				}

				face_verts[nverts++] = v;

				while (*line && !isspace(*line) && *line != '\n') {
					line++;
				}
			}

			if (nverts >= MAX_FACE_VERTS) {
				printf("Warning: Face with too many vertices (max %d)\n", MAX_FACE_VERTS);
			}

			if (!valid_face) {
				printf("Skipping invalid face\n");
				continue;
			}

			if (nverts < 3) {
				printf("Skipping face with less than 3 vertices\n");
				continue;
			}

			for (int j = 2; j < nverts; j++) {
				if (p_count->triangles >= parser.max_counts.triangles) {
					printf("Warning: Exceeded maximum triangle counts\n");
					break;
				}

				int idx = p_count->triangles * 3;
				p_mesh->indexed_vertices[idx]   = face_verts[0];
				p_mesh->indexed_vertices[idx+1] = face_verts[j-1];
				p_mesh->indexed_vertices[idx+2] = face_verts[j];
				p_count->triangles++;
			}
			p_count->faces++;
		}

		while (*line && *line != '\n') line++;
		if (*line == '\n') line++;
	}
}

void flatten(_mesh mesh, _counts counts, _vertex **p_vertices, u32 *p_vertices_count, u32 **p_indices, u32 *p_indices_count) {
    u32 total_indices = counts.triangles * 3;
    *p_indices_count = total_indices;

    if (total_indices == 0) {
        *p_vertices = NULL;
        *p_vertices_count = 0;
        *p_indices = NULL;
        return;
    }

    // Allocate temporary storage
    _vertex* unique_vertices = malloc(total_indices * sizeof(_vertex));
    *p_indices = malloc(total_indices * sizeof(u32));
    
    if (!unique_vertices || !*p_indices) {
        fprintf(stderr, "Memory allocation failed\n");
        free(unique_vertices);
        free(*p_indices);
        *p_indices = NULL;
        exit(EXIT_FAILURE);
    }

    // Initialize hash table
    u32 table_size = 1024;
    while (table_size < total_indices) table_size <<= 1;
    node** table = calloc(table_size, sizeof(node*));
    node* node_pool = malloc(total_indices * sizeof(node));
    
    if (!table || !node_pool) {
        fprintf(stderr, "Memory allocation failed\n");
        free(unique_vertices);
        free(*p_indices);
        free(table);
        free(node_pool);
        *p_indices = NULL;
        exit(EXIT_FAILURE);
    }

    u32 pool_index = 0;
    *p_vertices_count = 0;

    for (u32 i = 0; i < total_indices; i++) {
        _indexed_vertex iv = mesh.indexed_vertices[i];
        _vertex v = {0};

        // Fill vertex data
        if (iv.pos >= 0 && iv.pos < counts.positions) {
            v.pos[0] = (float)mesh.positions[iv.pos].x;
            v.pos[1] = (float)mesh.positions[iv.pos].y;
            v.pos[2] = (float)mesh.positions[iv.pos].z;
        }
        if (iv.tex >= 0 && iv.tex < counts.texcoords) {
            v.tex[0] = (float)mesh.texcoords[iv.tex].u;
            v.tex[1] = (float)mesh.texcoords[iv.tex].v;
        }
        if (iv.norm >= 0 && iv.norm < counts.normals) {
            v.norm[0] = (float)mesh.normals[iv.norm].x;
            v.norm[1] = (float)mesh.normals[iv.norm].y;
            v.norm[2] = (float)mesh.normals[iv.norm].z;
        }

        // Compute hash
        u32 hash = 5381;
        unsigned char* bytes = (unsigned char*)&v;
        for (size_t j = 0; j < sizeof(_vertex); j++) {
            hash = ((hash << 5) + hash) + bytes[j];
        }
        hash &= (table_size - 1);

        // Check hash table for existing vertex
        node* n = table[hash];
        u32 found_index = 0;
        int found = 0;
        while (n) {
            if (memcmp(&n->v, &v, sizeof(_vertex)) == 0) {
                found = 1;
                found_index = n->index;
                break;
            }
            n = n->next;
        }

        if (found) {
            (*p_indices)[i] = found_index;
        } else {
            // Add new vertex
            unique_vertices[*p_vertices_count] = v;
            (*p_indices)[i] = *p_vertices_count;

            // Add to hash table
            node* new_node = &node_pool[pool_index++];
            new_node->v = v;
            new_node->index = *p_vertices_count;
            new_node->next = table[hash];
            table[hash] = new_node;

            (*p_vertices_count)++;
        }
    }

    // Copy deduplicated vertices
    *p_vertices = malloc(*p_vertices_count * sizeof(_vertex));
    if (*p_vertices) {
        memcpy(*p_vertices, unique_vertices, *p_vertices_count * sizeof(_vertex));
    } else {
        *p_vertices_count = 0;
        free(*p_indices);
        *p_indices = NULL;
    }

    // Cleanup
    free(unique_vertices);
    free(table);
    free(node_pool);
}

// how to use it :D
int mainn(int argc, char** argv) {

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file.obj>\n", argv[0]);
		return 1;
	}

	char* file_data = read_obj(argv[1]);

	_parser parser = {0};
	parser.lines = preprocess(file_data, &parser.line_count);
	free(file_data);

	preparse(&parser);

	_mesh mesh = {0};
	_counts counts = {0};

	mesh.positions = malloc(sizeof(_vec3) * parser.max_counts.positions);
	mesh.texcoords = malloc(sizeof(_vec2) * parser.max_counts.texcoords);
	mesh.normals   = malloc(sizeof(_vec3) * parser.max_counts.normals);
	mesh.indexed_vertices = malloc(sizeof(_indexed_vertex) * parser.max_counts.triangles * 3);

	parse(parser, &mesh, &counts);

	mesh.positions = realloc(mesh.positions, sizeof(_vec3) * counts.positions);
	mesh.texcoords = realloc(mesh.texcoords, sizeof(_vec2) * counts.texcoords);
	mesh.normals = realloc(mesh.normals, sizeof(_vec3) * counts.normals);
	mesh.indexed_vertices = realloc(mesh.indexed_vertices, sizeof(_indexed_vertex) * counts.triangles * 3);
	free(parser.lines);

	_vertex* vertices = NULL;
	u32* indices = NULL;
	u32 vertex_count = 0;
	u32 index_count = 0;

	flatten(mesh, counts, &vertices, &vertex_count, &indices, &index_count);

	free(mesh.positions);
	free(mesh.texcoords);
	free(mesh.normals);
	free(mesh.indexed_vertices);

	free(vertices);
	free(indices);

	return 0;
}
