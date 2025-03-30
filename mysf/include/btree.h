#ifndef BTREE_H
#define BTREE_H

#include "file_metadata.h"
#include <stdbool.h>
#include <stdint.h>

// Минимальная степень B‑дерева (t). Каждый узел (кроме корня) содержит не менее t-1 ключей и не более 2*t-1.
#define MIN_DEGREE 3  
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MIN_KEYS (MIN_DEGREE - 1)

typedef struct BTreeNode {
    int num_keys;                           // Количество ключей в узле
    bool leaf;                              // Является ли узел листом
    file_metadata_t *keys[MAX_KEYS];        // Массив указателей на данные (ключи)
    struct BTreeNode *children[MAX_KEYS + 1]; // Массив указателей на дочерние узлы
} BTreeNode;

typedef struct BTree {
    BTreeNode *root;                        // Корень дерева
} BTree;

// Поиск элемента по уникальному ключу (file_id) в B‑дереве.
file_metadata_t *btree_search(BTreeNode *node, uint64_t key);

// Полноценная вставка элемента в B‑дерево (с разделением узлов).
void btree_insert(BTree *tree, file_metadata_t *data);

// Полноценное удаление элемента по ключу из B‑дерева.
int btree_delete(BTree *tree, uint64_t key);

#endif // BTREE_H
