#include <iostream>
#include <boost/program_options.hpp>
#include<ext2fs/ext2_fs.h>
#include<fcntl.h>

using std::cout;
using std::cerr;
using std::flush;
using std::endl;
using std::string;
using std::vector;

#define BASE_OFFSET 1024
int block_size;

int print_super(int fd, struct ext2_super_block *super) {

    int scount, rcount;
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

    cout << " Inodes count " << super->s_inodes_count << endl;
    cout << " Blocks count " << super->s_blocks_count << endl;
    cout << " Reserved blocks count " << super->s_r_blocks_count << endl;
    cout << " Free blocks count " << super->s_free_blocks_count << endl;
    cout << " Free inodes count " << super->s_free_inodes_count << endl;
    cout << " First Data    Block " << super->s_first_data_block << endl;
    cout << " Block size (log2 (block size) - 10.)" << super->s_log_block_size << endl;
    cout << " Allocation cluster size log2 (cluster size) - 10 " << super->s_log_cluster_size << endl;
    cout << " # Blocks per group " << super->s_blocks_per_group << endl;
    cout << " # Fragments per group " << super->s_clusters_per_group << endl;
    cout << " # Inodes per group " << super->s_inodes_per_group << endl;
    cout << " Mount time " << super->s_mtime << endl;
    cout << " Write time " << super->s_wtime << endl;
    cout << " Mount count " << super->s_mnt_count << endl;
    cout << " Maximal mount count " << super->s_max_mnt_count << endl;
    cout << " Magic signature " << super->s_magic << endl;
    cout << " File system state " << super->s_state << endl;
    cout << " Behaviour when detecting errors " << super->s_errors << endl;
    cout << " minor revision level " << super->s_minor_rev_level << endl;
    cout << " time of last check " << super->s_lastcheck << endl;
    cout << " max. time between checks " << super->s_checkinterval << endl;
    cout << " OS " << super->s_creator_os << endl;
    cout << " Revision level " << super->s_rev_level << endl;
    cout << " Default uid for reserved blocks " << super->s_def_resuid << endl;
    cout << " Default gid for reserved blocks " << super->s_def_resgid << endl;

    return 0;
}

int main(int argc, char **argv) {
    string img = (argc < 2) ? "ext2_fs_no_MBR.img" : argv[1];
    cout << img << endl;
    struct ext2_super_block super{};
    int fd;
    if ((fd = open(img.c_str(), O_RDONLY)) < 0)
        cout << "file open failure!" << endl;

    print_super(fd, &super);


}
