int fact(int n) {
    int r = 1;
    while (n > 1) {
        r = r * n;
        n = n - 1;
    }
    return r;
}

int main() {
    int a = 5;
    int f = 120; 
    if (f > 100) { f = f - 100; }
    else { f = f + 100; }
    return f;
}
