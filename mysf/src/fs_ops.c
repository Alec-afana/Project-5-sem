#include "fs_ops.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Корневой каталог нашей эмулированной FS
#define FS_ROOT "data"

// Создание каталога. Если каталог уже существует, возвращаем -EEXIST.
int custom_mkdir(const char *path) {
    // Формируем полный путь относительно FS_ROOT, если он ещё не начинается с FS_ROOT
    char full_path[512];
    if (strncmp(path, FS_ROOT, strlen(FS_ROOT)) != 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", FS_ROOT, path);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
	full_path[sizeof(full_path) - 1] = '\0';
    }
    //printf("Создаем каталог: %s\n", full_path);
    int ret = mkdir(full_path, 0755);
    if (ret == -1 && errno != EEXIST) {
        perror("mkdir");
        return -errno;
    }
    return 0;
}

// Создание файла. Если файл уже существует, возвращаем -EEXIST.
int custom_create_file(const char *path) {
    char full_path[512];
    if (strncmp(path, FS_ROOT, strlen(FS_ROOT)) != 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", FS_ROOT, path);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
	full_path[sizeof(full_path) - 1] = '\0';
    }
    //printf("Создаем файл: %s\n", full_path);
    FILE *fp = fopen(full_path, "w+b");  // Открываем в режиме записи с усечением
    if (!fp) {
        perror("fopen");
        return -errno;
    }
    fclose(fp);
    return 0;
}

// Запись данных в файл
int custom_write_file(const char *path, const char *data, size_t len) {
    char full_path[512];
    if (strncmp(path, FS_ROOT, strlen(FS_ROOT)) != 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", FS_ROOT, path);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
	full_path[sizeof(full_path) - 1] = '\0';
    }
    FILE *fp = fopen(full_path, "w+b");
    if (!fp) {
        perror("fopen");
        return -errno;
    }
    size_t written = fwrite(data, 1, len, fp);
    fclose(fp);
    if (written != len)
        return -EIO;
    return 0;
}

// Чтение файла. Выделяется буфер, который должен быть освобожден вызывающей стороной.
char *custom_read_file(const char *path, size_t *out_size) {
    char full_path[512];
    if (strncmp(path, FS_ROOT, strlen(FS_ROOT)) != 0) {
        snprintf(full_path, sizeof(full_path), "%s/%s", FS_ROOT, path);
    } else {
        strncpy(full_path, path, sizeof(full_path) - 1);
	full_path[sizeof(full_path) - 1] = '\0';
    }
    FILE *fp = fopen(full_path, "rb");
    if (!fp) {
        perror("fopen");
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *buffer = malloc(size);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }
    size_t read = fread(buffer, 1, size, fp);
    fclose(fp);
    if (read != size) {
        free(buffer);
        return NULL;
    }
    *out_size = size;
    return buffer;
}
