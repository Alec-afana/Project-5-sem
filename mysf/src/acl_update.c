#include "file_metadata.h"
#include "my_acl.h"
#include "fs_ops.h"
#include <stdio.h>
#include "acl_file.h"
#include "acl_update.h"


// Функция обновления ACL: записывает новые данные в файл acl.dat
int myfs_update_acl(struct file_metadata *index, my_acl_t *new_acl) {
    char acl_path[512];
    compute_acl_path(index->index, acl_path, sizeof(acl_path));
    return write_acl_to_file(acl_path, new_acl);
}
