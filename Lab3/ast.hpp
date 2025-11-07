#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdlib>

/* ===== Псевдокод ===== */
struct CodeLine { std::string op, a, b, c; };
using Code = std::vector<CodeLine>;

/* ===== Середовище виконання ===== */
struct Env { std::map<std::string, int> vars; };

/* ===== Базові вузли ===== */
struct Node { virtual ~Node() = default; };

/* ===== Вирази ===== */
struct Expr : Node {
    virtual int  eval(Env&) = 0;
    virtual void gen(Code&) = 0;
};

struct Num : Expr {
    int v;
    explicit Num(int v) : v(v) {}
    int  eval(Env&) override { return v; }
    void gen(Code& c) override { c.push_back({ "PUSH", std::to_string(v), "", "" }); }
};

struct Var : Expr {
    std::string n;
    explicit Var(char* s) : n(s) { free(s); }
    int  eval(Env& e) override { return e.vars[n]; }
    void gen(Code& c) override { c.push_back({ "LOAD", n, "", "" }); }
};

struct Bin : Expr {
    std::string op;
    std::unique_ptr<Expr> l, r;
    Bin(std::string op, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(std::move(op)), l(std::move(l)), r(std::move(r)) {
    }
    int  eval(Env& e) override;
    void gen(Code& c) override;
};

/* ===== Оператори ===== */
struct Stmt : Node {
    virtual void exec(Env&, int& ret, bool& hasRet) = 0;
    virtual void gen(Code&) = 0;
};

struct Decl : Stmt {
    std::string n;
    std::unique_ptr<Expr> init;
    Decl(char* s, std::unique_ptr<Expr> e) : n(s), init(std::move(e)) { free(s); }
    void exec(Env& e, int&, bool&) override { e.vars[n] = init ? init->eval(e) : 0; }
    void gen(Code& c) override { if (init) init->gen(c); c.push_back({ "DECL", n, "", "" }); }
};

struct Assign : Stmt {
    std::string n;
    std::unique_ptr<Expr> e;
    Assign(char* s, std::unique_ptr<Expr> ex) : n(s), e(std::move(ex)) { free(s); }
    void exec(Env& env, int&, bool&) override { env.vars[n] = e->eval(env); }
    void gen(Code& c) override { e->gen(c); c.push_back({ "STORE", n, "", "" }); }
};

struct Block : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    void exec(Env& e, int& ret, bool& hasRet) override {
        for (auto& s : stmts) { if (hasRet) break; s->exec(e, ret, hasRet); }
    }
    void gen(Code& c) override { for (auto& s : stmts) s->gen(c); }
};

struct If : Stmt {
    std::unique_ptr<Expr> c; std::unique_ptr<Stmt> t, f;
    If(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> f)
        : c(std::move(c)), t(std::move(t)), f(std::move(f)) {
    }
    void exec(Env& e, int& ret, bool& hasRet) override {
        if (c->eval(e)) t->exec(e, ret, hasRet); else if (f) f->exec(e, ret, hasRet);
    }
    void gen(Code& code) override {
        c->gen(code); code.push_back({ "IF", "", "", "" });
        t->gen(code);
        if (f) { code.push_back({ "ELSE", "", "", "" }); f->gen(code); }
        code.push_back({ "ENDIF", "", "", "" });
    }
};

struct While : Stmt {
    std::unique_ptr<Expr> c; std::unique_ptr<Stmt> b;
    While(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b)
        : c(std::move(c)), b(std::move(b)) {
    }
    void exec(Env& e, int& ret, bool& hasRet) override {
        while (c->eval(e) && !hasRet) b->exec(e, ret, hasRet);
    }
    void gen(Code& code) override {
        code.push_back({ "WHILE_BEGIN", "", "", "" });
        c->gen(code); b->gen(code);
        code.push_back({ "WHILE_END", "", "", "" });
    }
};

struct Ret : Stmt {
    std::unique_ptr<Expr> e;
    explicit Ret(std::unique_ptr<Expr> e) : e(std::move(e)) {}
    void exec(Env& env, int& ret, bool& hasRet) override { ret = e->eval(env); hasRet = true; }
    void gen(Code& c) override { e->gen(c); c.push_back({ "RET", "", "", "" }); }
};

/* ===== Функції та програма ===== */
struct Function : Node {
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<Block> body;

    // ВАЖЛИВО: без free(n) — уникнення double-free
    Function(char* n, const std::vector<std::string>& ps, std::unique_ptr<Block> b)
        : name(n), params(ps), body(std::move(b)) {
    }

    int  call(const std::vector<int>& args);
    void gen(Code& code);
};

struct Program : Node {
    std::vector<std::unique_ptr<Function>> funcs;
    Function* find(const std::string& n);
    int  run();        // викликає main()
    Code generate();   // стековий псевдокод
};

/* ===== утиліти ===== */
std::string toJSON(const Program&);
