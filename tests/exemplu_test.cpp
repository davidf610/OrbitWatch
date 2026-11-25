#include <cassert>
#include <chrono>
#include <cmath>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <filesystem>

#include "Coordonate.hpp"
#include "ElementeOrbitale.hpp"
#include "OrbitExceptii.hpp"
#include "Satelit.hpp"
#include "SatelitStarlink.hpp"
#include "StatieSpatiala.hpp"
#include "TelescopSpatial.hpp"
#include "ParserDateOrbitale.hpp"
#include "PropagatorOrbital.hpp"
#include "TrecereSatelit.hpp"
#include "ManagerTreceri.hpp"
#include "RaportTreceri.hpp"
#include "CatalogLocatii.hpp"
#include "PlanificatorObservatii.hpp"
#include "ComparatorLocatii.hpp"


namespace {

bool aproapeEgal(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

void testeazaCoordonate() {
    // 1. Caz valid (Bucuresti)
    const orbitwatch::CoordonateGeo loc(44.4268, 26.1025, 0.08);
    assert(aproapeEgal(loc.latitudine(), 44.4268));
    assert(aproapeEgal(loc.longitudine(), 26.1025));
    assert(aproapeEgal(loc.altitudineKm(), 0.08));

    // 2. Caz invalid - latitudine peste 90 grade
    bool aAruncatLat = false;
    try {
        const orbitwatch::CoordonateGeo invalid(95.0, 25.0);
        (void)invalid;
    } catch (const orbitwatch::CoordonateInvalide&) {
        aAruncatLat = true;
    }
    assert(aAruncatLat);


    // 3. Coordonate orizontale
    const orbitwatch::CoordonateOrizontale oriz(180.0, 45.0, 550.0);
    assert(aproapeEgal(oriz.azimut(), 180.0));
    assert(aproapeEgal(oriz.elevatie(), 45.0));
    assert(aproapeEgal(oriz.distantaKm(), 550.0));

    std::ostringstream ossGeo;
    ossGeo << loc;
    assert(!ossGeo.str().empty());

    std::ostringstream ossOriz;
    ossOriz << oriz;
    assert(!ossOriz.str().empty());
}

void testeazaElementeOrbitale() {
    const auto acum = std::chrono::system_clock::now();
    const orbitwatch::ElementeOrbitale elem(
    acum,
        51.6416,  // inclinatie
        247.4627, // raan
        0.00067,  // excentricitate
        130.5360, // argument perigeu
        325.0288, // anomalie medie
        15.4956// mean motion
        );


    assert(elem.epoch() == acum);
    assert(aproapeEgal(elem.inclinatie(), 51.6416));
    assert(aproapeEgal(elem.raan(), 247.4627));
    assert(aproapeEgal(elem.excentricitate(), 0.00067));
    assert(aproapeEgal(elem.argumentPerigeu(), 130.5360));
    assert(aproapeEgal(elem.anomalieMedie(), 325.0288));
    assert(aproapeEgal(elem.meanMotion(), 15.4956));
    std::ostringstream ossElem;
    ossElem << elem;
    assert(!ossElem.str().empty());
}

void testeazaIerarhieSateliti() {
    const auto acum = std::chrono::system_clock::now();
    const orbitwatch::ElementeOrbitale elem(
        acum, 51.64, 247.46, 0.0006, 130.5, 325.0, 15.49);

    // 1. Validare constructor Satelit (NORAD ID invalid arunca EroareParsare)
    bool aAruncatId = false;
    try {
        const orbitwatch::StatieSpatiala invalid(-5, "ISS", elem);
        (void)invalid;
    } catch (const orbitwatch::EroareParsare&) {
        aAruncatId = true;
    }
    assert(aAruncatId);

    // 2. Colectie polimorfica de sateliti
    std::vector<std::unique_ptr<orbitwatch::Satelit>> sateliti;
    sateliti.push_back(std::make_unique<orbitwatch::StatieSpatiala>(25544, "ISS (ZARYA)", elem));
    sateliti.push_back(std::make_unique<orbitwatch::SatelitStarlink>(58912, "STARLINK-31042", elem, true));
    sateliti.push_back(std::make_unique<orbitwatch::SatelitStarlink>(58913, "STARLINK-31043", elem, false));
    sateliti.push_back(std::make_unique<orbitwatch::TelescopSpatial>(20580, "HST", elem));

    // Pozitie la 12 grade elevatie si Soarele la -8 grade (crepuscul civil):
    // - StatieSpatiala (prag elevatie 10, soare -6) -> VIZIBIL
    // - Starlink (prag elevatie 20, soare -10) -> INVIZIBIL
    // - TelescopSpatial (prag elevatie 15, soare -12) -> INVIZIBIL
    const orbitwatch::CoordonateOrizontale pozitieJoasa(145.0, 12.0, 900.0);
    assert(sateliti[0]->poateFiObservat(pozitieJoasa, -8.0, true));
    assert(!sateliti[1]->poateFiObservat(pozitieJoasa, -8.0, true));
    assert(!sateliti[3]->poateFiObservat(pozitieJoasa, -8.0, true));

    // In umbra Pamantului (esteIluminatDeSoare == false) -> niciunul nu e vizibil
    const orbitwatch::CoordonateOrizontale pozitieInalta(180.0, 60.0, 420.0);
    assert(!sateliti[0]->poateFiObservat(pozitieInalta, -15.0, false));

    // 3. Verificare polimorfism magnitudine si getteri
    const double magISS = sateliti[0]->calculeazaMagnitudine(420.0, 30.0);
    const double magStarlinkTren = sateliti[1]->calculeazaMagnitudine(420.0, 30.0);
    const double magStarlinkOp = sateliti[2]->calculeazaMagnitudine(420.0, 30.0);
    const double magHubble = sateliti[3]->calculeazaMagnitudine(540.0, 30.0);

    // ISS e mai stralucitoare (magnitudine mai mica) decat Starlink in tren,
    // iar Starlink in tren e mai stralucitor decat Starlink operational
    assert(magISS < magStarlinkTren);
    assert(magStarlinkTren < magStarlinkOp);
    assert(!std::isnan(magHubble));

    for (const auto& sat : sateliti) {
        assert(sat->noradId() > 0);
        assert(!sat->nume().empty());
        assert(!sat->tipText().empty());
        assert(sat->elemente().meanMotion() > 0.0);

        // 4. Testare copiere polimorfica prin clone()
        auto copie = sat->clone();
        assert(copie->noradId() == sat->noradId());
        assert(copie->nume() == sat->nume());
        assert(copie->tipText() == sat->tipText());
        std::ostringstream ossSat;
        ossSat << *sat;
        assert(!ossSat.str().empty());
    }

    // Verificare getter specific Starlink
    const auto* starlinkPtr = dynamic_cast<const orbitwatch::SatelitStarlink*>(sateliti[1].get());
    assert(starlinkPtr != nullptr);
    assert(starlinkPtr->esteInTrenLansare());
}

    void testeazaParserDateOrbitale() {
    // 1. Incarcare reala din assets/sateliti.json
    const auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    assert(sateliti.size() == 3);
    assert(sateliti[0]->noradId() == 25544);
    assert(sateliti[0]->tipText() == "Statie Spatiala");
    assert(sateliti[1]->tipText() == "Starlink (Tren Lansare)");
    assert(sateliti[2]->tipText() == "Telescop Spatial");

    // 2. Fisier inexistent -> EroareFisier
    bool aAruncatFisier = false;
    try {
        const auto lista = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/nu_exista.json");
        (void)lista;
    } catch (const orbitwatch::EroareFisier&) {
        aAruncatFisier = true;
    }
    assert(aAruncatFisier);

    // 3. JSON invalid -> EroareParsare
    bool aAruncatJson = false;
    try {
        const auto lista = orbitwatch::ParserDateOrbitale::incarcaDinString("{ invalid json");
        (void)lista;
    } catch (const orbitwatch::EroareParsare&) {
        aAruncatJson = true;
    }
    assert(aAruncatJson);
}
    void testeazaPropagatorSiTreceri() {
    const auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    const orbitwatch::CoordonateGeo bucuresti(44.4268, 26.1025, 0.08);
    const auto start = sateliti[0]->elemente().epoch();

    // 1. Pozitia si elevatia Soarelui trebuie sa fie in limite fizice valide
    const auto poz = orbitwatch::PropagatorOrbital::calculeazaPozitie(*sateliti[0], bucuresti, start);
    assert(poz.azimut() >= 0.0 && poz.azimut() < 360.0);
    assert(poz.elevatie() >= -90.0 && poz.elevatie() <= 90.0);
    assert(poz.distantaKm() > 100.0);

    const double elevSoare = orbitwatch::PropagatorOrbital::calculeazaElevatieSoare(bucuresti, start);
    assert(elevSoare >= -90.0 && elevSoare <= 90.0);

    const bool iluminat = orbitwatch::PropagatorOrbital::esteIluminatDeSoare(*sateliti[0], start);
    (void)iluminat;

    // 2. Scanare pe 48h pentru ISS
    const auto treceri = orbitwatch::PropagatorOrbital::gasesteTreceriVizibile(
        *sateliti[0], bucuresti, start, std::chrono::hours{48}, std::chrono::seconds{60});

    for (const auto& t : treceri) {
        assert(!t.numeSatelit().empty());
        assert(!t.tipSatelit().empty());
        assert(t.oraInceput() <= t.oraMaxim());
        assert(t.oraMaxim() <= t.oraSfarsit());
        assert(t.elevatieMaxima() >= 10.0);
        assert(t.azimutMaxim() >= 0.0 && t.azimutMaxim() < 360.0);
        assert(!std::isnan(t.magnitudineMinima()));

        std::ostringstream oss;
        oss << t;
        assert(!oss.str().empty());
    }
}
    void testeazaManagerTreceri() {
    auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    const auto start = sateliti[0]->elemente().epoch();
    const orbitwatch::CoordonateGeo bucuresti(44.4268, 26.1025, 0.08);

    orbitwatch::ManagerTreceri manager(bucuresti, std::move(sateliti));
    assert(manager.numarSateliti() == 3);

    const auto& treceri = manager.calculeazaTreceri(start, std::chrono::hours{48});
    const auto filtrate = manager.filtreazaDupaElevatie(25.0);
    assert(filtrate.size() <= treceri.size());

    const auto ceaMaiBuna = manager.ceaMaiBunaTrecere();
    assert(ceaMaiBuna.has_value() == !treceri.empty());

    // Testare Rule of Three (copy constructor + operator=)
    orbitwatch::ManagerTreceri copie(manager);
    assert(copie.numarSateliti() == manager.numarSateliti());

    copie.actualizeazaObservator(orbitwatch::CoordonateGeo(46.7712, 23.6236, 0.34));
    assert(aproapeEgal(copie.observator().latitudine(), 46.7712));

    copie = manager;
    assert(aproapeEgal(copie.observator().latitudine(), 44.4268));

    std::ostringstream oss;
    oss << copie;
    assert(!oss.str().empty());
}
    void testeazaRaportTreceri() {
    auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    const auto start = sateliti[0]->elemente().epoch();
    const orbitwatch::CoordonateGeo bucuresti(44.4268, 26.1025, 0.08);

    orbitwatch::ManagerTreceri manager(bucuresti, std::move(sateliti));
    const auto& treceri = manager.calculeazaTreceri(start, std::chrono::hours{48});

    orbitwatch::RaportTreceri raport("Test Raport", bucuresti, treceri);
    assert(raport.titlu() == "Test Raport");
    assert(raport.numarTreceri() == treceri.size());
    assert(raport.durataMedieSecunde() >= 0.0);

    const auto stats = raport.statisticiPeTip();
    assert(stats.size() <= 3);

    const auto rezumat = raport.genereazaRezumatText();
    assert(!rezumat.empty());

    std::ostringstream oss;
    oss << raport;
    assert(oss.str() == rezumat);

    const auto caleCsv = std::filesystem::temp_directory_path() / "orbitwatch_test.csv";
    const auto caleTxt = std::filesystem::temp_directory_path() / "orbitwatch_test.txt";
    raport.exportaCsv(caleCsv);
    raport.exportaText(caleTxt);
    const bool existaCsv = std::filesystem::exists(caleCsv);
    const bool existaTxt = std::filesystem::exists(caleTxt);
    assert(existaCsv);
    assert(existaTxt);
    std::filesystem::remove(caleCsv);
    std::filesystem::remove(caleTxt);
}
    void testeazaCatalogSiPlanificator() {
    auto catalog = orbitwatch::CatalogLocatii::incarcaDinFisier("assets/locatii.json");
    assert(!catalog.gol());
    assert(catalog.dimensiune() >= 5);

    const auto buc = catalog.cautaDupaNume("bucuresti");
    assert(buc.has_value());
    assert(!buc->esteCerIntunecat());

    const auto cerIntunecat = catalog.filtreazaDupaBortleMaxim(3);
    assert(!cerIntunecat.empty());

    const auto celMaiIntunecat = catalog.celMaiIntunecatCer();
    assert(celMaiIntunecat.has_value());
    assert(celMaiIntunecat->esteCerIntunecat());

    const auto apropiata = catalog.ceaMaiApropiataDe(buc->coordonate());
    assert(apropiata.has_value());
    assert(apropiata->nume() == "Bucuresti");

    auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    const auto start = sateliti[0]->elemente().epoch();
    orbitwatch::ManagerTreceri manager(celMaiIntunecat->coordonate(), std::move(sateliti));
    const auto& treceri = manager.calculeazaTreceri(start, std::chrono::hours{48});

    orbitwatch::PlanificatorObservatii planificator(*celMaiIntunecat, treceri);
    assert(planificator.plan().size() == treceri.size());
    assert(planificator.scorMediu() >= 0.0);

    const auto programOptim = planificator.selecteazaProgramFaraConflicte();
    assert(programOptim.size() <= treceri.size());

    std::ostringstream oss;
    oss << catalog << planificator;
    assert(!oss.str().empty());
}
    void testeazaComparatorLocatii() {
    const auto catalog = orbitwatch::CatalogLocatii::incarcaDinFisier("assets/locatii.json");
    auto sateliti = orbitwatch::ParserDateOrbitale::incarcaDinFisier("assets/sateliti.json");
    const auto start = sateliti[0]->elemente().epoch();
    orbitwatch::ManagerTreceri manager(
        catalog.locatii().front().coordonate(), std::move(sateliti));

    orbitwatch::ComparatorLocatii comparator(
        catalog, manager, start, std::chrono::hours{48});
    assert(comparator.clasament().size() == catalog.dimensiune());

    const auto top = comparator.ceaMaiBunaLocatie();
    assert(top.has_value());
    assert(top->scorMediu() >= comparator.clasament().back().scorMediu());

    const auto eligibile = comparator.filtreazaCuTreceriMinime(1);
    assert(eligibile.size() <= comparator.clasament().size());

    std::ostringstream oss;
    oss << comparator;
    assert(!oss.str().empty());
}
}

int main() {
    try {
        testeazaCoordonate();
        testeazaElementeOrbitale();
        testeazaIerarhieSateliti();
        testeazaParserDateOrbitale();
        testeazaPropagatorSiTreceri();
        testeazaManagerTreceri();
        testeazaRaportTreceri();
        testeazaCatalogSiPlanificator();
        testeazaComparatorLocatii();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test esuat cu exceptie: " << e.what() << '\n';
        return 1;
    }
}