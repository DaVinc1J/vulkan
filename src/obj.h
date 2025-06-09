#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_FACE_VERTS 1024

typedef uint32_t u32;

typedef struct { double x, y, z; } _vec3;
typedef struct { double u, v;     } _vec2;
typedef struct { int pos, tex, norm;  } _indexed_vertex;

typedef struct _mesh {
	_vec3* positions;
	_vec2* texcoords;
	_vec3* normals;
	_indexed_vertex* indexed_vertices;
} _mesh;

typedef struct _counts {
	int positions, texcoords, normals, triangles, faces;
} _counts;

typedef struct _obj {
	char* lines;
	int line_count;
	int max_verts;
	_counts max_counts;
} _parser;

typedef struct {
	float pos[3];
	float tex[2];
	float norm[3];
} _vertex;

typedef struct node {
    _vertex v;
    u32 index;
    struct node* next;
} node;

char* preprocess(const char* input, int *p_line_count);
char* read_obj(const char* path);
void preparse(_parser *p_parser);
void parse(_parser parser, _mesh *p_mesh, _counts *p_count);
void flatten(_mesh mesh, _counts counts, _vertex **p_vertices, u32 *p_vertices_count, u32 **p_indices, u32 *p_indices_count);
