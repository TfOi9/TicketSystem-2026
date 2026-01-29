#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

void clearFiles(const std::string& directory = ".") {
    try {
        if (!fs::exists(directory)) {
            return;
        }
        int count = 0;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                const auto& path = entry.path();
                if (path.extension() == ".dat") {
                    fs::remove(path);
                    count++;
                }
            }
        }
        std::cout << "共删除了 " << count << " 个 .dat 文件" << std::endl;
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "文件系统错误: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << std::endl;
    }
}

int main() {
    std::cout << "开始清理当前目录下的 .dat 文件..." << std::endl;
    clearFiles(".");
    std::cout << "清理完成" << std::endl;
    return 0;
}