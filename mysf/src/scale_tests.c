// src/scale_tests.c
#include "file_metadata.h"
#include "btree.h"
#include "my_acl.h"
#include "fs_ops.h"
#include "acl_check.h"
#include "acl_update.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// Функция для замера времени в наносекундах
uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Реализация освобождения B‑дерева (рекурсивно)
void free_btree(BTreeNode *node) {
    if (!node) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_btree(node->children[i]);
        }
    }
    // Освобождаем ключи (каждый file_metadata_t)
    for (int i = 0; i < node->num_keys; i++) {
        free(node->keys[i]);
    }
    free(node);
}

// Функция масштабного теста: создает N файлов с метаданными, обновляет для них ACL и создает объемные файлы.
void run_scale_tests(int num_files) {
    if (num_files <= 0) {
        num_files = 100; // Значение по умолчанию
    }
    printf("\nЗапуск масштабного теста для %d файлов...\n", num_files);

    // Создаем каталог для пользовательских файлов
    if (custom_mkdir("data/userfiles") != 0) {
        printf("Ошибка создания каталога data/userfiles\n");
        return;
    }

    uint64_t t_start = get_time_ns();

    // Предполагаем, что глобальное дерево уже проинициализировано (например, из main.c)
    extern BTree global_tree;

    for (int i = 0; i < num_files; i++) {
        // Генерируем уникальный индекс; в данном примере базовое значение + i.
        uint64_t index = 0x10000000ULL + i;

        // Создаем метаданные для файла
        file_metadata_t *meta = malloc(sizeof(file_metadata_t));
        if (!meta) {
            perror("malloc meta");
            continue;
        }
        meta->index = index;
        meta->uid = 1000;
        meta->gid = 1000;
        meta->mode = 0644;
        meta->acl = NULL;

        // Вставляем объект в глобальное B‑дерево.
        btree_insert(&global_tree, meta);

        // Создаем тестовый ACL: 2 записи – для владельца и для остальных.
        my_acl_t *acl = malloc(sizeof(my_acl_t));
        if (!acl) {
            free(meta);
            continue;
        }
        acl->count = 2;
        acl->entries = malloc(2 * sizeof(acl_entry_t));
        if (!acl->entries) {
            free(acl);
            free(meta);
            continue;
        }
        acl->entries[0].tag = ACL_USER_OBJ;
        acl->entries[0].id = 0;
        acl->entries[0].perms = 7;  // rwx
        acl->entries[1].tag = ACL_OTHER;
        acl->entries[1].id = 0;
        acl->entries[1].perms = 5;  // r-x

        // Обновляем ACL для файла (записываем данные в соответствующий файл acl.dat)
        if (myfs_update_acl(meta, acl) != 0) {
            printf("Ошибка обновления ACL для файла с индексом %lu\n", index);
        }
        free(acl->entries);
        free(acl);

        // Создаем объемный пользовательский файл (например, 1 МБ данных)
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "data/userfiles/file_%lu.dat", index);
        if (custom_create_file(file_path) != 0) {
            printf("Ошибка создания файла %s\n", file_path);
        } else {
            size_t size = 1024 * 1024;
            char *buffer = malloc(size);
            if (buffer) {
                memset(buffer, 'A', size);
                if (custom_write_file(file_path, buffer, size) != 0) {
                    printf("Ошибка записи файла %s\n", file_path);
                }
                free(buffer);
            } else {
                perror("malloc buffer");
            }
        }
    }
    uint64_t t_end = get_time_ns();
    printf("Масштабный тест для %d файлов завершен. Общее время: %lu нс\n", num_files, t_end - t_start);
}

