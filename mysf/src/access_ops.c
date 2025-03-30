#include "file_metadata.h"
#include "btree.h"
#include <errno.h>
#include <string.h>

// Чтение прав доступа по inode из B‑дерева
int read_file_access_rights(BTree *btree, uint64_t index_key, file_metadata_t *result) {
    if (!btree || !btree->root || !result)
        return -EINVAL;
    file_metadata_t *meta = btree_search(btree->root, index_key);
    if (!meta)
        return -ENOENT;
    memcpy(result, meta, sizeof(file_metadata_t));
    return 0;
}

// Обновление прав доступа
int update_file_access_rights(BTree *btree, uint64_t index_key, uid_t new_uid, gid_t new_gid, mode_t new_mode) {
    file_metadata_t *meta = btree_search(btree->root, index_key);
    if (!meta)
        return -ENOENT;
    meta->uid  = new_uid;
    meta->gid  = new_gid;
    meta->mode = new_mode;
    return 0;
}
