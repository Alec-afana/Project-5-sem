#ifndef FS_OPS_H
#define FS_OPS_H

#include <stddef.h>  // Добавляем для определения size_t

// Абстракция над операциями с файловой системой (эмуляция FS через host-OS)
int custom_mkdir(const char *path);             // Создание каталога
int custom_create_file(const char *path);       // Создание файла
int custom_write_file(const char *path, const char *data, size_t len); // Запись данных в файл
char *custom_read_file(const char *path, size_t *out_size);            // Чтение файла в буфер

#endif // FS_OPS_H
