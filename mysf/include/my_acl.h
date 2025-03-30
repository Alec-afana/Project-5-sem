#ifndef MY_ACL_H
#define MY_ACL_H

#include <stdint.h>
#include <sys/types.h>

typedef enum {
    ACL_USER_OBJ,  // Права владельца файла
    ACL_USER,      // Права конкретного пользователя (user:uid)
    ACL_GROUP_OBJ, // Права группы-владельца
    ACL_GROUP,     // Права конкретной группы (group:gid)
    ACL_MASK,      // Маска для дополнительных записей
    ACL_OTHER      // Права для остальных
} acl_tag_t;

typedef struct acl_entry {
    acl_tag_t tag;  // Тип записи ACL
    uint32_t id;    // UID или GID (если применимо); для специальных записей – 0
    mode_t perms;   // Права доступа (битовая маска: 4 – чтение, 2 – запись, 1 – выполнение)
} acl_entry_t;

typedef struct my_acl {
    int count;              // Количество записей ACL
    acl_entry_t *entries;   // Динамический массив записей
} my_acl_t;

#endif // MY_ACL_H
