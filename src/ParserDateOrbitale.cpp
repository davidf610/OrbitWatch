#include "ParserDateOrbitale.hpp"

#include <chrono>
#include <fstream>
#include <json.hpp>
#include <sstream>

#include "ElementeOrbitale.hpp"
#include "OrbitExceptii.hpp"
#include "SatelitStarlink.hpp"
#include "StatieSpatiala.hpp"
#include "TelescopSpatial.hpp"

namespace orbitwatch {

namespace {

std::chrono::system_clock::time_point parseazaEpoch(const std::string& epochStr) {
    int an = 0;
    int luna = 0;
    int zi = 0;
    int ora = 0;
    int minut = 0;
    int secunda = 0;
    char sep1 = 0;
    char sep2 = 0;
    char sepT = 0;
    char sep3 = 0;
    char sep4 = 0;

    std::istringstream ss(epochStr);
    if (!(ss >> an >> sep1 >> luna >> sep2 >> zi >> sepT >> ora >> sep3 >> minut >> sep4 >>
          secunda) ||
        sep1 != '-' || sep2 != '-' || sepT != 'T' || sep3 != ':' || sep4 != ':') {
        throw EroareParsare("Format EPOCH invalid: " + epochStr);
    }

    if (ora < 0 || ora > 23 || minut < 0 || minut > 59 || secunda < 0 || secunda > 59) {
        throw EroareParsare("Ora invalida in EPOCH: " + epochStr);
    }

    const std::chrono::year_month_day ymd{
        std::chrono::year{an},
        std::chrono::month{static_cast<unsigned>(luna)},
        std::chrono::day{static_cast<unsigned>(zi)},
    };
    if (!ymd.ok()) {
        throw EroareParsare("Data calendaristica invalida in EPOCH: " + epochStr);
    }

    const auto timp = std::chrono::sys_days{ymd} + std::chrono::hours{ora} +
                      std::chrono::minutes{minut} + std::chrono::seconds{secunda};
    return std::chrono::system_clock::time_point{timp};
}

} 

std::vector<std::unique_ptr<Satelit>> ParserDateOrbitale::incarcaDinFisier(
    const std::filesystem::path& caleFisier) {
    std::ifstream fisier(caleFisier);
    if (!fisier.is_open()) {
        throw EroareFisier(caleFisier.string());
    }

    std::ostringstream buffer;
    buffer << fisier.rdbuf();
    return incarcaDinString(buffer.str());
}

std::vector<std::unique_ptr<Satelit>> ParserDateOrbitale::incarcaDinString(
    const std::string& continutJson) {
    nlohmann::json date;
    try {
        date = nlohmann::json::parse(continutJson);
    } catch (const nlohmann::json::parse_error& e) {
        throw EroareParsare(std::string("JSON malformat: ") + e.what());
    }

    if (!date.is_array()) {
        throw EroareParsare("Radacina JSON trebuie sa fie o lista (array) de sateliti.");
    }

    std::vector<std::unique_ptr<Satelit>> rezultat;
    rezultat.reserve(date.size());

    for (const auto& item : date) {
        try {
            const std::string nume = item.at("OBJECT_NAME").get<std::string>();
            const int noradId = item.at("NORAD_CAT_ID").get<int>();
            const std::string tip = item.at("TIP_SATELIT").get<std::string>();
            const auto epoch = parseazaEpoch(item.at("EPOCH").get<std::string>());

            const ElementeOrbitale elem(
                epoch,
                item.at("INCLINATION").get<double>(),
                item.at("RA_OF_ASC_NODE").get<double>(),
                item.at("ECCENTRICITY").get<double>(),
                item.at("ARG_OF_PERICENTER").get<double>(),
                item.at("MEAN_ANOMALY").get<double>(),
                item.at("MEAN_MOTION").get<double>());

            if (tip == "STATIE_SPATIALA") {
                rezultat.push_back(std::make_unique<StatieSpatiala>(noradId, nume, elem));
            } else if (tip == "STARLINK") {
                const bool inTren = item.value("IN_TREN_LANSARE", false);
                rezultat.push_back(std::make_unique<SatelitStarlink>(noradId, nume, elem, inTren));
            } else if (tip == "TELESCOP_SPATIAL") {
                rezultat.push_back(std::make_unique<TelescopSpatial>(noradId, nume, elem));
            } else {
                throw EroareParsare("Tip satelit necunoscut: " + tip);
            }
        } catch (const nlohmann::json::exception& e) {
            throw EroareParsare(std::string("Camp lipsa sau tip invalid in JSON: ") + e.what());
        }
    }

    return rezultat;
}

}
