#pragma once
#include <stdexcept>
#include <string>

namespace orbitwatch {

    class EroareOrbitWatch : public std::runtime_error {
    public:
        explicit EroareOrbitWatch(const std::string& mesaj)
            : std::runtime_error(mesaj) {}
    };

    class EroareFisier : public EroareOrbitWatch {
    public:
        explicit EroareFisier(const std::string& caleFisier)
            : EroareOrbitWatch("Nu s-a putut deschide sau citi fisierul: " + caleFisier) {}
    };

    class EroareParsare : public EroareOrbitWatch {
    public:
        explicit EroareParsare(const std::string& detalii)
            : EroareOrbitWatch("Eroare la parsarea datelor orbitale: " + detalii) {}
    };

    class CoordonateInvalide : public EroareOrbitWatch {
    public:
        explicit CoordonateInvalide(const std::string& detalii)
            : EroareOrbitWatch("Coordonate geografice invalide: " + detalii) {}
    };

}