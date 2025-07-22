#ifndef __APP_H
#define __APP_H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "ui_screen.h"
#include "ui_event_handler.h"
#include "ui_utils.h"
#include "maix_display.hpp"
#include <iostream>
#include <fstream>
using namespace maix;

class IgnoreFileHandler {
private:
    std::string filename;
    bool ensureFileExists() {
        if (!fs::exists(filename)) {
            std::ofstream newFile(filename);
            if (!newFile) {
                log::error("Could not create file %s", filename.c_str());
                return false;
            }
            newFile.close();
        }
        return true;
    }
public:
    // 构造函数，传入文件名
    IgnoreFileHandler(const std::string& file) : filename(file) {
        ensureFileExists();
    }

    ~IgnoreFileHandler() {
        system("sync");
    }

    bool checkLines(const std::string& searchStr) {
        bool result = false;
        std::ifstream file(filename);
        if (!file.is_open()) {
            log::error("Could not open file");
            return result;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.find(searchStr) != std::string::npos) {
                result = true;
                break;
            }
        }
        file.close();
        return result;
    }

    std::vector<std::pair<int, std::string>> findLines(const std::string& searchStr) {
        std::vector<std::pair<int, std::string>> results;
        std::ifstream file(filename);

        if (!file.is_open()) {
            log::error("Could not open file");
            return results;
        }

        std::string line;
        int lineNum = 1;

        while (std::getline(file, line)) {
            if (line.find(searchStr) != std::string::npos) {
                results.emplace_back(lineNum, line);
            }
            lineNum++;
        }

        file.close();
        return results;
    }

    bool appendLine(const std::string& newLine) {
        std::ofstream file(filename, std::ios::app);

        if (!file.is_open()) {
            log::error("Could not open file");
            return false;
        }

        file << newLine << std::endl;
        file.close();
        return true;
    }

    bool deleteLines(const std::string& deleteStr) {
        std::ifstream inFile(filename);
        if (!inFile.is_open()) {
            log::error("Could not open file");
            return false;
        }

        std::vector<std::string> lines;
        std::string line;

        while (std::getline(inFile, line)) {
            if (line.find(deleteStr) == std::string::npos) {
                lines.push_back(line);
            }
        }
        inFile.close();

        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            log::error("Could not open file");
            return false;
        }

        for (const auto& l : lines) {
            outFile << l << std::endl;
        }
        outFile.close();

        return true;
    }

    void printFile() {
        std::ifstream file(filename);
        if (!file.is_open()) {
            log::error("Could not open file");
            return;
        }

        std::string line;
        int lineNum = 1;
        while (std::getline(file, line)) {
            std::cout << lineNum++ << ": " << line << std::endl;
        }
        file.close();
    }
};

int app_pre_init(void);
int app_init(display::Display *disp);
int app_loop(void);
int app_deinit(void);

// #ifdef __cplusplus
// }
// #endif

#endif // __APP_H