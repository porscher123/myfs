#ifndef _BLOCK_BITMAP_HPP_
#define _BLOCK_BITMAP_HPP_

#include <iostream>
#include <fstream>

class Block_bitmap
{
    const std::string DISK_FILE = "disk.txt";
public:
    // 8K 个block, 每个block 1K
    const static int BLOCK_N = 1024 * 8;
    const static int BYTE_N = 1024;
private:
    // 最大支持1024 * 8个block
    char bitmap_byte[BYTE_N] = {};    // 1024 * 8 bit

    uint32_t VFS_bg_id = 0;
    uint32_t VFS_offset_beg = 0;
    uint8_t bitmap[BLOCK_N] = {};  // 1024 * 8 ¸ö uint8

public:
    Block_bitmap()
    {
        VFS_bg_id = 0;
        VFS_offset_beg = 1024; // Block 1, Ð´ËÀ
    }


    /**
     * 将block bitmap写回到磁盘中的文件系统
     */
    int write_to_disk() {
        for (int i = 0; i < 1024; ++i) {
            bitmap_byte[i] = 0;
            for (int j = 0; j < 8; ++j) {
                if(bitmap[i * 8 + j]) {
                    // 1000 0000 
                    bitmap_byte[i] += 0x80 >> j;
                }
            }
        }
        // 打开磁盘文件
        std::fstream disk_file;
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if (!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        
        disk_file.seekp(VFS_offset_beg);
        for (uint32_t i = 0; i < BYTE_N; ++i) {
            disk_file.write(bitmap_byte + i, 1);
        }

        disk_file.close();
        return 0;
    }

    /**
     * 从磁盘文件中读取block bitmap
     */
    int read_to_VFS() {
        std::fstream disk_file;
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if (!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        // 文件指针移动到block bitmap的起始
        disk_file.seekp(VFS_offset_beg);
        // 读取到 bitmap_byte[] 中
        for (uint32_t i = 0; i < 1024; ++i) {
            disk_file.read(bitmap_byte + i, 1);
        }
        // 将bitmap_byte[]中的数据以bit为单位拷到bitmap[]
        for (int i = 0; i < 1024; ++i) {
            for (int j = 0; j < 8; ++j) {
                // check 第 i 个字节中的第j位是否是 1
                if ((bitmap_byte[i] << j) & 0x80) {
                    bitmap[i * 8 + j] = 1;
                }
            }
        }
        disk_file.close();
        return 0;
    }

    int print()
    {
        std::cout << "--- block bitmap ---" << std::endl;
        std::cout << "VFS_offset_beg" << VFS_offset_beg << std::endl;
        return 0;
    }
    /**
     * 遍历block bitmap, 查找第一个空闲的block
     * @return 第一个空闲的block id
     */
    uint32_t get_next_free_blockId() {
        for (uint32_t i = 0; i < BLOCK_N; ++i) {
            if (bitmap[i] == 0) {
                return i;
            }
        }
        return 0;
    }

    int set_block(uint32_t i)
    {
        bitmap[i] = 1;
        return 0;
    }
    
    int reset_block(uint32_t i)
    {
        bitmap[i] = 0;
        return 0;
    }

    int test()
    {
        bitmap_byte[0] = 1;
        bitmap[8] = 1;
        bitmap[1024*8-2] = 1;
        bitmap[1024*8-1] = 1;
        return 0;
    }
};

#endif