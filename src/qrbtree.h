
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _QRBTREE_H__
#define _QRBTREE_H__

typedef unsigned int  qrbtree_key_t;
typedef int           qrbtree_key_int_t;


typedef struct qrbtree_node_s  qrbtree_node_t;

struct qrbtree_node_s {
    qrbtree_key_t       key;
    qrbtree_node_t     *left;
    qrbtree_node_t     *right;
    qrbtree_node_t     *parent;
    unsigned char                 color;
    unsigned char                 data;
};


typedef struct qrbtree_s  qrbtree_t;

typedef void (*qrbtree_insert_pt) (qrbtree_node_t *root,
    qrbtree_node_t *node, qrbtree_node_t *sentinel);

struct qrbtree_s {
    qrbtree_node_t     *root;
    qrbtree_node_t     *sentinel;
    qrbtree_insert_pt   insert;
};


#define qrbtree_init(tree, s, i)                                           \
    qrbtree_sentinel_init(s);                                              \
    (tree)->root = s;                                                         \
    (tree)->sentinel = s;                                                     \
    (tree)->insert = i


void qrbtree_insert( qrbtree_t *tree,
    qrbtree_node_t *node);
void qrbtree_delete( qrbtree_t *tree,
    qrbtree_node_t *node);
void qrbtree_insert_value(qrbtree_node_t *root, qrbtree_node_t *node,
    qrbtree_node_t *sentinel);
void qrbtree_insert_timer_value(qrbtree_node_t *root,
    qrbtree_node_t *node, qrbtree_node_t *sentinel);


#define ngx_rbt_red(node)               ((node)->color = 1)
#define ngx_rbt_black(node)             ((node)->color = 0)
#define ngx_rbt_is_red(node)            ((node)->color)
#define ngx_rbt_is_black(node)          (!ngx_rbt_is_red(node))
#define ngx_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define qrbtree_sentinel_init(node)  ngx_rbt_black(node)

static qrbtree_node_t *
qrbtree_min(qrbtree_node_t *node, qrbtree_node_t *sentinel)
{
    while (node->left != sentinel) {
        node = node->left;
    }

    return node;
}


#endif  /*  _QRBTREE_H__ */
