#include "acl_file.h"
#include "fs_ops.h"  // Для custom_mkdir, custom_read_file, custom_write_file
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACL_ROOT "/acl"

/**
 * compute_acl_path:
 * Вычисляет путь к файлу ACL по уникальному индексу.
 * Формирует путь вида:
 *   "data/acl/<byte0>/<byte1>/.../<byte7>/acl.dat"
 * где каждый байт представляется в виде двухсимвольного шестнадцатеричного числа.
 * Если какого-либо каталога не существует, вызывается custom_mkdir для его создания.
 */
void compute_acl_path(uint64_t index, char *out_path, size_t max_len) {
    unsigned char key[8];
    for (int i = 0; i < 8; i++) {
        key[i] = (index >> (i * 8)) & 0xFF;
    }
    // Начинаем с корневого каталога FS и специального каталога для ACL
    snprintf(out_path, max_len, "data%s", ACL_ROOT);
    custom_mkdir(out_path);
    char part[8];
    // Для каждого байта добавляем подкаталог, например, "/f0"
    for (int i = 0; i < 8; i++) {
        snprintf(part, sizeof(part), "/%02x", key[i]);
        strncat(out_path, part, max_len - strlen(out_path) - 1);
        // Создаем каталог, если его ещё нет
        custom_mkdir(out_path);
    }
    // Добавляем имя файла, где хранятся данные ACL
    strncat(out_path, "/acl.dat", max_len - strlen(out_path) - 1);
}

/**
 * read_acl_from_file:
 * Считывает данные из файла, который должен содержать:
 * 1. Заголовок: 4 байта (int) — количество записей ACL.
 * 2. Затем подряд count записей типа acl_entry_t.
 * Возвращает указатель на динамически выделенную структуру my_acl_t или NULL при ошибке.
 */
my_acl_t *read_acl_from_file(const char *file_path) {
    size_t file_size = 0;
    char *buffer = custom_read_file(file_path, &file_size);
    if (!buffer) {
        fprintf(stderr, "Ошибка чтения файла %s\n", file_path);
        return NULL;
    }
    if (file_size < sizeof(int)) {
        fprintf(stderr, "Файл %s повреждён: недостаточный размер\n", file_path);
        free(buffer);
        return NULL;
    }
    int count = 0;
    memcpy(&count, buffer, sizeof(int));
    if (count < 0 || file_size < sizeof(int) + count * sizeof(acl_entry_t)) {
        fprintf(stderr, "Файл %s повреждён: неверное количество записей\n", file_path);
        free(buffer);
        return NULL;
    }
    my_acl_t *acl = malloc(sizeof(my_acl_t));
    if (!acl) {
        free(buffer);
        return NULL;
    }
    acl->count = count;
    acl->entries = malloc(count * sizeof(acl_entry_t));
    if (!acl->entries) {
        free(acl);
        free(buffer);
        return NULL;
    }
    memcpy(acl->entries, buffer + sizeof(int), count * sizeof(acl_entry_t));
    free(buffer);
    return acl;
}

/**
 * write_acl_to_file:
 * Сериализует структуру my_acl_t в бинарный формат:
 * - 4 байта: количество записей (int)
 * - Затем подряд записи acl_entry_t.
 * Записывает полученный буфер в файл по указанному пути.
 * Возвращает 0 при успехе или отрицательный код ошибки.
 */
int write_acl_to_file(const char *file_path, my_acl_t *acl) {
    if (!acl || acl->count < 0)
        return -1;
    size_t buf_size = sizeof(int) + acl->count * sizeof(acl_entry_t);
    char *buffer = malloc(buf_size);
    if (!buffer)
        return -1;
    memcpy(buffer, &acl->count, sizeof(int));
    memcpy(buffer + sizeof(int), acl->entries, acl->count * sizeof(acl_entry_t));
    int ret = custom_write_file(file_path, buffer, buf_size);
    free(buffer);
    return ret;
}
