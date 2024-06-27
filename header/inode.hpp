/*
 * @Author: your name
 * @Date: 2020-12-08 19:20:55
 * @LastEditTime: 2020-12-08 22:43:59
 * @LastEditors: Please set LastEditors
 * @Description: Inode class for VFS
 * @FilePath: /code/inode.hpp
 */

#ifndef _INODE_HPP_
#define _INODE_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "./file_block.hpp"
#include "./superblock.hpp"

class Inode
{
// 要保存到磁盘的数据
public:
    uint16_t    i_mode = 0;        // 文件类型和 RWX 等权限描述
    uint16_t    i_uid = 0;         // 用户目录 id
    uint32_t    i_atime = 0;       // inode 上一次被访问时间 unix_time
    uint32_t    i_ctime = 0;       // inode 创建时间
    uint32_t    i_mtime = 0;       // inode 上一次 modified 时间
    uint32_t    i_dtime = 0;       // inode 上一次 删除时间
    std::string i_name;            // inode 对应文件的名字
    
    uint32_t VFS_offset_beg = 0;    // Inode 的起始地址
    uint32_t VFS_inode_id = 0;      // Inode 的 ID
    uint32_t VFS_ingroup_id = 0;    // Inode 在 Inode Table 里面的对应位置
    uint32_t indirect_block = 0; // 一级索引
    std::vector<VFS_file_block> block_table;    // Inode 文件 所有对应 Block 的 Table
    std::vector<uint16_t> children_num;

// 内存中使用的数据
public:
    std::string DISK_FILE = "disk.txt";
public:
    // 用于切换目录和列出文件
    // 😍😍😍
    std::vector<Inode*> children;
    Inode* parent;
    // 文件类型
    const static uint16_t FILE = 0x0001;
    const static uint16_t DIR = 0x0002;
    // 文件权限
    const static uint16_t X = 0x0100;
    const static uint16_t W = 0x0200;
    const static uint16_t R = 0x0400;

    // inode list 在 vfs中的起始偏移
    const static uint32_t INODES_START_OFFSET = 3072;
    // 一个inode 128Byte
    const static uint32_t INODE_SIZE = 128;


    bool isFile() {
        return this->i_mode & FILE;
    }
    bool isDir() {
        return this->i_mode & DIR;
    }
    void set_mode(uint16_t mode) {
        this->i_mode |= mode;
    }


    void clear() {
        std::fstream disk_file;
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        disk_file.seekp(INODES_START_OFFSET);
        uint8_t temp = 0;
        disk_file.write((char*)&temp, 127 * 1024 * 8);
    }
    /**
     * 构造函数
     */
    Inode() {}
    Inode(uint16_t inode_id, std::string str_name = "", uint16_t uid = 0) {
        i_uid = uid;    // 默认为用户 0
        i_name = str_name;
        VFS_inode_id = inode_id;
        VFS_ingroup_id = inode_id;
        //😍😍😍 根据inode id 初始化 这个inode 在文件系统中的偏移
        VFS_offset_beg = INODES_START_OFFSET + (inode_id) * INODE_SIZE;
        this->children_num = std::vector<uint16_t>();
    }

    void add_child(Inode* child) {
        this->children.push_back(child);
    }
    /**
     * 写文件
     */
    int write_to_disk()
    {
        std::fstream disk_file;
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good())
        {
            std::cout << "disk_file is not exist" << std::endl;
        }
        // 从当前inode的起始位置开始写
        disk_file.seekp(VFS_offset_beg);
        // 写indoe信息
        disk_file.write((char*)&i_mode, 2);
        disk_file.write((char*)&i_uid, 2);
        disk_file.write((char*)&i_atime, 4);
        disk_file.write((char*)&i_ctime, 4);
        disk_file.write((char*)&i_mtime, 4);
        disk_file.write((char*)&i_dtime, 4);
        disk_file.write((char*)&indirect_block, 4);
        // 最多有1024 * 8个inode, 所以需要 13 bit 保存inode编号
        // 即每个inode的编号需要2个字节存储
        for (int i = 0; i < 16; i++) {
            if (i < children_num.size()) {
                disk_file.write((char*)&children_num[i], 2);
            } else {
                uint16_t temp = 0;
                disk_file.write((char*)&temp, 2);
            }
        }
        char temp_c_str[71] = {};
        disk_file.write(temp_c_str, 71);
        disk_file.seekp(-71, std::ios::cur);
        disk_file.write(i_name.c_str(), i_name.size() < 71 ? i_name.size() : 71);
        disk_file.close();
        // 将inode 对应的blocks 也写到磁盘
        for(int i = 0; i < block_table.size(); ++i) {
            block_table[i].write_to_disk();
        }
        VFS_file_block temp_block(indirect_block);
        char* temp_block_p = temp_block.get_block_pointer();
        for(uint32_t i = 0; i < block_table.size(); ++i) {
            *(uint32_t*)temp_block_p = (uint32_t)block_table[i].get_VFS_block_id();
            temp_block_p += 4;
        }
        temp_block.write_to_disk();

        return 0;
    }
    

    /**
     * 从用户的文件中读取
     */
    int read_from_disk()
    {
        std::fstream disk_file;
        // 以二进制的形式读取文件
        disk_file.open(DISK_FILE, std::ios::binary | std::ios::out | std::ios::in);
        if(!disk_file.good()) {
            std::cout << "disk_file is not exist" << std::endl;
        }
        // 文件指针移动到inode的位置
        disk_file.seekp(VFS_offset_beg);
        // 读取inode信息
        disk_file.read((char*)&i_mode, 2);
        disk_file.read((char*)&i_uid, 2);
        disk_file.read((char*)&i_atime, 4);
        disk_file.read((char*)&i_ctime, 4);
        disk_file.read((char*)&i_mtime, 4);
        disk_file.read((char*)&i_dtime, 4);
        disk_file.read((char*)&indirect_block, 4);
        // 😍😍😍新增16 * 2字节保存inode的所有儿子结点编号
        children_num.clear();
        for (int i = 0; i < 16; i++) {
            uint16_t temp = 0;
            disk_file.read((char*)&temp, 2);
            if (temp != 0) {
                children_num.push_back(temp);
            }
        }

        // 读取文件名
        char temp_c_str[71] = {};
        disk_file.read(temp_c_str, 71);
        i_name = std::string(temp_c_str);

        disk_file.close();
        return 0;
    }


    /**
     * 将所有 Block 读到 Block table 中
     */
    int load_blocks() {
        VFS_file_block temp_block(indirect_block);
        uint32_t temp_block_id = 0;
        temp_block.read_from_disk();

        char* temp_block_p = temp_block.get_block_pointer();
        while ((uint32_t)*temp_block_p != 0) {
            block_table.push_back(VFS_file_block((uint32_t)*temp_block_p));
            block_table.back().read_from_disk();
            temp_block_p += 4;
        }
        return 0;
    }



    char* trans_pointer(uint32_t file_offset)   // 将文件偏移量转换为对应 Block 内的指针
    {
        if(file_offset > block_table.size() * 1024)
        {
            std::cout << "FILE OFFSET OUT OF INDEX!" << std::endl;
        }
        return block_table[file_offset / 1024].get_block_pointer() + file_offset % 1024;
    }


    /**
     * 从文件的 Block 中读取内容
     */
    int read(char* output, uint32_t file_offset ,uint32_t size) {
        for (uint32_t i = 0; i < size; ++i) {
            char temp_byte = *trans_pointer(file_offset + i);
            *(output + i) = temp_byte;
        }
        return 0;
    }


    /**
     * 将数据的写入的 Block 中
     */
    int write(char* input, uint32_t file_offset ,uint32_t size) {
        for(uint32_t i = 0; i < size; ++i) {
            *trans_pointer(file_offset + i) = *(input+i);
        }
        return 0;
    }


    // 将对应 block 重置，并添加到 block table 中
    int add_block(uint32_t block_id) {
        block_table.push_back(VFS_file_block(block_id));
        return 0;
    }



    int truncate()
    {
        block_table = std::vector<VFS_file_block>();

        VFS_file_block temp_block(indirect_block);
        char* temp_block_p = temp_block.get_block_pointer();
        for(uint32_t i = 0; i < 1024; ++i)
        {
            *(temp_block_p+i) = 0;
        }
        return 0;
    }



    bool is_null()
    {
        return i_name == "";
    }



    uint32_t get_VFS_inode_id()
    {
        return this->VFS_inode_id;
    }



    uint32_t get_indirect_block()
    {
        return this->indirect_block;
    }
    

    uint32_t get_used_blocks_count() {
        return block_table.size();
    }


    /**
     * 设置间接块
     */
    int set_indirect_block(uint32_t block_id, std::string disk_file_path) {
        this->indirect_block = block_id;
        VFS_file_block temp_block(block_id);
        temp_block.reset();
        temp_block.write_to_disk();
        return 0;
    }



    std::string get_i_name()
    {
        return this->i_name;
    }



    int set_i_name(std::string name_str)
    {
        this->i_name = name_str;
        return 0;
    }



    uint16_t get_uid()
    {
        return this->i_uid;
    }



    int set_uid(uint16_t uid)
    {
        this->i_uid = uid;
        return 0;
    }



    int print()
    {
        std::cout << "i_name: " << i_name.c_str() << std::endl;
        std::cout << "VFS_inode_id: " << VFS_inode_id << std::endl;
        std::cout << "VFS_blocks_count: " << block_table.size() << std::endl;
        std::cout << "VFS_ingroup_id: " << VFS_ingroup_id  << std::endl;
        std::cout << "VFS_offset_beg: " << VFS_offset_beg << std::endl;
        std::cout << "indirect_block :" << indirect_block << std::endl; 
        return 0;
    }


    /**
     * 当需要删除文件时，获取一串即将被释放的Block Number
     */
    std::vector<uint32_t> get_delete_blocks_ids()
    {
        std::vector<uint32_t> temp_vec;
        for(uint32_t i = 0; i < block_table.size(); ++i) {
            temp_vec.push_back(block_table[i].get_VFS_block_id());
        }
        temp_vec.push_back(indirect_block);
        return temp_vec;
    }



    // 当需要截断文件时，获取一串即将被释放的Block Number
    std::vector<uint32_t> get_truncate_block_ids()
    {
        std::vector<uint32_t> temp_vec;
        for(uint32_t i = 0; i < block_table.size(); ++i)
        {
            temp_vec.push_back(block_table[i].get_VFS_block_id());
        }
        return temp_vec;
    }




    /**
     * 打印inode信息
     */
    int ls_i_print() {
        std::string mode;
        if (this->isDir()) mode += "d";
        else mode += "_";
        if (this->i_mode & R) mode += "r";
        else mode += "_";
        if (this->i_mode & W) mode += "w";
        else mode += "_";
        if (this->i_mode & X) mode += "x";
        else mode += "_";
        std::cout << std::left << std::setw(10) << mode
                  << std::left << std::setw(10) << VFS_inode_id
                  << std::left << std::setw(10) << block_table.size() * 1024 
                  << std::left << std::setw(10) << i_name
                  << std::left << std::setw(10) << block_table.size();
        std::setw(10);
        std::cout << "[ ";
        for(int i = 0; i < block_table.size(); ++i) {
            std::cout << block_table[i].get_VFS_block_id() << ", ";
        }
        std::cout << "]";
        std::cout << std::endl;
        return 0;
    }
};

#endif