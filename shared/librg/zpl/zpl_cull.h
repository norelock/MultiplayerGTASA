/*

ZPL - Tree culler module

Usage:
    #define ZPLC_IMPLEMENTATION exactly in ONE source file right BEFORE including the library, like:

    #define ZPLC_IMPLEMENTATION
    #include"zpl_cull.h"

Dependencies:
    zpl.h

Credits:
  Read AUTHORS.md

GitHub:
  https://github.com/zpl-c/zpl

Version History:
    4.0.0 - New syntax
    3.1.4 - Small fixes
    3.1.3 - Fixed tests and warnings
    3.1.2 - Fixed zplc_insert bug with invalid branched nodes
    3.1.1 - Fixed zplc_insert signature
    3.1.0 - Added min_bounds checks + fixes
    3.0.1 - A lot of bug fixes.
    3.0.0 - Added the ability to remove specific node, Few utilities were added.
    2.1.2 - Small fixes for tiny cpp warnings
    2.0.0 - Slight fixes and design changes
    1.0.2 - Small oversight fixed
    1.0.1 - Fixes.
    1.0.0 - Initial version    

    This Software is dual licensed under the following licenses:

    Unlicense
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org/>

    Apache 2.0
    Copyright 2017-2018 Dominik Madarász <zaklaus@outlook.com>
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
        
        http://www.apache.org/licenses/LICENSE-2.0
        
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License. 

*/

///////////////////////////////////////////////////////////
//
// NOTE(ZaKlaus): If you use memory arena, it gets reset
// when clear() is called, that means,
// if your tree shares memory with other systems
// in the arena, the data might get corrupted!
//
// Due to the nature of how linear allocators work
// you can't free single resource of the arena and
// the only way to clear it is to completely,
// reset it. Please, consider using either
// a different allocator supporting _free_ operation
// or make sure the (sub)arena's memory is
// exclusive to the tree only.
//

#ifndef ZPL_INCLUDE_ZPL_CULL_H
#define ZPL_INCLUDE_ZPL_CULL_H

#if defined(__cplusplus)
extern "C" {
#endif

    //
    // Spatial bounding box
    //

#define zplc_bounds zplc_bounds
    typedef zplm_aabb3 zplc_bounds;


    //
    // Tree culler structure
    //

    typedef enum zplcDimensions {
        ZPLC_DIM_2D = 2,
        ZPLC_DIM_3D = 3,
    } zplcDimensions;

#define zplc_node zplc_node
    typedef struct zplc_node {
        zplm_vec3 position;

        // NOTE(ZaKlaus): Custom data to be set by user.
        void *ptr;
        u64   tag;
        b32   unused;
    } zplc_node;

#define zplc zplc
    typedef struct zplc {
        zpl_allocator            allocator;

        u32                        max_nodes;
        isize                      dimensions;
        zplc_bounds              boundary;
        zplm_vec3                min_bounds;
        b32                        use_min_bounds;
        zpl_array_t(zplc_node)   nodes;
        zpl_array_t(usize)         free_nodes;
        zpl_array_t(struct zplc) trees;
    } zplc;

    ZPL_DEF void         zplc_init       (zplc *c, zpl_allocator a, isize dims, zplc_bounds bounds, zplm_vec3 min_bounds, u32 max_nodes);
    ZPL_DEF void         zplc_query      (zplc *c, zplc_bounds bounds, zpl_array_t(zplc_node) *out_nodes);
    ZPL_DEF zplc      *zplc_insert     (zplc *c, zplc_node node);
    ZPL_DEF b32          zplc_remove     (zplc *c, u64 tag);
    ZPL_DEF zplc      *zplc_find_branch(zplc *c, u64 tag);
    ZPL_DEF void         zplc_split      (zplc *c);
    ZPL_DEF void         zplc_clear      (zplc *c);

    #define zplc_free zplc_clear


#if defined(__cplusplus)
}
#endif

#if defined(ZPLC_IMPLEMENTATION) && !defined(ZPLC_IMPLEMENTATION_DONE)
#define ZPLC_IMPLEMENTATION_DONE

#if defined(__cplusplus)
extern "C" {
#endif

    void zplc_init(zplc *c, zpl_allocator a, isize dims, zplc_bounds bounds, zplm_vec3 min_bounds, u32 max_nodes) {
        zplc c_ = {0};
        *c            = c_;
        c->allocator  = a;
        c->boundary   = bounds;
        c->min_bounds = min_bounds;
        c->use_min_bounds = zplm_vec3_mag(min_bounds) > 0.0f;
        c->max_nodes  = max_nodes;
        c->dimensions = dims;

    }

    b32 zplc__contains(isize dims, zplc_bounds a, f32 *point) {
        for (i32 i = 0; i < dims; ++i) {
            if (!( a.centre.e[i] - a.half_size.e[i] <= point[i]
                   && a.centre.e[i] + a.half_size.e[i] >= point[i])) {
                return false;
            }
        }

        return true;
    }

    b32 zplc__intersects(isize dims, zplc_bounds a, zplc_bounds b) {
        for (i32 i = 0; i < dims; ++i) {
            if (zpl_abs(a.centre.e[i] - b.centre.e[i]) > (a.half_size.e[i] + b.half_size.e[i])) return false;
        }

        return true;
    }

    void zplc_query(zplc *c, zplc_bounds bounds, zplc_node **out_nodes) {
        if (c->nodes == NULL) return;
        if (!zplc__intersects(c->dimensions, c->boundary, bounds)) return;

        isize nodes_count = zpl_array_count(c->nodes);
        for (i32 i = 0; i < nodes_count; ++i) {
            if (c->nodes[i].unused) continue;

            b32 inside = zplc__contains(c->dimensions, bounds, c->nodes[i].position.e);

            if (inside) {
                zpl_array_append(*out_nodes, c->nodes[i]);
            }

        }
        if (c->trees == NULL) return;

        isize trees_count = zpl_array_count(c->trees);
        if (trees_count == 0) return;

        for (i32 i = 0; i < trees_count; ++i) {
            zplc_query((c->trees+i), bounds, out_nodes);
        }
    }

    b32 zplc_remove(zplc *c, u64 tag) {
        if (c->nodes == NULL) return false;
        for (i32 i = 0; i < zpl_array_count(c->nodes); ++i) {
            zplc_node *node = &c->nodes[i];
            if (node->tag == tag) {
                if (node->unused) return false;
                if (c->free_nodes == NULL) {
                    zpl_array_init_reserve(c->free_nodes, c->allocator, c->max_nodes);
                }
                zpl_array_append(c->free_nodes, i);
                node->unused = true;
                return true;
            }
        }

        return false;
    }

    zplc *zplc_find_branch(zplc *c, u64 tag) {
        for (i32 i = 0; i < zpl_array_count(c->nodes); ++i) {
            if (c->nodes[i].tag == tag) {
                return c;
            }
        }

        if (c->trees == NULL) return NULL;
        isize trees_count = zpl_array_count(c->trees);
        if (trees_count == 0) return NULL;

        for (i32 i = 0; i < trees_count; ++i) {
            zplc *branch = zplc_find_branch(&c->trees[i], tag);
            if (branch) return branch;
        }

        return NULL;
    }

    b32 zplc__bounds_small_enough(zplc_bounds a, zplm_vec3 b) {
        //TODO(zaklaus): Is this the best way we can determine bounds for k-d ?
        return a.half_size.x <= b.x && a.half_size.y <= b.y && a.half_size.z <= b.z;
    }

    zplc *zplc_insert(zplc *c, zplc_node node) {
        if (!zplc__contains(c->dimensions, c->boundary, node.position.e)) return NULL;

        if (c->nodes == NULL) {
            zpl_array_init(c->nodes, c->allocator);
        }

        if (c->free_nodes && zpl_array_count(c->free_nodes) > 0) {
            node.unused = false;
            c->nodes[c->free_nodes[zpl_array_count(c->free_nodes)-1]] = node;
            zpl_array_pop(c->free_nodes);
            return c;
        }

        if ((usize)zpl_array_count(c->nodes) < c->max_nodes) {
            insert:
            zpl_array_append(c->nodes, node);
            return c;
        }

        if (c->use_min_bounds && zplc__bounds_small_enough(c->boundary, c->min_bounds)) {
            goto insert;
        }

        if (c->trees == NULL) {
            zpl_array_init(c->trees, c->allocator);
        }

        isize trees_count = zpl_array_count(c->trees);
        if (trees_count == 0) {
            zplc_split(c);
        }

        trees_count = zpl_array_count(c->trees);
        for (i32 i = 0; i < trees_count; ++i) {
            zplc *tree = zplc_insert((c->trees+i), node);
            if (tree) return tree;
        }

        return NULL;
    }

    zpl_global f32 zplc__tpl[][3] = {
        {-1.0f, -1.0f, -1.0f},
        {+1.0f, -1.0f, -1.0f},
        {-1.0f, +1.0f, -1.0f},
        {+1.0f, +1.0f, -1.0f},
        {-1.0f, -1.0f, +1.0f},
        {+1.0f, -1.0f, +1.0f},
        {-1.0f, +1.0f, +1.0f},
        {+1.0f, +1.0f, +1.0f},
    };

    void zplc_split(zplc *c) {
        zplc_bounds hd = c->boundary;
        for (i32 i = 0; i < c->dimensions; ++i) {
            hd.half_size.e[i] /= 2.0f;
        }

        i32 loops = 0;
        /**/ if (c->dimensions == ZPLC_DIM_2D) {
            loops = 4;
        }
        else if (c->dimensions == ZPLC_DIM_3D) {
            loops = 8;
        }

        f32 p[3] = {0};
        for (i32 i = 0; i < loops; ++i) {
            zplc tree = {0};
            zplc_bounds bounds = {0};
            p[0] = c->boundary.centre.e[0] + hd.half_size.e[0]*zplc__tpl[i][0];
            p[1] = c->boundary.centre.e[1] + hd.half_size.e[1]*zplc__tpl[i][1];
            p[2] = c->boundary.centre.e[2] + hd.half_size.e[2]*zplc__tpl[i][2];
            zpl_memcopy(bounds.centre.e, p, 3 * sizeof(f32));
            bounds.half_size = hd.half_size;

            tree.boundary   = bounds;
            tree.min_bounds = c->min_bounds;
            tree.use_min_bounds = c->use_min_bounds;
            tree.max_nodes  = c->max_nodes;
            tree.dimensions = c->dimensions;
            tree.allocator  = c->allocator;

            zpl_array_append(c->trees, tree);
        }
    }

    void zplc_clear(zplc *c) {
        // TODO(ZaKlaus): Support more allocators?
        if (c->allocator.proc == zpl_arena_allocator_proc) {
            zpl_free_all(c->allocator);
            c->nodes = NULL;
            c->trees = NULL;
            return;
        }

        if (c->nodes) {
            zpl_array_free(c->nodes);
            c->nodes = NULL;
        }

        if (!c->trees) return;
        isize trees_count = zpl_array_count(c->trees);
        for (i32 i = 0; i < trees_count; ++i) {
            zplc_clear((c->trees+i));
        }

        zpl_array_free(c->trees);
        if (c->free_nodes) zpl_array_free(c->free_nodes);
        c->trees = NULL;
        c->free_nodes = NULL;
    }

#if defined(__cplusplus)
}
#endif

#endif // ZPLC_IMPLEMENTATION

#endif // ZPL_INCLUDE_ZPL_CULL_H
