#ifndef ACL_CHECK_H
#define ACL_CHECK_H

#include "file_metadata.h"  // Для определения структуры file_metadata
#include <sys/types.h>      // Для определения mode_t

/**
 * Функция myfs_check_access:
 *  Проверяет, удовлетворяет ли ACL для заданного файла требуемым правам доступа.
 *
 *  Параметры:
 *    index - указатель на структуру file_metadata, содержащую метаданные файла (включая индекс)
 *    desired_mode - требуемая битовая маска прав доступа (например, выполнение = 1)
 *
 *  Возвращает:
 *    0, если доступ разрешён, или отрицательный код ошибки (например, -EACCES)
 */
int myfs_check_access(struct file_metadata *index, mode_t desired_mode);

#endif // ACL_CHECK_H
