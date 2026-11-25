#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <utility>
#include <filesystem>
#include <string>

#include "Coordonate.hpp"
#include "ManagerTreceri.hpp"
#include "ParserDateOrbitale.hpp"
#include "RaportTreceri.hpp"
#include "CatalogLocatii.hpp"
#include "PlanificatorObservatii.hpp"
#include "ComparatorLocatii.hpp"

namespace {

void afiseazaMeniu(const orbitwatch::CoordonateGeo& observator) {
    std::cout << "\nOrbit Watch\n"
        << "Locatie curenta: " << observator << '\n'
        << "1. Afiseaza starea curenta (sateliti si treceri)\n"
        << "2. Seteaza locatia observatorului\n"
        << "3. Calculeaza trecerile vizibile\n"
        << "4. Filtreaza trecerile dupa elevatie minima\n"
        << "5. Afiseaza cea mai buna trecere (magnitudine minima)\n"
        << "6. Compara cu o alta locatie (test copiere manager)\n"
        << "7. Genereaza raport detaliat\n"
        << "8. Alege locatie din catalog si genereaza plan de observatii\n"
        << "9. Compara toate locatiile din catalog (clasament)\n"
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
        const auto catalog = orbitwatch::CatalogLocatii::incarcaDinFisier("assets/locatii.json");
        if (catalog.gol()) {
            std::cout << "Nu s-au gasit locatii in assets/locatii.json.\n";
            return 0;
        }

        auto sateliti =
            orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");

        if (sateliti.empty()) {
            std::cout << "Nu s-au gasit sateliti in assets/sateliti.json.\n";
            return 0;
        }

        const auto epochStart = sateliti[0]->elemente().epoch();
        orbitwatch::ManagerTreceri manager(
            catalog.locatii().front().coordonate(), std::move(sateliti));

        int optiune = -1;
        while (true) {
            afiseazaMeniu(manager.observator());
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
                std::cout << '\n' << manager;
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
                    manager.actualizeazaObservator(orbitwatch::CoordonateGeo(lat, lon, alt));
                    std::cout << "Locatie actualizata: " << manager.observator() << '\n';
                } catch (const std::exception& e) {
                    std::cout << "Eroare coordonate: " << e.what() << '\n';
                }
            } else if (optiune == 3) {
                int ore = 48;
                std::cout << "Durata scanare in ore (1..168): ";
                if (!(std::cin >> ore) || ore <= 0 || ore > 168) {
                    std::cout << "Durata invalida.\n";
                    curataInput();
                    continue;
                }

                const auto& treceri =
                    manager.calculeazaTreceri(epochStart, std::chrono::hours{ore});
                if (treceri.empty()) {
                    std::cout << "Nicio trecere vizibila in urmatoarele " << ore << " ore.\n";
                } else {
                    std::cout << "\nTreceri vizibile (" << treceri.size() << "):\n";
                    for (const auto& t : treceri) {
                        std::cout << " - " << t << '\n';
                    }
                }
            } else if (optiune == 4) {
                double elevMin = 20.0;
                std::cout << "Elevatie minima (grade): ";
                if (!(std::cin >> elevMin) || elevMin < 0.0 || elevMin > 90.0) {
                    std::cout << "Elevatie invalida.\n";
                    curataInput();
                    continue;
                }

                const auto filtrate = manager.filtreazaDupaElevatie(elevMin);
                if (filtrate.empty()) {
                    std::cout << "Nicio trecere peste " << elevMin << " grade.\n";
                } else {
                    std::cout << "\nTreceri peste " << elevMin << " grade (" << filtrate.size()
                              << "):\n";
                    for (const auto& t : filtrate) {
                        std::cout << " - " << t << '\n';
                    }
                }
            } else if (optiune == 5) {
                const auto ceaMaiBuna = manager.ceaMaiBunaTrecere();
                if (!ceaMaiBuna.has_value()) {
                    std::cout << "Nu exista treceri calculate inca (ruleaza optiunea 3).\n";
                } else {
                    std::cout << "Cea mai buna trecere:\n - " << *ceaMaiBuna << '\n';
                }
            } else if (optiune == 6) {
                std::cout << "Introdu numele locatiei pentru comparatie: ";
                std::string numeComparatie;
                std::getline(std::cin >> std::ws, numeComparatie);

                const auto locatieComp = catalog.cautaDupaNume(numeComparatie);
                if (!locatieComp.has_value()) {
                    std::cout << "Locatia '" << numeComparatie << "' nu exista in catalog.\n";
                } else {
                    orbitwatch::ManagerTreceri copie(manager);
                    copie.actualizeazaObservator(locatieComp->coordonate());
                    const auto& treceriComp =
                        copie.calculeazaTreceri(epochStart, std::chrono::hours{48});
                    std::cout << "Comparatie " << locatieComp->nume() << " ("
                              << copie.numarSateliti() << " sateliti): " << treceriComp.size()
                              << " treceri vizibile.\n";

                    copie = manager;
                    std::cout << "Dupa restaurare copie: " << copie.observator() << '\n';
                }
            } else if (optiune == 7) {
                const auto filtrate = manager.filtreazaDupaElevatie(0.0);
                orbitwatch::RaportTreceri raport("Raport OrbitWatch", manager.observator(), filtrate);
                std::cout << '\n' << raport;

                const auto dirTemp = std::filesystem::temp_directory_path();
                raport.exportaCsv(dirTemp / "orbitwatch_raport.csv");
                raport.exportaText(dirTemp / "orbitwatch_raport.txt");
                std::cout << "Raport exportat in: " << dirTemp.string() << '\n';
            } else if (optiune == 8) {
                std::cout << '\n' << catalog;
                std::cout << "Total locatii disponibile: " << catalog.locatii().size() << '\n';
                if (const auto intunecat = catalog.celMaiIntunecatCer()) {
                    std::cout << "Cel mai intunecat cer: " << *intunecat << '\n';
                }
                if (const auto apropiata = catalog.ceaMaiApropiataDe(manager.observator())) {
                    std::cout << "Cea mai apropiata locatie din catalog: " << *apropiata << '\n';
                }

                std::cout << "Introdu numele locatiei din catalog: ";
                std::string numeOras;
                std::getline(std::cin >> std::ws, numeOras);

                double scorMinim = 0.0;
                std::cout << "Scor minim dorit (0..100): ";
                if (!(std::cin >> scorMinim)) {
                    std::cout << "Valoare invalida.\n";
                    curataInput();
                    continue;
                }

                const auto locatieAleasa = catalog.cautaDupaNume(numeOras);
                if (!locatieAleasa.has_value()) {
                    std::cout << "Locatia '" << numeOras << "' nu a fost gasita in catalog.\n";
                } else {
                    manager.actualizeazaObservator(locatieAleasa->coordonate());
                    const auto& treceriNoi =
                        manager.calculeazaTreceri(epochStart, std::chrono::hours{48});
                    orbitwatch::PlanificatorObservatii planificator(*locatieAleasa, treceriNoi);
                    std::cout << '\n' << planificator;

                    const auto faraConflicte = planificator.selecteazaProgramFaraConflicte();
                    const auto scorBun = planificator.filtreazaDupaScorMinim(scorMinim);

                    std::cout << "Plan generat pentru: " << planificator.locatie().nume() << ", "
                              << planificator.locatie().tara() << '\n';
                    std::cout << "Program fara conflicte: " << faraConflicte.size()
                              << " treceri | Cu scor >= " << scorMinim << ": " << scorBun.size()
                              << '\n';
                    if (!faraConflicte.empty()) {
                        std::cout << "Prima recomandare: " << faraConflicte.front().recomandare()
                                  << '\n';
                    }
                }
            } else if (optiune == 9) {
                int oreComp = 0;
                std::size_t minimTreceri = 0;
                std::cout << "Durata comparatie in ore (1..168) si numar minim treceri: ";
                if (!(std::cin >> oreComp >> minimTreceri) || oreComp <= 0 || oreComp > 168) {
                    std::cout << "Valori invalide.\n";
                    curataInput();
                    continue;
                }

                orbitwatch::ComparatorLocatii comparator(
                    catalog, manager, epochStart, std::chrono::hours{oreComp});
                std::cout << '\n' << comparator;

                const auto eligibile = comparator.filtreazaCuTreceriMinime(minimTreceri);
                std::cout << "Locatii analizate: " << comparator.clasament().size()
                          << " | Cu minim " << minimTreceri << " treceri: " << eligibile.size() << '\n';
                if (const auto top = comparator.ceaMaiBunaLocatie()) {
                    std::cout << "Recomandare principala: " << top->locatie().nume()
                              << " (fara conflicte: " << top->numarTreceriFaraConflicte()
                              << ", scor max: " << top->scorMaxim() << ")\n";
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