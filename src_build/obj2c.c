#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include "vec.h"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#include "flag.h"

typedef struct {
    int *items;
    size_t capacity;
    size_t count;
} Face_Indices;

typedef struct {
    int *items;
    size_t capacity;
    size_t count;
} Vertex_Indices;

typedef struct {
    Vector3 position;
    Face_Indices faces;
    int component;              // 0 means never visited, >0 is the index of the component vertex belongs to
} Vertex;

Vertex make_vertex(float x, float y, float z)
{
    return (Vertex) {
        .position = make_vector3(x, y, z),
    };
}

typedef struct {
    Vertex *items;
    size_t capacity;
    size_t count;
} Vertices;

#define VERTICES_PER_FACE 3

typedef struct {
    int v[VERTICES_PER_FACE];
    int vt[VERTICES_PER_FACE];
    int vn[VERTICES_PER_FACE];
} Face;

Face make_face(int v1, int v2, int v3, int vt1, int vt2, int vt3, int vn1, int vn2, int vn3)
{
    static_assert(VERTICES_PER_FACE == 3, "");
    Face f = {
        .v  = {v1, v2, v3},
        .vt = {vt1, vt2, vt3},
        .vn = {vn1, vn2, vn3},
    };
    return f;
}

typedef struct {
    Face *items;
    size_t capacity;
    size_t count;
} Faces;

typedef struct {
    Vector3 *items;
    size_t capacity;
    size_t count;
} Normals;

typedef struct {
    Vector2 *items;
    size_t capacity;
    size_t count;
} TexCoords;

typedef struct {
    int *items;
    size_t count;
    size_t capacity;
} Component_Indices;

bool is_deleted_face(Vertices vertices, Face face, Component_Indices delete_components)
{
    for (size_t i = 0; i < VERTICES_PER_FACE; ++i) {
        for (size_t j = 0; j < delete_components.count; ++j) {
            if (vertices.items[face.v[i]].component == delete_components.items[j]) {
                return true;
            }
        }
    }
    return false;
}

void generate_code(FILE *out, Vertices vertices, TexCoords texcoords, Normals normals, Faces faces, Component_Indices delete_components, const char *name)
{
    fprintf(out, "#ifndef OBJ_H_\n");
    fprintf(out, "#define OBJ_H_\n");
    fprintf(out, "#define %s_vertices_count %zu\n", name, vertices.count);
    if (vertices.count == 0) {
        fprintf(out, "static const float %s_vertices[1][3] = {0};\n", name);
    } else {
        fprintf(out, "static const float %s_vertices[][3] = {\n", name);
        for (size_t i = 0; i < vertices.count; ++i) {
            Vector3 v = vertices.items[i].position;
            fprintf(out, "    {%f, %f, %f},\n", v.x, v.y, v.z);
        }
        fprintf(out, "};\n");
    }

    fprintf(out, "#define %s_texcoords_count %zu\n", name, texcoords.count);
    if (texcoords.count == 0) {
        fprintf(out, "static const float %s_texcoords[1][2] = {0};\n", name);
    } else {
        fprintf(out, "static const float %s_texcoords[][2] = {\n", name);
        for (size_t i = 0; i < texcoords.count; ++i) {
            Vector2 vt = texcoords.items[i];
            fprintf(out, "    {%f, %f},\n", vt.x, vt.y);
        }
        fprintf(out, "};\n");
    }

    fprintf(out, "#define %s_normals_count %zu\n", name, normals.count);
    if (normals.count == 0) {
        fprintf(out, "static const float %s_normals[1][3] = {0};\n", name);
    } else {
        fprintf(out, "static const float %s_normals[][3] = {\n", name);
        for (size_t i = 0; i < normals.count; ++i) {
            Vector3 vn = normals.items[i];
            fprintf(out, "    {%f, %f, %f},\n", vn.x, vn.y, vn.z);
        }
        fprintf(out, "};\n");
    }

    size_t visible_faces_count = 0;
    for (size_t i = 0; i < faces.count; ++i) {
        if (!is_deleted_face(vertices, faces.items[i], delete_components)) {
            visible_faces_count += 1;
        }
    }

    fprintf(out, "#define %s_faces_count %zu\n", name, visible_faces_count);
    if (visible_faces_count == 0) {
        fprintf(out, "static const int %s_faces[1][9] = {0};\n", name);
    } else {
        fprintf(out, "static const int %s_faces[%zu][9] = {\n", name, visible_faces_count);
        for (size_t i = 0; i < faces.count; ++i) {
            if (!is_deleted_face(vertices, faces.items[i], delete_components)) {
                Face f = faces.items[i];
                fprintf(out, "    {%d, %d, %d, %d, %d, %d, %d, %d, %d},\n", f.v[0], f.v[1], f.v[2], f.vt[0], f.vt[1], f.vt[2], f.vn[0], f.vn[1], f.vn[2]);
            }
        }
        fprintf(out, "};\n");
    }
    fprintf(out, "#endif // OBJ_H_\n");
}

Vector3 remap_object(Vector3 v, float scale, float lx, float hx, float ly, float hy, float lz, float hz)
{
    float cx = lx + (hx - lx)/2;
    float cy = ly + (hy - ly)/2;
    float cz = lz + (hz - lz)/2;
    v.z = (v.z - cz)*scale;
    v.x = (v.x - cx)*scale;
    v.y = (v.y - cy)*scale;
    return v;
}

void parse_face_triple(String_View *line, int *lf, int *hf, int *v, int *vt, int *vn)
{
    char *endptr;

    *line = sv_trim_left(*line);
    *v = strtol(line->data, &endptr, 10) - 1; // 1‑based -> 0‑based
    if (*lf > *v) *lf = *v;
    if (*hf < *v) *hf = *v;
    sv_chop_left(line, endptr - line->data);
    *vt = 0;
    if (line->count > 0 && line->data[0] == '/') {
        sv_chop_left(line, 1);
        *vt = strtol(line->data, &endptr, 10) - 1;
        sv_chop_left(line, endptr - line->data);
    }
    *vn = 0;
    if (line->count > 0 && line->data[0] == '/') {
        sv_chop_left(line, 1);
        *vn = strtol(line->data, &endptr, 10) - 1;
        sv_chop_left(line, endptr - line->data);
    }
    while (line->count > 0 && !isspace(*line->data)) sv_chop_left(line, 1);
}

int unvisited_vertex(Vertices vertices)
{
    for (size_t i = 0; i < vertices.count; ++i) {
        if (!vertices.items[i].component) {
            return (int)i;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    int result = 0;

    char **input_path  = flag_str("i", NULL, "Input .obj file path (MANDATORY)");
    char **output_path = flag_str("o", NULL, "Output .h file path (MANDATORY)");
    char **name        = flag_str("n", NULL, "Name of the model (MANDATORY)");
    char **scale_str   = flag_str("s", "0.75", "Scale factor for the model (default: 0.75)");
    char **delete_str  = flag_str("d", NULL, "Comma-separated list of component indices to delete, e.g. \"1,3,5\"");
    bool *help         = flag_bool("help", true, "Print this help message");

    if (!flag_parse(argc, argv)) {
        fprintf(stderr, "Error parsing command line arguments.\n");
        flag_print_error(stderr);
        return 1;
    }

    if (*help) {
        fprintf(stderr, "Usage: %s [OPTIONS]\n", flag_program_name());
        fprintf(stderr, "Convert OBJ file to C header with vertex, normal and face arrays.\n\n");
        fprintf(stderr, "OPTIONS:\n");
        flag_print_options(stderr);
        // return 0;
    }

    if (*input_path == NULL) {
        fprintf(stderr, "ERROR: missing required option -i <input.obj>\n");
        return 1;
    }
    if (*output_path == NULL) {
        fprintf(stderr, "ERROR: missing required option -o <output.h>\n");
        return 1;
    }
    if (*name == NULL) {
        fprintf(stderr, "ERROR: missing required option -n <name>\n");
        return 1;
    }

    float scale = strtof(*scale_str, NULL);
    Component_Indices delete_components = {0};

    if (*delete_str != NULL) {
        String_View sv = sv_from_cstr(*delete_str);
        while (sv.count > 0) {
            String_View num_sv = sv_chop_by_delim(&sv, ',');
            if (num_sv.count == 0) continue;
            char *endptr;
            long val = strtol(num_sv.data, &endptr, 10);
            if (endptr == num_sv.data) {
                fprintf(stderr, "Warning: invalid number in delete list: '%.*s'\n", (int)num_sv.count, num_sv.data);
            } else {
                da_append(&delete_components, (int)val);
            }
        }
    }

    String_Builder buffer = {0};
    if (!read_entire_file(*input_path, &buffer)) return 1;

    String_View content = sb_to_sv(buffer);
    Vertices vertices = {0};
    TexCoords texcoords = {0};
    Normals normals = {0};
    Faces faces = {0};
    float lx = FLT_MAX, hx = FLT_MIN;
    float ly = FLT_MAX, hy = FLT_MIN;
    float lz = FLT_MAX, hz = FLT_MIN;
    int lf = INT_MAX, hf = INT_MIN;
    bool one_object_encountered = false;
    size_t one_object_line_number = 0;

    for (size_t line_number = 1; content.count > 0; ++line_number) {
        String_View line = sv_trim_left(sv_chop_by_delim(&content, '\n'));
        if (line.count > 0 && *line.data != '#') {
            String_View kind = sv_chop_by_delim(&line, ' ');
            if (sv_eq(kind, sv_from_cstr("v"))) {
                char *endptr;
                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                if (lx > x) lx = x;
                if (hx < x) hx = x;
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                if (ly > y) ly = y;
                if (hy < y) hy = y;
                sv_chop_left(&line, endptr - line.data);

                line = sv_trim_left(line);
                float z = strtof(line.data, &endptr);
                if (lz > z) lz = z;
                if (hz < z) hz = z;
                sv_chop_left(&line, endptr - line.data);

                da_append(&vertices, make_vertex(x, y, z));
            } else if (sv_eq(kind, sv_from_cstr("f"))) {
                int v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3;
                int face_index = faces.count;

                parse_face_triple(&line, &lf, &hf, &v1, &vt1, &vn1);
                da_append(&vertices.items[v1].faces, face_index);

                parse_face_triple(&line, &lf, &hf, &v2, &vt2, &vn2);
                da_append(&vertices.items[v2].faces, face_index);

                parse_face_triple(&line, &lf, &hf, &v3, &vt3, &vn3);
                da_append(&vertices.items[v3].faces, face_index);

                da_append(&faces, make_face(v1, v2, v3, vt1, vt2, vt3, vn1, vn2, vn3));
            } else if (sv_eq(kind, sv_from_cstr("mtllib"))) {
                fprintf(stderr, "%s:%zu: WARNING: mtllib not supported, ignoring.\n", *input_path, line_number);
            } else if (sv_eq(kind, sv_from_cstr("usemtl"))) {
                fprintf(stderr, "%s:%zu: WARNING: usemtl not supported, ignoring.\n", *input_path, line_number);
            } else if (sv_eq(kind, sv_from_cstr("o"))) {
                if (one_object_encountered) {
                    fprintf(stderr, "%s:%zu: ERROR: Only one object per file supported (previous object at line %zu).\n", *input_path, line_number, one_object_line_number);
                    return 1;
                }
                line = sv_trim_left(line);
                String_View obj_name = line;
                fprintf(stderr, "%s:%zu: INFO: processing object `"SV_Fmt"`\n", *input_path, line_number, SV_Arg(obj_name));
                one_object_encountered = true;
                one_object_line_number = line_number;
            } else if (sv_eq(kind, sv_from_cstr("s"))) {
                fprintf(stderr, "%s:%zu: WARNING: smooth groups not supported, ignoring.\n", *input_path, line_number);
            } else if (sv_eq(kind, sv_from_cstr("vn"))) {
                char *endptr;
                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);
                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);
                line = sv_trim_left(line);
                float z = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);
                da_append(&normals, make_vector3(x, y, z));
            } else if (sv_eq(kind, sv_from_cstr("vt"))) {
                char *endptr;
                line = sv_trim_left(line);
                float x = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);
                line = sv_trim_left(line);
                float y = strtof(line.data, &endptr);
                sv_chop_left(&line, endptr - line.data);
                da_append(&texcoords, make_vector2(x, y));
            } else {
                fprintf(stderr, "%s:%zu: ERROR: unknown keyword `"SV_Fmt"`\n", *input_path, line_number, SV_Arg(kind));
                return 1;
            }
        }
    }

    int min_faces = INT_MAX, max_faces = INT_MIN;
    for (size_t i = 0; i < vertices.count; ++i) {
        int cnt = (int)vertices.items[i].faces.count;
        if (min_faces > cnt) min_faces = cnt;
        if (max_faces < cnt) max_faces = cnt;
    }

    size_t comp_count = 0;
    int start = unvisited_vertex(vertices);
    while (start >= 0) {
        comp_count += 1;
        Vertex_Indices wave = {0}, next_wave = {0};
        da_append(&wave, start);
        vertices.items[start].component = comp_count;
        while (wave.count > 0) {
            for (size_t i = 0; i < wave.count; ++i) {
                Vertex *v = &vertices.items[wave.items[i]];
                for (size_t j = 0; j < v->faces.count; ++j) {
                    for (size_t k = 0; k < VERTICES_PER_FACE; ++k) {
                        int nbr = faces.items[v->faces.items[j]].v[k];
                        if (!vertices.items[nbr].component) {
                            da_append(&next_wave, nbr);
                            vertices.items[nbr].component = comp_count;
                        }
                    }
                }
            }
            wave.count = 0;
            Vertex_Indices tmp = wave;
            wave = next_wave;
            next_wave = tmp;
        }
        start = unvisited_vertex(vertices);
    }

    printf("Input:               %s\n", *input_path);
    printf("Output:              %s\n", *output_path);
    printf("Vertices:            %zu (x: %f..%f, y: %f..%f, z: %f..%f)\n", vertices.count, lx, hx, ly, hy, lz, hz);
    printf("Normals:             %zu\n", normals.count);
    printf("Texture Coordinates: %zu\n", texcoords.count);
    printf("Faces:               %zu (index: %d..%d)\n", faces.count, lf, hf);
    printf("Faces per vertex:    %d..%d\n", min_faces, max_faces);
    printf("Components Count:    %zu\n", comp_count);
    printf("Deleted Components:  ");
    for (size_t i = 0; i < delete_components.count; ++i) {
        printf("%d ", delete_components.items[i]);
    }
    printf("\n");

    for (size_t i = 0; i < vertices.count; ++i) {
        vertices.items[i].position = remap_object(vertices.items[i].position, scale, lx, hx, ly, hy, lz, hz);
    }

    FILE *out = fopen(*output_path, "wb");
    if (out == NULL) {
        fprintf(stderr, "ERROR: could not open %s for writing: %s\n", *output_path, strerror(errno));
        return 1;
    }
    generate_code(out, vertices, texcoords, normals, faces, delete_components, *name);
    fclose(out);

    nob_log(INFO, "generated %s", *output_path);
    return 0;
}
