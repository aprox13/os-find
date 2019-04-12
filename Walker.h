#pragma once

#include <string>
#include <vector>
#include <functional>
#include <set>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <libgen.h>
#include <cstring>

#include <unistd.h>
#include <wait.h>

#include <fcntl.h>
#include <syscall.h>

//
// Created by Roman aka ifkbhit (https://github.com/aprox13) on 03.04.2019.
//



class Walker {
public:


    explicit Walker(const std::string& path);

    int walk();

    void addFilter(const std::function<bool(const std::string&)>& filter);

    void test();

    void withName(const std::string& name);

    void withInod(ino_t size);

    void withNLinks(__nlink_t size);

    void withSize(char& op, __off_t& size);

    void printResult();


    void exec(std::string& exec);

private:


    static const size_t BUFFER_SIZE = 8192;

    const char FILE_SEPARATOR = '/';

    static std::string getNameFromPath(const std::string& path);

    static bool isValidName(char* name);

//    struct stat sb;
    std::vector<std::string> result;
    std::string basePath;
    std::vector<std::function<bool(const std::string&)>> filters;


    static bool pathExist(const std::string& path);

    static bool isDirectory(const std::string& path);

    void recursive(const std::string&);

    void fillFiles(const std::string& path, std::vector<std::string>& files);

    bool match(const std::string& path);


    int execFromHw1(std::vector<std::string> args);


    struct linux_dirent {
        unsigned long  d_ino;
        off64_t        d_off;
        unsigned short d_reclen;
        unsigned char  d_type;
        char           d_name[];
    };
};

