#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include "ast.hpp"
#include "parser.hpp"   // генерується bison’ом і містить yyparse()

extern FILE* yyin;      // з flex
extern std::unique_ptr<Program> g_prog;

int main(int argc, char** argv) {
    std::cout << "=== MiniC (flex+bison) ===\n";
    if (argc < 2) {
        std::cerr << "Usage:\n  Lab3.exe <file.c> [--dump-ast-json <ast.json>]\n";
        return 1;
    }

    // відкрити вхід
    if (fopen_s(&yyin, argv[1], "r") != 0 || !yyin) {
        std::cerr << "Cannot open: " << argv[1] << "\n";
        return 2;
    }

    // парсинг
    if (yyparse() != 0) {
        std::cerr << "Parse failed\n";
        return 3;
    }

    // виконання
    try {
        int ret = g_prog->run();
        std::cout << "[run] main() returned: " << ret << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Runtime error: " << e.what() << "\n";
    }

    // генерація простого стекового коду
    auto code = g_prog->generate();
    std::cout << "\n[code]\n";
    for (auto& l : code) {
        std::cout << l.op;
        if (!l.a.empty()) std::cout << " " << l.a;
        if (!l.b.empty()) std::cout << ", " << l.b;
        if (!l.c.empty()) std::cout << ", " << l.c;
        std::cout << "\n";
    }

    // (опційно) дамп AST у JSON
    if (argc >= 4 && std::string(argv[2]) == "--dump-ast-json") {
        std::string rawPath = argv[3];

        if (!rawPath.empty() && rawPath.front() == '"') rawPath.erase(rawPath.begin());
        if (!rawPath.empty() && rawPath.back() == '"') rawPath.pop_back();

        // ?? Прибираємо \r, \n, пробіли
        while (!rawPath.empty() && (rawPath.back() == '\r' || rawPath.back() == '\n' || rawPath.back() == ' '))
            rawPath.pop_back();

        std::filesystem::path path(rawPath);
        path = path.make_preferred();

        try {
            if (path.has_parent_path())
                std::filesystem::create_directories(path.parent_path());
        }
        catch (...) {
            std::cerr << "Cannot create directory for " << path << "\n";
        }

        std::ofstream jf(path);
        if (!jf.is_open()) {
            std::cerr << "Cannot write file: " << path << "\n";
            perror("Reason");
            return 4;
        }

        jf << toJSON(*g_prog);
        std::cout << "[ast] JSON -> " << path << "\n";
    }


    return 0;
}
