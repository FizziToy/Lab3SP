#pragma once
#include <string>
#include <vector>
#include "ast.hpp"

// Проста фабрика міток
struct LabelGen {
    int n = 0;
    std::string next(const std::string& prefix = "L") {
        return prefix + std::to_string(n++);
    }
};

// Утиліти для генерації (спільні для всіх вузлів)
struct CG {
    Code& code;
    LabelGen& L;
    CG(Code& code, LabelGen& L) : code(code), L(L) {}

    void emit(const std::string& op,
        const std::string& a = "",
        const std::string& b = "",
        const std::string& c = "") {
        code.push_back({ op, a, b, c });
    }
};

