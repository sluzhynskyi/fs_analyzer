#include <iostream>
#include <boost/program_options.hpp>
#include <ext2fs/ext2_fs.h>
#include <fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include <ctime>

using std::cout;
using std::cerr;
using std::flush;
using std::endl;
using std::string;
using std::vector;

#define BASE_OFFSET 1024
int block_size;
int inode_size;
int inode_table;

int read_group_desc(int, struct ext2_group_desc *);

int print_super(int, struct ext2_super_block *);

int read_inode_from_table(int, int, int, int, struct ext2_inode *);

int print_recursive_dir(int fd, struct ext2_inode *inode, string path);

int main(int argc, char **argv) {
    int fd;
    struct ext2_super_block super{};
    struct ext2_group_desc group_desc{};
    struct ext2_inode *root_inode;
    string img = (argc < 2) ? "ext2_fs_no_MBR.img" : argv[1];
    cout << img << endl;

    if ((fd = open(img.c_str(), O_RDONLY)) < 0)
        cout << "file open failure!" << endl;

    if (print_super(fd, &super) != 0) {
        cout << "failed to print superblock" << endl;
        return EXIT_FAILURE;
    }
    read_group_desc(fd, &group_desc);
    inode_table = group_desc.bg_inode_table;

    root_inode = (struct ext2_inode *) malloc(inode_size);
    if (read_inode_from_table(fd, inode_table, inode_size, 2, root_inode) != 0) {
        cout << "failed to read root inode" << endl;
        return EXIT_FAILURE;
    }
    print_recursive_dir(fd, root_inode, "/");
    close(fd);
    exit(0);

}

int print_super(int fd, struct ext2_super_block *super) {

    int scount;
    string s;

    if ((scount = lseek(fd, BASE_OFFSET, SEEK_SET)) != BASE_OFFSET) {
        cout << "scount is " << (int) scount << endl;
        return EXIT_FAILURE;
    }

    read(fd, super, sizeof(struct ext2_super_block));

    s = (super->s_magic == EXT2_SUPER_MAGIC) ? "This is ext2" : "This is not ext2 system";
    cout << s << " magic is: " << super->s_inodes_count << endl;

    if (super->s_magic != EXT2_SUPER_MAGIC)
        return EXIT_FAILURE;
    block_size = 1024 << super->s_log_block_size;
    inode_size = super->s_inode_size;
    cout << "Inodes count                                   : " << super->s_inodes_count << endl;
    cout << "Blocks count                                   : " << super->s_blocks_count << endl;
    cout << "Reserved blocks count                          : " << super->s_r_blocks_count << endl;
    cout << "Free blocks count                              : " << super->s_free_blocks_count << endl;
    cout << "Free inodes count                              : " << super->s_free_inodes_count << endl;
    cout << "First Data    Block                            : " << super->s_first_data_block << endl;
    cout << "Block size (log2 (block size) - 10.)           : " << super->s_log_block_size << endl;
    cout << "Allocation cluster size log2(cluster size) - 10: " << super->s_log_cluster_size << endl;
    cout << "Blocks per group                               : " << super->s_blocks_per_group << endl;
    cout << "Fragments per group                            : " << super->s_clusters_per_group << endl;
    cout << "Inodes per group                               : " << super->s_inodes_per_group << endl;
    cout << "Mount time                                     : " << super->s_mtime << endl;
    cout << "Write time                                     : " << super->s_wtime << endl;
    cout << "Mount count                                    : " << super->s_mnt_count << endl;
    cout << "Maximal mount count                            : " << super->s_max_mnt_count << endl;
    cout << "Magic signature                                : " << super->s_magic << endl;
    cout << "File system state                              : " << super->s_state << endl;
    cout << "Behaviour when detecting errors                : " << super->s_errors << endl;
    cout << "minor revision level                           : " << super->s_minor_rev_level << endl;
    cout << "time of last check                             : " << super->s_lastcheck << endl;
    cout << "max. time between checks                       : " << super->s_checkinterval << endl;
    cout << "OS                                             : " << super->s_creator_os << endl;
    cout << "Revision level                                 : " << super->s_rev_level << endl;
    cout << "Default uid for reserved blocks                : " << super->s_def_resuid << endl;
    cout << "Default gid for reserved blocks                : " << super->s_def_resgid << endl;
    return 0;
}

int read_group_desc(int fd, struct ext2_group_desc *group_desc) {
    int scount;

    if ((scount = lseek(fd, block_size, SEEK_SET)) < 0) {
        cout << "scount is " << (int) scount << endl;
        return EXIT_FAILURE;
    }
    read(fd, group_desc, sizeof(struct ext2_group_desc));
    return 0;
}

int read_inode_from_table(int fd, int table, int size, int inode_numb, struct ext2_inode *inode) {
    if ((lseek(fd, block_size * table + ((inode_numb - 1) * size), SEEK_SET)) < 0) {
        return EXIT_FAILURE;
    }
    read(fd, inode, size);
    return 0;
}

int print_recursive_dir(int fd, struct ext2_inode *inode, string path) {
    char *block;
    size_t c = 0;
    if (S_ISDIR(inode->i_mode)) {
        struct ext2_dir_entry_2 *entry;
        block = static_cast<char *>(malloc(block_size));
        lseek(fd, inode->i_block[0] * block_size, SEEK_SET);
        read(fd, block, block_size);                /* read block from disk*/
        while (c < block_size) {
            entry = reinterpret_cast<struct ext2_dir_entry_2 *>(block + c);
            char file_name[EXT2_NAME_LEN + 1];
            memcpy(file_name, entry->name, entry->name_len);
            file_name[entry->name_len] = 0;
            string s_file_name = file_name;
            c += entry->rec_len;
            if (s_file_name == "." || s_file_name == "..") {
                continue;
            }
            read_inode_from_table(fd, inode_table, inode_size, entry->inode, inode);
            string file_type;
            if S_ISREG(inode->i_mode){
                file_type = "File";
            } else if S_ISDIR(inode->i_mode){
                file_type = "Dir";
            }
            time_t modification_time = inode->i_mtime;
            cout << path << file_name << endl; // File path
            cout << "File size: " << inode->i_size << endl;
            cout << "Modification time: " << ctime(&modification_time) << flush;
            cout << file_type << endl;
            cout << "Owner ID: " << inode->i_uid << endl;
            cout << "Hard links: " << inode->i_links_count << endl;
            cout << "----------------------------------------------------------" << endl;


            print_recursive_dir(fd, inode, path + file_name + "/");
        }
        free(block);
    }
    return 0;
}
