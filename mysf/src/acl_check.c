#include "file_metadata.h"
#include "my_acl.h"
#include "fs_ops.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "acl_file.h"
#include "acl_check.h"


// Простейшая реализация проверки ACL
int perform_acl_check(my_acl_t *acl, uid_t user, mode_t desired_mode) {
    for (int i = 0; i < acl->count; i++) {
        if (acl->entries[i].tag == ACL_USER_OBJ) {
            if ((acl->entries[i].perms & desired_mode) == desired_mode)
                return 0;
        }
    }
    return -EACCES;
}

// Функция проверки доступа, использующая файл ACL
int myfs_check_access(struct file_metadata *index, mode_t desired_mode) {
    char acl_path[512];
    compute_acl_path(index->index, acl_path, sizeof(acl_path));
    my_acl_t *acl = read_acl_from_file(acl_path);
    if (!acl)
        return -ENOENT;
    int ret = perform_acl_check(acl, index->uid, desired_mode);
    free(acl->entries);
    free(acl);
    return ret;
}
