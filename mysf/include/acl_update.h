#ifndef ACL_UPDATE_H
#define ACL_UPDATE_H

#include "file_metadata.h"  // Для структуры file_metadata
#include "my_acl.h"         // Для структуры my_acl

/**
 * Функция myfs_update_acl:
 *  Обновляет (записывает) ACL для файла, представленного структурой file_metadata.
 *
 *  Параметры:
 *    index    - указатель на структуру file_metadata, для которой обновляется ACL
 *    new_acl  - указатель на новую структуру my_acl, содержащую обновленные ACL
 *
 *  Возвращает:
 *    0 при успешном обновлении или отрицательный код ошибки при неудаче.
 */
int myfs_update_acl(struct file_metadata *index, my_acl_t *new_acl);

#endif // ACL_UPDATE_H
