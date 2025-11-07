#include "codegen.hpp"

static void genStmt(Stmt* s, Code& out, LabelGen& L); // ‚ÔÂÂ‰

// === ¬»–¿«» ===
// ¬‡¯ Bin::gen ÛÊÂ ‚ËÁÌ‡˜ÂÌËÈ Û ast.cpp (‚ËÍÓËÒÚÓ‚Û∫Ú¸Òˇ ÒÚÂÍÓ‚ËÈ Ô≥‰ı≥‰).

// === ¬—œŒÃŒ√¿“≈À‹Õ≤ ===
struct GenBlock {
    static void go(Block* b, Code& out, LabelGen& L) {
        for (auto& st : b->stmts) genStmt(st.get(), out, L);
    }
};

// === √≈Õ≈–¿÷≤ﬂ Œœ≈–¿“Œ–≤¬ ===
static void genStmt(Stmt* s, Code& out, LabelGen& L) {
    if (auto d = dynamic_cast<Decl*>(s)) {
        // int n = expr;
        if (d->init) d->init->gen(out);
        CG cg(out, L);
        cg.emit("DECL", d->n);
        return;
    }

    if (auto a = dynamic_cast<Assign*>(s)) {
        // n = expr;
        a->e->gen(out);
        CG cg(out, L);
        cg.emit("STORE", a->n);
        return;
    }

    if (auto r = dynamic_cast<Ret*>(s)) {
        r->e->gen(out);
        CG cg(out, L);
        cg.emit("RET");
        return;
    }

    if (auto w = dynamic_cast<While*>(s)) {
        CG cg(out, L);
        std::string Lcond = L.next("Lcond");
        std::string Lend  = L.next("Lend");

        cg.emit("LABEL", Lcond);
        w->c->gen(out);
        cg.emit("JZ", Lend);
        w->b->gen(out);
        cg.emit("JMP", Lcond);
        cg.emit("LABEL", Lend);
        return;
    }

    if (auto i = dynamic_cast<If*>(s)) {
        CG cg(out, L);
        std::string Lelse = L.next("Lelse");
        std::string Lend  = L.next("Lend");

        i->c->gen(out);
        cg.emit("JZ", Lelse);
        i->t->gen(out);
        cg.emit("JMP", Lend);
        cg.emit("LABEL", Lelse);
        if (i->f) i->f->gen(out);
        cg.emit("LABEL", Lend);
        return;
    }

    if (auto b = dynamic_cast<Block*>(s)) {
        GenBlock::go(b, out, L);
        return;
    }
}

// === √≈Õ≈–¿÷≤ﬂ ‘”Õ ÷≤… ===
void genFunction(Function* f, Code& out) {
    LabelGen L;
    CG cg(out, L);
    cg.emit("FUNC", f->name);
    if (f->body) f->body->gen(out);
    cg.emit("ENDFUNC", f->name);
}

// === √≈Õ≈–¿÷≤ﬂ œ–Œ√–¿Ã» ===
Code genProgram(Program* p) {
    Code code;
    for (auto& fn : p->funcs) {
        genFunction(fn.get(), code);
    }
    return code;
}
