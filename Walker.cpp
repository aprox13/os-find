//
// Created by Roman aka ifkbhit (https://github.com/aprox13) on 03.04.2019.
//

#include "Walker.h"


Walker::Walker(const std::string& path) {
    this->basePath = path;
}

int Walker::walk() {
    if (!pathExist(basePath)) {
        std::cout << basePath << "doesn't exist\n";
        return 0;
    } else {
        if (isDirectory(basePath)) {
            recursive(basePath);
        } else {
            if (match(basePath)) {
                result.push_back(basePath);
            }
        }
    }
    return result.size();
}

void Walker::recursive(const std::string& path) {
    std::vector<std::string> files;
    fillFiles(path, files);
    for (const auto& f: files) {
        if (match(f)) {
            result.push_back(f);
        }
    }
    for (const auto& f: files) {
        if (isDirectory(f)) {
            recursive(f);
        }
    }
}

bool Walker::pathExist(const std::string& path) {
    struct stat sb{};
    return stat(path.c_str(), &sb) == 0;
}


/**
 * Function fill input vector of <b>files</b> to all files in directory at <b>path</b>
 *
 * @param path path to directory
 * @param files vector of collected files
 */
void Walker::fillFiles(const std::string& path, std::vector<std::string>& files) {
    if (pathExist(path)) {
        int fd = open(path.c_str(), O_RDONLY | O_DIRECTORY);
        if (fd == -1) {
            perror(("open@" + path).c_str());
            return;
        }
        static char buffer[BUFFER_SIZE];

        struct linux_dirent* dir;
        while (true) {
            int n = syscall(SYS_getdents64, fd, buffer, BUFFER_SIZE);
            if (n == -1) {
                perror("fillFiles@read");
                break;
            }
            if (n == 0) {
                break;
            }
            for (int i = 0; i < n; i += dir->d_reclen) {
                dir = (struct linux_dirent*) (buffer + i);
                std::string name = std::string(dir->d_name);
                if (isValidName(dir->d_name)) {
                    auto s = std::string(path);
                    if (path[path.size() - 1] != FILE_SEPARATOR) {
                        s.append(std::string(1, FILE_SEPARATOR));
                    }
                    s.append(dir->d_name);
                    files.emplace_back(s);
                }
            }
        }
        int cl = close(fd);
        if (cl == -1) {
            std::cerr << "Failed closing directory " << path << strerror(errno) << std::endl;
        }
    }
}

bool Walker::isDirectory(const std::string& path) {
    struct stat sb{};
    if (stat(path.c_str(), &sb) == 0) {
        return S_ISDIR(sb.st_mode);
    }
    return false;
}


void Walker::addFilter(const std::function<bool(const std::string&)>& filter) {
    filters.push_back(filter);
}

bool Walker::match(const std::string& path) {
    for (const auto& filter: filters) {
        if (!filter(path)) {
            return false;
        }
    }
    return true;
}

void Walker::test() {

}

std::string Walker::getNameFromPath(const std::string& path) {
    char* p = new char[path.size() + 1];
    strcpy(p, path.c_str());
    return basename(p);
}

bool Walker::isValidName(char* nameOfFile) {
    std::vector<std::string> notValid = {".", ".."};
    for (const auto& name: notValid) {
        if (strcmp(name.c_str(), nameOfFile) == 0) {
            return false;
        }
    }
    return true;
}

void Walker::withName(const std::string& fileName) {
    addFilter([&](const std::string& path) {
        auto current = getNameFromPath(path);
        return current == fileName;
    });
}

void Walker::withInod(ino_t size) {
    addFilter([&](const std::string& path) {
        struct stat sb{};
        if (stat(path.c_str(), &sb) == 0) {
            return size == sb.st_ino;

        }
        return false;
    });
}

void Walker::withNLinks(__nlink_t size) {
    addFilter([&](const std::string& path) {
        struct stat sb{};
        if (stat(path.c_str(), &sb) == 0) {
            return size == sb.st_ino;

        }
        return false;
    });
}

void Walker::withSize(char& op, __off_t& size) {
    addFilter([&](const std::string& path) {
        struct stat sb{};
        if (stat(path.c_str(), &sb) == 0) {
            if (op == '-') {
                return sb.st_size < size;
            }
            if (op == '=') {
                return sb.st_size == size;
            }
            if (op == '+') {
                return sb.st_size > size;
            }
        } else {
//            std::cout << "for path " << path << " stat is non zero. Path exist: " << std::boolalpha << pathExist(path)
//                      << std::endl;
            return false;
        }
        return false;
    });
}

void Walker::printResult() {
    for (const auto& path: result) {
        std::cout << path << std::endl;
    }
}

void Walker::exec(std::string& exec) {
    execFromHw1({exec, result[0]});
}

int Walker::execFromHw1(std::vector<std::string> args) {
    pid_t pid;
    int status = 0;

    char** callArguments = new char* [args.size() - 1];
    for (int i = 0; i < args.size(); i++) {
        callArguments[i] = new char[args[i].size() + 1];
        strcpy(callArguments[i], args[i].c_str());
    }

    pid = fork();
    if (pid == 0) {
        if (execve(callArguments[0], callArguments, nullptr) == -1) {
            perror("kidshell@execve");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("kidshell@negpid");
    } else {
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (WIFEXITED(status) == EXIT_SUCCESS &&
                 WIFSIGNALED(status) == EXIT_SUCCESS); // NOLINT(hicpp-signed-bitwise)
        std::cout << "status: " << status << std::endl;
    }
    return status;
}


