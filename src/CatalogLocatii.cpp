#include "CatalogLocatii.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <numbers>
#include <utility>

#include <json.hpp>

#include "OrbitExceptii.hpp"

namespace orbitwatch {

namespace {

constexpr double kRazaPamantKm = 6371.0;

double gradeLaRadiani(double grade) noexcept {
    return grade * std::numbers::pi / 180.0;
}

} // namespace

std::string LocatieObservare::descriereBortle(int bortle) {
    if (bortle <= 2) {
        return "Cer excelent (fara poluare luminoasa)";
    }
    if (bortle <= 4) {
        return "Cer rural / tranzitie";
    }
    if (bortle <= 6) {
        return "Cer suburban";
    }
    return "Cer urban (poluare luminoasa ridicata)";
}

LocatieObservare::LocatieObservare(std::string nume,
                                   std::string tara,
                                   const CoordonateGeo& coordonate,
                                   int scalaBortle)
    : nume_(std::move(nume)),
      tara_(std::move(tara)),
      coordonate_(coordonate),
      scalaBortle_(scalaBortle) {
    if (nume_.empty()) {
        throw EroareParsare("Numele locatiei nu poate fi gol");
    }
    if (tara_.empty()) {
        throw EroareParsare("Tara locatiei nu poate fi goala");
    }
    if (scalaBortle_ < 1 || scalaBortle_ > 9) {
        throw EroareParsare("Scala Bortle trebuie sa fie in intervalul [1, 9]: " +
                            std::to_string(scalaBortle_));
    }
}

double LocatieObservare::magnitudineLimitaVizibila() const noexcept {
    // Aproximare standard NELM (Naked-Eye Limiting Magnitude) in functie de scala Bortle
    static constexpr double kNelm[] = {7.6, 7.1, 6.6, 6.2, 5.6, 5.1, 4.6, 4.1, 3.6};
    return kNelm[scalaBortle_ - 1];
}

double LocatieObservare::distantaPanaLaKm(const CoordonateGeo& tinta) const noexcept {
    const double lat1 = gradeLaRadiani(coordonate_.latitudine());
    const double lat2 = gradeLaRadiani(tinta.latitudine());
    const double dLat = gradeLaRadiani(tinta.latitudine() - coordonate_.latitudine());
    const double dLon = gradeLaRadiani(tinta.longitudine() - coordonate_.longitudine());

    const double sinLat = std::sin(dLat / 2.0);
    const double sinLon = std::sin(dLon / 2.0);
    const double a = sinLat * sinLat + std::cos(lat1) * std::cos(lat2) * sinLon * sinLon;
    const double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return kRazaPamantKm * c;
}

std::ostream& operator<<(std::ostream& os, const LocatieObservare& locatie) {
    os << locatie.nume_ << ", " << locatie.tara_ << " | " << locatie.coordonate_
       << " | Bortle " << locatie.scalaBortle_ << " ("
       << LocatieObservare::descriereBortle(locatie.scalaBortle_) << ", NELM ~" << std::fixed
       << std::setprecision(1) << locatie.magnitudineLimitaVizibila() << ")";
    return os;
}

std::string CatalogLocatii::normalizeazaText(const std::string& text) {
    std::string rezultat;
    rezultat.reserve(text.size());
    for (unsigned char ch : text) {
        rezultat.push_back(static_cast<char>(std::tolower(ch)));
    }
    return rezultat;
}

CatalogLocatii::CatalogLocatii(std::vector<LocatieObservare> locatii)
    : locatii_(std::move(locatii)) {}

CatalogLocatii CatalogLocatii::incarcaDinFisier(const std::filesystem::path& caleFisier)
{
    std::ifstream fisier(caleFisier);
    if (!fisier.is_open()) {
        throw EroareFisier(caleFisier.string());
    }

    nlohmann::json date;
    try {
        fisier >> date;
    } catch (const nlohmann::json::exception& e) {
        throw EroareParsare(std::string("JSON invalid in catalogul de locatii: ") + e.what());
    }

    if (!date.is_array()) {
        throw EroareParsare("Catalogul de locatii trebuie sa fie un array JSON");
    }

    std::vector<LocatieObservare> rezultat;
    rezultat.reserve(date.size());

    for (const auto& item : date) {
        try {
            const auto nume = item.at("nume").get<std::string>();
            const auto tara = item.at("tara").get<std::string>();
            const auto lat = item.at("latitudine").get<double>();
            const auto lon = item.at("longitudine").get<double>();
            const auto alt = item.at("altitudine_km").get<double>();
            const auto bortle = item.at("bortle").get<int>();

            rezultat.emplace_back(nume, tara, CoordonateGeo(lat, lon, alt), bortle);
        } catch (const nlohmann::json::exception& e) {
            throw EroareParsare(std::string("Camp lipsa sau invalid in locatii.json: ") +
                                e.what());
        }
    }

    return CatalogLocatii(std::move(rezultat));
}

std::optional<LocatieObservare> CatalogLocatii::cautaDupaNume(const std::string& nume) const {
    const std::string tinta = normalizeazaText(nume);
    for (const auto& locatie : locatii_) {
        if (normalizeazaText(locatie.nume()) == tinta) {
            return locatie;
        }
    }
    return std::nullopt;
}

std::vector<LocatieObservare> CatalogLocatii::filtreazaDupaBortleMaxim(int bortleMax) const {
    std::vector<LocatieObservare> rezultat;
    for (const auto& locatie : locatii_) {
        if (locatie.scalaBortle() <= bortleMax) {
            rezultat.push_back(locatie);
        }
    }
    return rezultat;
}

std::optional<LocatieObservare> CatalogLocatii::ceaMaiApropiataDe(
    const CoordonateGeo& referinta) const {
    if (locatii_.empty()) {
        return std::nullopt;
    }
    const auto it = std::min_element(
        locatii_.begin(),
        locatii_.end(),
        [&referinta](const LocatieObservare& a, const LocatieObservare& b) {
            return a.distantaPanaLaKm(referinta) < b.distantaPanaLaKm(referinta);
        });
    return *it;
}

std::optional<LocatieObservare> CatalogLocatii::celMaiIntunecatCer() const {
    if (locatii_.empty()) {
        return std::nullopt;
    }
    const auto it = std::min_element(
        locatii_.begin(),
        locatii_.end(),
        [](const LocatieObservare& a, const LocatieObservare& b) {
            return a.scalaBortle() < b.scalaBortle();
        });
    return *it;
}

std::ostream& operator<<(std::ostream& os, const CatalogLocatii& catalog) {
    os << "Catalog locatii (" << catalog.locatii_.size() << "):\n";
    for (std::size_t i = 0; i < catalog.locatii_.size(); ++i) {
        os << "  " << (i + 1) << ". " << catalog.locatii_[i] << '\n';
    }
    return os;
}

}
