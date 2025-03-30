// src/main.c
#include <stdlib.h>
#include <stdio.h>
#include "scale_tests.h"

// Глобальное B-дерево для метаданных
#include "btree.h"
BTree global_tree = {0};

int main(int argc, char *argv[]) {
    // Инициализируем глобальное B-дерево (корень пуст)
    global_tree.root = malloc(sizeof(BTreeNode));
    if (!global_tree.root) {
        perror("malloc");
        return 1;
    }
    global_tree.root->num_keys = 0;
    global_tree.root->leaf = true;

    int num_files = 100; // Значение по умолчанию

    if (argc > 1) {
        num_files = atoi(argv[1]);  // Получаем количество файлов из аргумента
    }

    printf("Запуск масштабных тестов...\n");
    run_scale_tests(num_files);

    // Освобождаем глобальное B-дерево
    // Предполагается, что B-дерево содержит все метаданные, освобождаем его целиком.
    // (Эту функцию нужно реализовать, как показано в scale_tests.c)
    extern void free_btree(BTreeNode *node);
    free_btree(global_tree.root);

    return 0;
}
