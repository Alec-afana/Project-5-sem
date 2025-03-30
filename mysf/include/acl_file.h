#ifndef ACL_FILE_H
#define ACL_FILE_H

#include <stddef.h>
#include <stdint.h>
#include "my_acl.h"

// Вычисляет путь к файлу ACL (acl.dat) по уникальному индексу.
// Полный путь имеет вид: "data/acl/<byte1>/<byte2>/.../<byteN>/acl.dat"
void compute_acl_path(uint64_t index, char *out_path, size_t max_len);

// Считывает данные ACL из файла по заданному пути и возвращает указатель на структуру my_acl_t.
// При ошибке возвращает NULL.
my_acl_t *read_acl_from_file(const char *file_path);

// Сериализует данные ACL из структуры my_acl_t и записывает их в файл по указанному пути.
// Возвращает 0 при успехе или отрицательный код ошибки.
int write_acl_to_file(const char *file_path, my_acl_t *acl);

#endif // ACL_FILE_H
