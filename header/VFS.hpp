/*
 * @Author: your name
 * @Date: 2020-12-07 18:33:43
 * @LastEditTime: 2020-12-07 21:44:57
 * @LastEditors: Please set LastEditors
 * @Description: The VFS main header
 * @FilePath: /code/VFS.hpp
 */
#ifndef _VFS_HPP_
#define _VFS_HPP_

#define FIND_FALSE 0xffffffff

#include "./superblock.hpp"
#include "./block_bitmap.hpp"
#include "./inode_bitmap.hpp"
#include "./inode.hpp"
// #include "./file_block.hpp"

#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <string>
#include <vector>

class VFS
{
public:
    Superblock          superblock;     // superblock 常驻内存
    Block_bitmap        block_bitmap;   // block_bitmap 常驻内存
    Inode_bitmap        inode_bitmap;   // inode_bitmap 常驻内存
    std::vector<Inode>  inode_table;    // inode_table  常驻内存
    Inode *root;
    Inode *cur_inode;
    uint16_t             VFS_login_uid = 0;      // 登录进文件系统的 uid
    uint16_t             VFS_working_uid = 0;    // 文件系统工作的 uid，用于实现多用户共享

public:
    const std::string DISK_FILE = "disk.txt";
    std::string cur_dir = "/";
    std::string username;
public:
    VFS()
        : superblock(Superblock()), block_bitmap(Block_bitmap()), inode_bitmap(Inode_bitmap()), inode_table(std::vector<Inode>())
    {
        this->root == nullptr;
        VFS_login_uid = 0;      // 默认登入 0 号用户
        VFS_working_uid = 0;    // 默认cd 到 0 号目录

    }


    /**
     * 文件系统初始化(格式化)
     * 
     */
    int format() {
        printf("formatting...\n");
        // superblock 初始化
        superblock.initialize();
        superblock.write_to_disk();
        superblock.read_to_VFS();
        superblock.print();
        std::cout << std::endl;

        // block_bitmap 初始化
        block_bitmap.write_to_disk();
        block_bitmap.print();
        std::cout << std::endl;

        // inode_bitmap 初始化
        inode_bitmap.write_to_disk();
        inode_bitmap.print();
        std::cout << std::endl;
        // 将inode列表清空
        Inode inode;
        inode.clear();

        // 加载根目录和root用户目录
        this->root = new Inode(0);
        this->root->i_name = "/";
        this->inode_bitmap.set_inode(0);
        this->cur_inode = root;
        
        Inode *user = mkdir("root");

        this->root->write_to_disk();
        user->write_to_disk();
        this->inode_bitmap.write_to_disk();
        this->block_bitmap.write_to_disk();
        this->superblock.write_to_disk();
        std::cout << "format finish!\n";
        
        Inode *test = new Inode(0);
        test->read_from_disk();
        
        for (auto &cn : test->children_num) {
            std::cout << cn << ", ";
        }
        return 0;   
    }

    void clearTree(Inode* root) {
        if (root == nullptr) return;
        for (auto &child : root->children) {
            clearTree(child);
        }
        root->children.clear();
        delete root;
    }
    /**
     * 从disk.txt中加载文件系统到内存
     * 加载除了block的其它所有内容
     */
    int load_from_file() {
        
        // superblock 初始化
        superblock.read_to_VFS();
        
        // block_bitmap 初始化
        block_bitmap.read_to_VFS();
        
        // inode_bitmap 初始化
        inode_bitmap.read_to_VFS(DISK_FILE);

        // 读取 '/' 对应的inode, 再内存中建立 inode tree
        init_tree();
        return 0;        
    }
    void init_tree() {
        // 😍😍😍读取第0个inode, 作为根结点
        this->root = new Inode(0);
        this->root->read_from_disk();
        this->cur_inode = root;
        init(this->root);
        this->root->write_to_disk();
        this->superblock.write_to_disk();
        this->inode_bitmap.write_to_disk();
        this->block_bitmap.write_to_disk();
    }
    /**
     * 读取磁盘中的children_num
     * 在内存中建立树形结构
     */
    void init(Inode *root) {
        if (root == nullptr) return;
        root->read_from_disk();
        if (!root->is_null()) {
            root->load_blocks();  
        } 
        for (auto &child_num : root->children_num) {
            
            Inode *child = new Inode(child_num);
            child->read_from_disk();
            child->parent = root;
            root->add_child(child);
            init(child);
        }
    }

    void save(Inode *root) {
        for (auto &child : root->children) {
            save(child);
        }
        root->write_to_disk();
    }
    /**
     * 添加新用户
     * 创建一个名位用户名的inode, 挂到根目录上
     */
    void add_user(std::string username) {
        uint32_t newInodeId = inode_bitmap.get_next_free_inode();
        Inode *user = new Inode(newInodeId);
        user->set_mode(Inode::DIR | Inode::R | Inode::W | Inode::X);
        // Inode *newUser = mkdir()
        user->parent = root;
        user->i_name = username;
        this->root->add_child(user);
        this->root->children_num.push_back(newInodeId);


        inode_bitmap.write_to_disk();
        this->root->write_to_disk();
        user->write_to_disk();
    } 

    void login(std::string username) {
        for (auto &child : this->cur_inode->children) {
            if (child->i_name == username) {
                this->cur_inode = child;
                break;
            }
        }
    }

    std::string get_current_dir() {
        std::string pwd;
        if (this->cur_inode == this->root) {
            pwd = "/";
        }
        for (Inode *p = this->cur_inode; p != this->root; p = p->parent) {
            pwd = "/" + p->i_name + pwd;
        }
        return pwd;
    }
    /**
     * 打印文件系统的信息
     */
    void info() {
        superblock.print();
        std::cout << std::endl;
        block_bitmap.print();
        std::cout << std::endl;
        inode_bitmap.print();
        std::cout << std::endl;
    }




    Inode* findInode(std::string path) {
        if (path.rfind("./", 0) == 0) {

        } else if (path.rfind("../", 0) == 0) {

        } else if (path.rfind("/", 0) == 0) {
            // 
            std::string absPath = path;
            int pos = absPath.find_first_of('/');
        } else { // 在当前目录下找
            assert(this->cur_inode != nullptr);
            for (auto &child : this->cur_inode->children) {
                if (child->i_name == path) {
                    return child;
                }
            }
        }
        return nullptr;
    }

    /**
     * 创建目录
     */
    Inode* mkdir(std::string filename, uint32_t size_of_block = 1) {
        // std::cout << "create file: " + str_name << std::endl;

        // 检查是否有空闲的inode
        if(superblock.get_free_inode_count() == 0) {
            std::cout << "No free inode!" << std::endl;
            return 0;
        }
        // 剩余block不足以满足申请
        if(size_of_block > superblock.get_free_blocks_count()) {
            std::cout << "No enough blocks!" << std::endl;
            return 0;
        }
        if(size_of_block > 1024 * 4) {
            std::cout << "File size limitedbreak!" << std::endl;
            return 0;
        }
        // 一个目录最多创建16个文件
        if (this->cur_inode->children_num.size() == 16) {
            std::cout << "content is filled!" << std::endl;
            return 0;
        }
        // 在当前目录下查找是否已经有该文件
        for (auto &child : this->cur_inode->children) {
            if (child->i_name == filename) {
                std::cout << "file exists!" << std::endl;
                return 0;
            }
        }

        // 😍😍😍😍😍关于创建目录
        uint32_t inode_id = inode_bitmap.get_next_free_inode();
        Inode *inode_dir = new Inode(inode_id, filename);
        this->cur_inode->add_child(inode_dir);
        this->cur_inode->children_num.push_back(inode_dir->VFS_inode_id);
        inode_dir->parent = this->cur_inode;
        inode_dir->set_mode(Inode::DIR | Inode::R | Inode::W | Inode::X);



        inode_bitmap.set_inode(inode_id);
        std::cout << "use inode: " << inode_id << "; ";
        
        // 选取一级索引的 block, 并将文件中该 block reset
        std::cout << "use blocks: ";
        uint32_t indirect_block = block_bitmap.get_next_free_blockId();
        // inode_table[inode_id].set_indirect_block(indirect_block, DISK_FILE);
        inode_dir->set_indirect_block(indirect_block, DISK_FILE);

        // block_bitmap 标记
        block_bitmap.set_block(indirect_block);
        std::cout << indirect_block << " ";

        // 选取存放数据的 block，并每次都在内存中重置该 block，最后和 inode 一起写回
        for(uint32_t i = 0; i < size_of_block; ++i)
        {
            uint32_t temp_id = block_bitmap.get_next_free_blockId();
            // 添加 block
            // inode_table[inode_id].add_block(temp_id);
            inode_dir->add_block(temp_id);
            // block_bitmap 标记
            block_bitmap.set_block(temp_id);
            std::cout << temp_id << " ";
        }
        std::cout << std::endl;

        // superblock 对应标记更新
        superblock.set_free_blocks_count(superblock.get_free_blocks_count() - 1 - size_of_block);
        superblock.set_free_inode_count(superblock.get_free_inode_count() - 1);

        // 将涉及改动写回文件
        inode_bitmap.write_to_disk();
        block_bitmap.write_to_disk();
        inode_dir->write_to_disk();
        this->cur_inode->write_to_disk();
        this->root->write_to_disk();
        superblock.write_to_disk();
        save(this->root);
        return inode_dir;
    }

    

    /**
     * 创建文件函数
     */
    int create(std::string filename, uint32_t size_of_block = 1) {

        // 一个目录最多创建16个文件
        if (this->cur_inode->children_num.size() == 16) {
            std::cout << "content is filled!" << std::endl;
            return 0;
        }
        // 检查是否有空闲的inode
        if(superblock.get_free_inode_count() == 0) {
            std::cout << "No free inode!" << std::endl;
            return 0;
        }
        // 剩余block不足以满足申请
        if(size_of_block > superblock.get_free_blocks_count()) {
            std::cout << "No enough blocks!" << std::endl;
            return 0;
        }
        if(size_of_block > 1024 * 4) {
            std::cout << "File size limitedbreak!" << std::endl;
            return 0;
        }
        // 在当前目录下查找是否已经有该文件
        for (auto &child : this->cur_inode->children) {
            if (child->i_name == filename) {
                std::cout << "file exists!" << std::endl;
                return 0;
            }
        }


    
        // 选取空闲 inode
        uint32_t inode_id = inode_bitmap.get_next_free_inode();
        // 创建文件inode结点
        Inode *file_inode = new Inode(inode_id);

        this->cur_inode->add_child(file_inode);
        this->cur_inode->children_num.push_back(inode_id);
        this->cur_inode->write_to_disk();
        file_inode->parent = this->cur_inode;
        file_inode->set_mode(Inode::FILE | Inode::R | Inode::W);
        file_inode->i_name = filename;


        inode_bitmap.set_inode(inode_id);
        std::cout << "use inode: " << inode_id << "; ";
        
        // 选取一级索引的 block, 并将文件中该 block reset
        std::cout << "use blocks: ";
        uint32_t indirect_block = block_bitmap.get_next_free_blockId();
        // inode_table[inode_id].set_indirect_block(indirect_block, DISK_FILE);
        file_inode->set_indirect_block(indirect_block, DISK_FILE);
        // block_bitmap 标记
        block_bitmap.set_block(indirect_block);
        std::cout << indirect_block << " ";

        // 选取存放数据的 block，并每次都在内存中重置该 block，最后和 inode 一起写回
        for (uint32_t i = 0; i < size_of_block; ++i) {
            uint32_t temp_id = block_bitmap.get_next_free_blockId();
            // 添加 block
            file_inode->add_block(temp_id);
            // block_bitmap 标记
            block_bitmap.set_block(temp_id);
            std::cout << temp_id << " ";
        }
        std::cout << std::endl;

        // superblock 对应标记更新
        superblock.set_free_blocks_count(superblock.get_free_blocks_count() - 1 - size_of_block);
        superblock.set_free_inode_count(superblock.get_free_inode_count() - 1);

        // 将涉及改动写回文件
        inode_bitmap.write_to_disk();
        block_bitmap.write_to_disk();
        file_inode->write_to_disk();
        this->cur_inode->write_to_disk();
        superblock.write_to_disk();
        this->root->write_to_disk();
        save(this->root);
        return 0;
    }




    /**
     * 删除文件函数
     */
    int remove(std::string filename) {


        // uint32_t inode_id = find(filename);
        Inode *rmInode = findInode(filename);
        if (rmInode == nullptr) {
            return 0;
        }
        uint32_t inode_id = rmInode->VFS_inode_id;

        auto &v = this->cur_inode->children_num;
        for (int i = 0; i < v.size(); i++) {
            if (v[i] == inode_id) {
                v.erase(v.begin() + i);
            }
        }
        
        auto &p = this->cur_inode->children;
        for (int i = 0; i < p.size(); i++) {
            if (p[i] == rmInode) {
                p.erase(p.begin() + i);
            }
        }
        // 获取需要释放的 block 列表
        std::vector<uint32_t> free_blocks = rmInode->get_delete_blocks_ids();

        // 释放 block
        for(uint32_t i = 0; i < free_blocks.size(); ++i) {
            // 修改 block_bitmap 标记
            block_bitmap.reset_block(free_blocks[i]);
        }

        // 释放 inode
        inode_bitmap.reset_inode(inode_id);

        // 修改 inode bitmap 标记
        rmInode = new Inode(inode_id);

        // superblock 对应记录修改
        superblock.set_free_blocks_count(superblock.get_free_blocks_count() + free_blocks.size());
        superblock.set_free_inode_count(superblock.get_free_inode_count() + 1);

        // 将文件系统内容写会磁盘
        block_bitmap.write_to_disk();
        inode_bitmap.write_to_disk();
        rmInode->write_to_disk();
        superblock.write_to_disk();

        std::cout << "release block count: " << free_blocks.size() << std::endl;
        std::cout << "release inode id: " << inode_id << std::endl;
        return 0;
    }





    // 截断文件函数
    int truncate(std::string filename) {
        std::cout << "truncate file: " + filename << std::endl;
        Inode* inode = findInode(filename);
        if (inode == nullptr) {
            return 0;
        }

        // 获取需要释放的 block 列表
        std::vector<uint32_t> free_blocks = inode->get_truncate_block_ids();
        // 释放 block
        for(uint32_t i = 0; i < free_blocks.size(); ++i) {
            block_bitmap.reset_block(free_blocks[i]);
        }

        inode->truncate();

        superblock.set_free_blocks_count(superblock.get_free_blocks_count() + free_blocks.size());

        block_bitmap.write_to_disk();
        inode->write_to_disk();
        superblock.write_to_disk();

        std::cout << "release block count: " << free_blocks.size() << std::endl;
        return 0;
    }





    // 扩大文件函数
    int increase(std::string str_name, uint32_t inc_count) {

        std::cout << "increase file: " + str_name << std::endl;

        if(inc_count > superblock.get_free_blocks_count())
        {
            std::cout << "NOT ENOUGH BLOCKS!" << std::endl; 
            return 0;           
        }
        Inode *inode = findInode(str_name);
        if (inode == nullptr) {
            return 0;
        }
        if (inode->get_used_blocks_count() + inc_count > 256) {
            std::cout << "FILE SIZE LIMITEDBREAK!" << std::endl;
            return 0;
        }

        // 选取存放数据的 block
        std::cout << "use blocks: ";
        for(uint32_t i = 0; i < inc_count; ++i)
        {
            uint32_t temp_id = block_bitmap.get_next_free_blockId();
            inode->add_block(temp_id);
            block_bitmap.set_block(temp_id);
            std::cout << temp_id << " ";
        }
        std::cout << std::endl;

        block_bitmap.write_to_disk();
        inode->write_to_disk();

        superblock.set_free_blocks_count(superblock.get_free_blocks_count() - inc_count);

        return 0;
    }

    /**
     * 切换目录
     * 当前仅支持相邻目录的切换
     */
    void cd(std::string filename) {
        if (filename == ".") return;
        if (filename == ".." && this->cur_inode == this->root) return;
        else if (filename == ".." && this->cur_inode != this->root) {
            this->cur_inode = this->cur_inode->parent;
            return;
        }
        for (auto &child : this->cur_inode->children) {
            if (child->i_name == filename && child->isFile()) {
                std::cout << "can't go into a file.\n"; 
                return;
            } else if (child->i_name == filename) {
                this->cur_inode = child;
                return;
            }
        }
        std::cout << "directory not found!\n";
    }

    /**
     * 文件重命名
     * 找到文件对应的inode
     */
    int rename(std::string old_filename, std::string new_filename) {
        Inode *inode = findInode(old_filename);
        if (inode == nullptr) {
            return 0;
        }
        inode->set_i_name(new_filename);
        return 0;
    }




    /**
     * 写文件函数，使用随机位置指针
     */
    int write(std::string filename, char* input, uint32_t file_offset, uint32_t size) {
        Inode *inode = findInode(filename);
        if (inode == nullptr) {
            return 0;
        }
        if (file_offset + size > inode->get_truncate_block_ids().size() * 1024) {
            std::cout << "OFFSET OUT OF INDEX!" << std::endl;
            return 0;
        }
        inode->write(input, file_offset, size);
        inode->write_to_disk();
        return 0;
    }





    // 读文件函数，随机指针
    /**
     * 
     */
    int read(std::string filename, char* ouput, uint32_t file_offset ,uint32_t size) {
        Inode *inode = findInode(filename);
        if (inode == nullptr) {
            return 0;
        }
        if(file_offset + size > inode->get_truncate_block_ids().size() * 1024) {
            std::cout << "OFFSET OUT OF INDEX!" << std::endl;
            return 0;
        }
        inode->read(ouput, file_offset, size);
        inode->write_to_disk();
        return 0;
    }





    // 显示指定文件的信息函数
    // int ls(std::string str_name) {
    //     // uint32_t inode_id = find(str_name);
    //     // if(inode_id == FIND_FALSE) {
    //     //     return 0;
    //     // }
    //     // inode_table[inode_id].ls_print();
    //     return 0;
    // }


    int ls() {
        std::cout << std::left << std::setw(10) << "mode" 
                  << std::left << std::setw(10) << "inode id" 
                  << std::left << std::setw(10) << "size"
                  << std::left << std::setw(10) << "filename"
                  << "blocks\n";  
        for (auto &child_inode : this->cur_inode->children) {
            child_inode->ls_i_print();
        }
        return 0;
    }


    /**
     * 展示当前工作目录下的所有文件
     */
    int ls_i() {
        // std::cout << "free inodes count: " << superblock.get_free_inode_count() << std::endl;
        // std::cout << "free blocks count: " << superblock.get_free_blocks_count() << std::endl;
        std::cout << std::left << std::setw(10) << "mode" 
                  << std::left << std::setw(10) << "inode id" 
                  << std::left << std::setw(10) << "size"
                  << std::left << std::setw(10) << "filename"
                  << "blocks\n";  
        for(uint32_t i = 0; i < inode_table.size(); ++i) {
            if(!inode_table[i].is_null() && inode_table[i].get_uid() == VFS_working_uid) {
                inode_table[i].ls_i_print();
            }
        }
        return 0;
    }

};

#endif