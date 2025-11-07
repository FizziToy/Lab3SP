#include "ast.hpp"
#include <stdexcept>
#include <sstream>

static int bop(const std::string& op, int a, int b) {
	if (op == "+") return a + b; if (op == "-") return a - b; if (op == "*") return a * b;
	if (op == "/") return b ? a / b : 0; if (op == "%") return b ? a % b : 0;
	if (op == "==") return a == b; if (op == "!=") return a != b;
	if (op == "<") return a < b; if (op == "<=") return a <= b; if (op == ">") return a > b; if (op == ">=") return a >= b;
	throw std::runtime_error("op");
}

int  Bin::eval(Env& e) { return bop(op, l->eval(e), r->eval(e)); }
void Bin::gen(Code& c) { l->gen(c); r->gen(c); c.push_back({ op,"","","" }); }

int Function::call(const std::vector<int>& args) {
	Env env;
	for (size_t i = 0;i < params.size();++i) env.vars[params[i]] = (i < args.size() ? args[i] : 0);
	int ret = 0; bool hasRet = false; body->exec(env, ret, hasRet); return ret;
}
void Function::gen(Code& code) { code.push_back({ "FUNC",name,"","" }); body->gen(code); code.push_back({ "ENDFUNC",name,"","" }); }

Function* Program::find(const std::string& n) { for (auto& f : funcs) if (f->name == n) return f.get(); return nullptr; }
int Program::run() { auto* m = find("main"); if (!m) throw std::runtime_error("no main"); return m->call({}); }
Code Program::generate() { Code c; for (auto& f : funcs) f->gen(c); return c; }

std::string toJSON(const Program&) {
	return R"({"name":"Program","children":[{"name":"...AST..."}]})";
}
