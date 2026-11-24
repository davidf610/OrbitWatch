#include <algorithm>
#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <vector>

#include "Coordonate.hpp"
#include "ParserDateOrbitale.hpp"
#include "PropagatorOrbital.hpp"
#include "TrecereSatelit.hpp"

namespace {

void afiseazaMeniu(const orbitwatch::CoordonateGeo& observator) {
    std::cout << "\nOrbit Watch\n"
              << "Locatie curenta: lat=" << observator.latitudine()
              << ", lon=" << observator.longitudine()
              << ", alt=" << observator.altitudineKm() << " km\n"
              << "1. Afiseaza satelitii incarcati\n"
              << "2. Seteaza locatia observatorului\n"
              << "3. Calculeaza trecerile vizibile\n"
              << "0. Iesire\n"
              << "Optiune: ";
}

void curataInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

} // namespace

int main() {
    try {
        const auto sateliti =
            orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");

        if (sateliti.empty()) {
            std::cout << "Nu s-au gasit sateliti in assets/sateliti.json.\n";
            return 0;
        }

        // Locatie implicita: Bucuresti
        orbitwatch::CoordonateGeo observator(44.4268, 26.1025, 0.08);

        int optiune = -1;
        while (true) {
            afiseazaMeniu(observator);
            if (!(std::cin >> optiune)) {
                if (std::cin.eof()) {
                    break;
                }
                std::cout << "Input invalid. Introdu un numar.\n";
                curataInput();
                continue;
            }

            if (optiune == 0) {
                std::cout << "La revedere!\n";
                break;
            }

            if (optiune == 1) {
                std::cout << "\nSateliti incarcati (" << sateliti.size() << "):\n";
                for (const auto& sat : sateliti) {
                    std::cout << " - [" << sat->noradId() << "] " << sat->nume()
                              << " | Tip: " << sat->tipText()
                              << " | Elevatie min: " << sat->elevatieMinimaNecesara() << " deg\n";
                }
            } else if (optiune == 2) {
                double lat = 0.0;
                double lon = 0.0;
                double alt = 0.0;
                std::cout << "Introdu latitudine [-90, 90], longitudine [-180, 180], altitudine (km): ";
                if (!(std::cin >> lat >> lon >> alt)) {
                    std::cout << "Valori invalide.\n";
                    curataInput();
                    continue;
                }
                try {
                    observator = orbitwatch::CoordonateGeo(lat, lon, alt);
                    std::cout << "Locatie actualizata.\n";
                } catch (const std::exception& e) {
                    std::cout << "Eroare coordonate: " << e.what() << '\n';
                }
            } else if (optiune == 3) {
                int ore = 48;
                std::cout << "Durata scanare in ore (ex: 24, 48): ";
                if (!(std::cin >> ore) || ore <= 0 || ore > 168) {
                    std::cout << "Durata invalida (alege intre 1 si 168 ore).\n";
                    curataInput();
                    continue;
                }

                // Pornim de la epoch-ul datelor pentru rezultate consistente si reproductibile
                const auto start = sateliti[0]->elemente().epoch();
                std::vector<orbitwatch::TrecereSatelit> toateTrecerile;

                for (const auto& sat : sateliti) {
                    auto treceri = orbitwatch::PropagatorOrbital::gasesteTreceriVizibile(
                        *sat, observator, start, std::chrono::hours{ore}, std::chrono::seconds{60});
                    toateTrecerile.insert(toateTrecerile.end(), treceri.begin(), treceri.end());
                }

                std::sort(toateTrecerile.begin(),
                          toateTrecerile.end(),
                          [](const orbitwatch::TrecereSatelit& a,
                             const orbitwatch::TrecereSatelit& b) {
                              return a.oraInceput() < b.oraInceput();
                          });

                if (toateTrecerile.empty()) {
                    std::cout << "Nicio trecere vizibila in urmatoarele " << ore << " ore.\n";
                } else {
                    std::cout << "\nTreceri vizibile (" << toateTrecerile.size() << "):\n";
                    for (const auto& trecere : toateTrecerile) {
                        std::cout << " - " << trecere << '\n';
                    }
                }
            } else {
                std::cout << "Optiune necunoscuta.\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Eroare fatala: " << e.what() << '\n';
        return 1;
    }

    return 0;
}