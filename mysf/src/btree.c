// src/btree.c
#include "btree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Forward declarations для всех вспомогательных функций удаления
static void btree_split_child(BTreeNode *parent, int i, BTreeNode *child);
static void btree_insert_nonfull(BTreeNode *node, file_metadata_t *data);
static void btree_delete_key(BTreeNode *node, uint64_t index);
static void remove_from_leaf(BTreeNode *node, int idx);
static void remove_from_nonleaf(BTreeNode *node, int idx);
static file_metadata_t *get_predecessor(BTreeNode *node, int idx);
static file_metadata_t *get_successor(BTreeNode *node, int idx);
static void borrow_from_prev(BTreeNode *node, int idx);
static void borrow_from_next(BTreeNode *node, int idx);
static void fill(BTreeNode *node, int idx);
static void merge(BTreeNode *node, int idx);

// Поиск в B‑дереве по ключу (используется поле index)
file_metadata_t *btree_search(BTreeNode *node, uint64_t index) {
    if (!node) return NULL;
    int i = 0;
    while (i < node->num_keys && node->keys[i]->index < index)
        i++;
    if (i < node->num_keys && node->keys[i]->index == index)
        return node->keys[i];
    if (node->leaf)
        return NULL;
    return btree_search(node->children[i], index);
}

// Вставка в узел, который гарантированно не заполнен
static void btree_insert_nonfull(BTreeNode *node, file_metadata_t *data) {
    int i = node->num_keys - 1;
    if (node->leaf) {
        while (i >= 0 && data->index < node->keys[i]->index) {
            node->keys[i+1] = node->keys[i];
            i--;
        }
        node->keys[i+1] = data;
        node->num_keys++;
    } else {
        while (i >= 0 && data->index < node->keys[i]->index)
            i--;
        i++;
        if (node->children[i]->num_keys == MAX_KEYS) {
            btree_split_child(node, i, node->children[i]);
            if (data->index > node->keys[i]->index)
                i++;
        }
        btree_insert_nonfull(node->children[i], data);
    }
}

// Разделение заполненного ребенка child, находящегося в родительском узле parent по индексу i
static void btree_split_child(BTreeNode *parent, int i, BTreeNode *child) {
    BTreeNode *z = malloc(sizeof(BTreeNode));
    if (!z) {
        perror("malloc");
        exit(1);
    }
    z->leaf = child->leaf;
    z->num_keys = MIN_DEGREE - 1;
    
    // Копируем последние MIN_DEGREE - 1 ключей из child в z
    for (int j = 0; j < MIN_DEGREE - 1; j++) {
        z->keys[j] = child->keys[j + MIN_DEGREE];
    }
    if (!child->leaf) {
        for (int j = 0; j < MIN_DEGREE; j++) {
            z->children[j] = child->children[j + MIN_DEGREE];
        }
    }
    child->num_keys = MIN_DEGREE - 1;
    
    // Сдвигаем дочерние указатели родителя для освобождения места для нового ребенка
    for (int j = parent->num_keys; j >= i+1; j--) {
        parent->children[j+1] = parent->children[j];
    }
    parent->children[i+1] = z;
    
    // Сдвигаем ключи родителя
    for (int j = parent->num_keys-1; j >= i; j--) {
        parent->keys[j+1] = parent->keys[j];
    }
    // Ключ, находящийся в child в позиции MIN_DEGREE - 1, переносим в родитель
    parent->keys[i] = child->keys[MIN_DEGREE - 1];
    parent->num_keys++;
}

// Вставка элемента в B‑дерево
void btree_insert(BTree *tree, file_metadata_t *data) {
    if (tree->root == NULL) {
        tree->root = malloc(sizeof(BTreeNode));
        if (!tree->root) {
            perror("malloc");
            exit(1);
        }
        tree->root->leaf = true;
        tree->root->num_keys = 0;
        tree->root->keys[0] = data;
        tree->root->num_keys = 1;
    } else {
        if (tree->root->num_keys == MAX_KEYS) {
            BTreeNode *s = malloc(sizeof(BTreeNode));
            if (!s) {
                perror("malloc");
                exit(1);
            }
            s->leaf = false;
            s->num_keys = 0;
            s->children[0] = tree->root;
            btree_split_child(s, 0, tree->root);
            int i = (data->index > s->keys[0]->index) ? 1 : 0;
            btree_insert_nonfull(s->children[i], data);
            tree->root = s;
        } else {
            btree_insert_nonfull(tree->root, data);
        }
    }
}

// ---------------------- УДАЛЕНИЕ ----------------------

// Удаление ключа из листового узла
static void remove_from_leaf(BTreeNode *node, int idx) {
    free(node->keys[idx]); // Освобождаем память для метаданных
    for (int i = idx+1; i < node->num_keys; i++) {
        node->keys[i-1] = node->keys[i];
    }
    node->num_keys--;
}

// Получение предшественника ключа
static file_metadata_t* get_predecessor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx];
    while (!cur->leaf)
        cur = cur->children[cur->num_keys];
    return cur->keys[cur->num_keys - 1];
}

// Получение преемника ключа
static file_metadata_t* get_successor(BTreeNode *node, int idx) {
    BTreeNode *cur = node->children[idx+1];
    while (!cur->leaf)
        cur = cur->children[0];
    return cur->keys[0];
}

// Удаление ключа из внутреннего (не листового) узла
static void remove_from_nonleaf(BTreeNode *node, int idx) {
    uint64_t key = node->keys[idx]->index;
    if (node->children[idx]->num_keys >= MIN_DEGREE) {
        file_metadata_t *pred = get_predecessor(node, idx);
        free(node->keys[idx]);
        node->keys[idx] = pred;
        btree_delete_key(node->children[idx], pred->index);
    } else if (node->children[idx+1]->num_keys >= MIN_DEGREE) {
        file_metadata_t *succ = get_successor(node, idx);
        free(node->keys[idx]);
        node->keys[idx] = succ;
        btree_delete_key(node->children[idx+1], succ->index);
    } else {
        merge(node, idx);
        btree_delete_key(node->children[idx], key);
    }
}

// Заимствование ключа у предыдущего соседа
static void borrow_from_prev(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx-1];
    
    // Сдвигаем ключи ребенка вправо
    for (int i = child->num_keys - 1; i >= 0; i--) {
        child->keys[i+1] = child->keys[i];
    }
    if (!child->leaf) {
        for (int i = child->num_keys; i >= 0; i--) {
            child->children[i+1] = child->children[i];
        }
    }
    child->keys[0] = node->keys[idx-1];
    if (!child->leaf)
        child->children[0] = sibling->children[sibling->num_keys];
    node->keys[idx-1] = sibling->keys[sibling->num_keys-1];
    child->num_keys++;
    sibling->num_keys--;
}

// Заимствование ключа у следующего соседа
static void borrow_from_next(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx+1];
    
    child->keys[child->num_keys] = node->keys[idx];
    if (!child->leaf)
        child->children[child->num_keys+1] = sibling->children[0];
    node->keys[idx] = sibling->keys[0];
    
    for (int i = 1; i < sibling->num_keys; i++) {
        sibling->keys[i-1] = sibling->keys[i];
    }
    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->num_keys; i++) {
            sibling->children[i-1] = sibling->children[i];
        }
    }
    child->num_keys++;
    sibling->num_keys--;
}

// Функция fill: гарантирует, что ребенок по индексу idx имеет не менее MIN_DEGREE ключей.
static void fill(BTreeNode *node, int idx) {
    if (idx != 0 && node->children[idx-1]->num_keys >= MIN_DEGREE)
        borrow_from_prev(node, idx);
    else if (idx != node->num_keys && node->children[idx+1]->num_keys >= MIN_DEGREE)
        borrow_from_next(node, idx);
    else {
        if (idx != node->num_keys)
            merge(node, idx);
        else
            merge(node, idx-1);
    }
}

// Функция merge: сливает ребенка с его следующим соседом.
static void merge(BTreeNode *node, int idx) {
    BTreeNode *child = node->children[idx];
    BTreeNode *sibling = node->children[idx+1];
    
    // Переносим ключ из родительского узла в ребенка
    child->keys[MIN_DEGREE - 1] = node->keys[idx];
    
    // Копируем ключи из sibling в child
    for (int i = 0; i < sibling->num_keys; i++) {
        child->keys[i + MIN_DEGREE] = sibling->keys[i];
    }
    if (!child->leaf) {
        for (int i = 0; i <= sibling->num_keys; i++) {
            child->children[i + MIN_DEGREE] = sibling->children[i];
        }
    }
    
    child->num_keys += sibling->num_keys + 1;
    
    for (int i = idx+1; i < node->num_keys; i++) {
        node->keys[i-1] = node->keys[i];
    }
    for (int i = idx+2; i <= node->num_keys; i++) {
        node->children[i-1] = node->children[i];
    }
    node->num_keys--;
    
    free(sibling);
}

// Рекурсивная функция удаления ключа из узла
static void btree_delete_key(BTreeNode *node, uint64_t key) {
    int idx = 0;
    while (idx < node->num_keys && node->keys[idx]->index < key)
        idx++;
    
    // Если ключ найден в этом узле
    if (idx < node->num_keys && node->keys[idx]->index == key) {
        if (node->leaf) {
            remove_from_leaf(node, idx);
        } else {
            remove_from_nonleaf(node, idx);
        }
    } else {
        if (node->leaf)
            return; // Ключ не найден
        
        if (node->children[idx]->num_keys < MIN_DEGREE)
            fill(node, idx);
        
        // После fill, если idx стало больше, корректируем
        if (idx > node->num_keys)
            btree_delete_key(node->children[idx-1], key);
        else
            btree_delete_key(node->children[idx], key);
    }
}

// Публичная функция удаления ключа из B‑дерева.
int btree_delete(BTree *tree, uint64_t key) {
    if (!tree->root)
        return -1;
    btree_delete_key(tree->root, key);
    if (tree->root->num_keys == 0) {
        BTreeNode *tmp = tree->root;
        if (!tree->root->leaf)
            tree->root = tree->root->children[0];
        else
            tree->root = NULL;
        free(tmp);
    }
    return 0;
}
