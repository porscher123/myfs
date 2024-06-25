#include <bits/stdc++.h>
#include "header/VFS.hpp"
VFS vfs;
void run() {
    std::cout << "sopoorfs > "; 
    std::string cmd;
    std::cin >> cmd;
    if (cmd == "touch") {
        std::string filename;
        std::cin >> filename;
        vfs.create(filename);
    } else if ("ls") {
        vfs.ls_i();
    }

}
int main() {
    vfs.load_from_file();
    while (true) {
        run();
    }    
    return 0;
}