#include <bits/stdc++.h>
#include "header/VFS.hpp"
VFS *vfs = new VFS();
int run() {
    std::cout << "sopoorfs" << vfs->get_current_dir() << "> "; 
    std::string cmd;
    std::string filename;

    std::cin >> cmd;
    if (cmd == "touch") { // 创建文件
        std::cin >> filename;
        vfs->create(filename);
    } else if (cmd == "ls") { // 列出当前目录下的文件
        // vfs->ls_i();
        vfs->ls();
    } else if (cmd == "format") {
        // 磁盘格式化
        vfs->format();
    } else if (cmd == "mkdir") { // 创建文件夹
        // 创建目录
        std::cin >> filename;
        vfs->mkdir(filename);
    } else if (cmd == "exit") {
        vfs->save(vfs->root);
        return -1;
    } else if (cmd == "cls") {
        system("cls");
    } else if (cmd == "help") {
        std::cout << "+ ls: list file in current directory\n"
                  << "+ format: format the file system\n"
                  << "+ cd: change the current directory to a child or parent\n"
                  << "+ mkdir: make a directory in current directory\n"
                  << "+ touch: make a file in current directory\n"
                  << "+ rm: remove the file/dir in current directory\n"
                  << "+ mv: rename the file/dir in current directory\n"
                  << "+ write: write to a file\n"
                  << "+ read: read from a file\n"
                  << "+ exit: exit the file system\n";
    } else if (cmd == "info") {
        vfs->info();
    } else if (cmd == "rm") { // 删除文件
        std::cin >> filename;
        vfs->remove(filename);
    } else if (cmd == "cd") {
        std::cin >> filename;
        vfs->cd(filename);
    } else if (cmd == "adduser") {
        std::string username;
        std::cin >> username;
        vfs->add_user(username);
    } else if (cmd == "mv") {
        std::string new_filename;
        std::cin >> filename >> new_filename;
        vfs->rename(filename, new_filename);
    } else if (cmd == "write") {
        char buf[1024];
        std::cin >> filename;
        scanf("%s", buf);
        uint32_t os, sz;
        std::cin >> os >> sz;
        vfs->write(filename, buf, os, sz);
        char buf2[1024];
        vfs->read(filename, buf2, os, sz);
        std::string test(buf2);
        std::cout << test << std::endl;
    } else if (cmd == "read") {
        std::cin >> filename;
        char buf[1024];
        uint32_t os, sz;
        std::cin >> os >> sz;
        vfs->read(filename, buf, os, sz);
        std::string output(buf);
        std::cout << output << std::endl;
    }

    else {
        std::cout << "command not found!" << std::endl;
        return 1;
    }
    return 1;

}
int login() {
    std::cout << "login:\n";
    std::cout << "username: ";
    std::string username;
    std::cin >> username;
    std::cout << "password: ";
    std::string password;
    std::cin >> password;
    for(auto& child : vfs->root->children) {
        if (child->i_name == username) {
            vfs->login(username);
            return 1;
        }
    }
    return -1;
}
int main() {
    // vfs->format();
    vfs->load_from_file();
    while (login() != 1);
    std::cout << "login successfully!\n";
    while (run() != -1);   
    return 0;
}