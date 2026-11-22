#include <iostream>

#ifdef _WIN32
#include <cstdio>
#include <io.h>
#include <ftxui/dom/elements.hpp> //temporar
#else
#include <unistd.h>
#endif

namespace {

    bool esteTerminalInteractiv() {
#ifdef _WIN32
        return _isatty(_fileno(stdin)) != 0;
#else
        return isatty(STDIN_FILENO) != 0;
#endif
    }

    void ruleazaModCLI() {
        // Citeste comenzi din std::cin (folosit de CI cu tastatura.txt)
        // Apeleaza functiile din biblioteca ta oop (incarcare TLE, calcul treceri, filtrare, sortare)
    }

    void ruleazaModFTXUI() {
        // Porneste interfata FTXUI cu harta cerului (Canvas)
        // Apeleaza aceleasi functii din biblioteca ta oop
    }

} // namespace

int main() {
    try {
        if (esteTerminalInteractiv()) {
            ruleazaModFTXUI();
        } else {
            ruleazaModCLI();
        }
    } catch (const std::exception& e) {
        std::cerr << "Eroare fatala: " << e.what() << '\n';
        return 1;
    }
    return 0;
}