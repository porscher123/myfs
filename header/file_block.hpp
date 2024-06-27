#include <iostream>
#include <string>
#include <fstream>
class VFS_file_block {
    std::string DISK_FILE = "disk.txt";
    static const int BLOCK_SIZE = 1024;
    static const int BLOCK_START_OFFSET = 1051648;
public:  
    // 当前若是一级索引block
    char VFS_block[BLOCK_SIZE] = {};
    // 该block在文件系统中的起始偏移
    uint32_t VFS_offset_beg = 0; // begin offset
    // 块号
    uint32_t VFS_block_id = 0; // 
    uint32_t VFS_bg_id = 0;

public:
    // ❗❗❗❗根据id计算起始偏移
    VFS_file_block(uint32_t block_id) {
        VFS_block_id = block_id;
        VFS_bg_id = 0;
        VFS_offset_beg = BLOCK_START_OFFSET + block_id * BLOCK_SIZE;
    }

    int print()
    {
        std::cout << "VFS_bg_id: " << VFS_bg_id <<std::endl;
        std::cout << "VFS_block_id: " << VFS_block_id <<std::endl;
        std::cout << "VFS_offset_beg: " << VFS_offset_beg <<std::endl;
        std::cout << "context: " << std::endl;
        std::cout << VFS_block << std::endl;
        return 0;
    }

    /**
     * 将本block写如磁盘(文件)
     */
    int write_to_disk() {
        std::fstream disk_file;
        // 以二进制打开文件
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);

        if(!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(VFS_offset_beg);
        disk_file.write(VFS_block, BLOCK_SIZE);
        disk_file.close();
        return 0;
    }

    /**
     * 从磁盘中读取一个block
     */
    int read_from_disk() {
        std::fstream disk_file;
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        disk_file.seekp(VFS_offset_beg);
        disk_file.read(VFS_block, BLOCK_SIZE);
        disk_file.close();
        return 0;
    }

    char* get_block_pointer()
    {
        return this->VFS_block;
    }
    
    uint32_t get_VFS_block_id()
    {
        return this->VFS_block_id;
    }

    uint32_t get_VFS_offset_beg()
    {
        return this->VFS_offset_beg;
    }
    
    int reset()
    {
        std::string tempstr(1024, 0);
        tempstr.copy(VFS_block, 1024);
        return 0;
    }

    int test()
    {
        VFS_block[0] = '@';
        VFS_block[1] = 'X';
        for(int i = 2; i < 1023; ++i)
        {
            VFS_block[i] = '1';
        }
        VFS_block[1023] = 'E';
        return 0;
    }
};

// #endif