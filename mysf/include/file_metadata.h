#ifndef FILE_METADATA_H
#define FILE_METADATA_H

#include <stdint.h>
#include <sys/types.h>

// Предварительное объявление структуры ACL
struct my_acl;

typedef struct file_metadata {
    uint64_t index;      // Уникальный идентификатор файла
    uid_t uid;           // Владелец файла
    gid_t gid;           // Группа-владелец
    mode_t mode;         // Права доступа (например, 0644)
    struct my_acl *acl;  // Указатель на собственный ACL
    // Другие поля: временные метки, размеры и т.д.
} file_metadata_t;

#endif // FILE_METADATA_H
