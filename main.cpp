#include <iostream>
#include "argparser/ArgParser.h"
#include "argparser/ParserException.h"
#include <algorithm>
#include "Walker.h"
#include <sys/stat.h>

//
// Created by Roman aka ifkbhit (https://github.com/aprox13) on 01.04.2019.
//


int main(int argc, char **argv) {
    parser::ArgParser arguments;
    arguments.setOptionalCount(1);
    arguments.addArgument("inum", 1);
    arguments.addArgument("name", 1);
    arguments.addArgument("size", 1);
    arguments.addArgument("nlinks", 1);
    arguments.addArgument("exec", 1);

    try {
        arguments.parse(argv);
//        arguments.parse("pr /home/ifkbhit/Desktop/test -name EmptyJava.java -exec /usr/bin/sha1sum");
    } catch (parser::ParserException &e) {
        std::cout << "Error while parsing arguments. " << e.what() << std::endl;
        return 0;
    }


    std::string path = arguments.getOptional()[1];

    Walker walker = Walker(path);


    if (arguments.exist("inum")) {
        walker.withInod(std::stoi(arguments["inum"]));
    }
    if (arguments.exist("name")) {
        walker.withName(arguments["name"]);
    }
    if (arguments.exist("size")) {
        auto s = arguments["size"];
        if (s.size() <= 1 || (s[0] != '-' && s[0] != '+' && s[0] != '=')) {
            std::cout << "Wrong argument size\n";
            return 0;
        }
        char op = s[0];
        __off_t size = std::stol(s.substr(1));
        walker.withSize(op, size);

    }
    if (arguments.exist("nlinks")) {
	std::cout << "Find with nlinks" << arguments["nlinks"] << '\n';
	    walker.withNLinks(std::stoi(arguments["nlinks"]));
    }

    int matches = walker.walk();
    std::cout << "found: " << matches << " matches\n";
    if (arguments.exist("exec")) {
        if (matches != 0) {
            walker.exec(arguments["exec"]);
        }
        return 0;
    } else {
        walker.printResult();
    }
    return 0;
}



