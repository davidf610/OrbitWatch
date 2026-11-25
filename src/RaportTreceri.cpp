#include "RaportTreceri.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

#include "OrbitExceptii.hpp"

namespace orbitwatch {

std::string RaportTreceri::azimutLaDirectie(double azimutDeg) {
    static constexpr std::array<const char*, 8> kDirectii = {
        "N", "NE", "E", "SE", "S", "SV", "V", "NV"};
    double norm = std::fmod(azimutDeg, 360.0);
    if (norm < 0.0) {
        norm += 360.0;
    }
    const auto index = static_cast<std::size_t>(std::llround(norm / 45.0)) % 8;
    return kDirectii[index];
}

std::string RaportTreceri::clasificaVizibilitate(double elevatieMax, double magMin) {
    if (elevatieMax >= 45.0 && magMin <= 1.0) {
        return "Excelenta";
    }
    if (elevatieMax >= 25.0 && magMin <= 3.0) {
        return "Buna";
    }
    if (elevatieMax >= 15.0) {
        return "Moderata";
    }
    return "Slaba";
}

long long RaportTreceri::durataSecunde(const TrecereSatelit& trecere) {
    const auto diff =
        std::chrono::duration_cast<std::chrono::seconds>(trecere.oraSfarsit() - trecere.oraInceput());
    return diff.count() > 0 ? diff.count() : 0;
}

std::string RaportTreceri::formateazaOraUtc(std::chrono::system_clock::time_point tp) {
    const auto sec = std::chrono::floor<std::chrono::seconds>(tp);
    const auto zile = std::chrono::floor<std::chrono::days>(sec);
    const std::chrono::year_month_day ymd{zile};
    const std::chrono::hh_mm_ss hms{sec - zile};

    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << static_cast<int>(ymd.year()) << '-' << std::setw(2)
        << static_cast<unsigned>(ymd.month()) << '-' << std::setw(2)
        << static_cast<unsigned>(ymd.day()) << ' ' << std::setw(2) << hms.hours().count() << ':'
        << std::setw(2) << hms.minutes().count() << ':' << std::setw(2) << hms.seconds().count()
        << " UTC";
    return oss.str();
}

RaportTreceri::RaportTreceri(std::string titlu,
                             const CoordonateGeo& observator,
                             std::vector<TrecereSatelit> treceri)
    : titlu_(std::move(titlu)), observator_(observator), treceri_(std::move(treceri)) {}

std::map<std::string, std::size_t> RaportTreceri::statisticiPeTip() const {
    std::map<std::string, std::size_t> contor;
    for (const auto& trecere : treceri_) {
        ++contor[trecere.tipSatelit()];
    }
    return contor;
}

double RaportTreceri::durataMedieSecunde() const {
    if (treceri_.empty()) {
        return 0.0;
    }
    long long total = 0;
    for (const auto& trecere : treceri_) {
        total += durataSecunde(trecere);
    }
    return static_cast<double>(total) / static_cast<double>(treceri_.size());
}

std::size_t RaportTreceri::numarTreceriDupaCalitate(const std::string& calitate) const {
    std::size_t total = 0;
    for (const auto& trecere : treceri_) {
        if (clasificaVizibilitate(trecere.elevatieMaxima(), trecere.magnitudineMinima()) ==
            calitate) {
            ++total;
        }
    }
    return total;
}

std::string RaportTreceri::genereazaRezumatText() const {
    std::ostringstream oss;
    oss << "\n"
        << "RAPORT: " << titlu_ << "\n"
        << "Observator: " << observator_ << "\n"
        << "Total treceri: " << treceri_.size() << " | Durata medie: " << std::fixed
        << std::setprecision(1) << (durataMedieSecunde() / 60.0) << " min\n"
        << "\n";

    if (treceri_.empty()) {
        oss << "Nu exista treceri vizibile in acest interval.\n";
        return oss.str();
    }

    oss << "\nDistributie pe tip de satelit:\n";
    for (const auto& [tip, numar] : statisticiPeTip()) {
        oss << "  - " << tip << ": " << numar << '\n';
    }

    oss << "\nDistributie pe calitate:\n"
        << "  - Excelenta: " << numarTreceriDupaCalitate("Excelenta") << '\n'
        << "  - Buna:      " << numarTreceriDupaCalitate("Buna") << '\n'
        << "  - Moderata:  " << numarTreceriDupaCalitate("Moderata") << '\n'
        << "  - Slaba:     " << numarTreceriDupaCalitate("Slaba") << '\n';

    oss << "\nDetalii treceri:\n";
    for (std::size_t i = 0; i < treceri_.size(); ++i) {
        const auto& t = treceri_[i];
        const auto durataMin = static_cast<double>(durataSecunde(t)) / 60.0;
        const auto directie = azimutLaDirectie(t.azimutMaxim());
        const auto calitate = clasificaVizibilitate(t.elevatieMaxima(), t.magnitudineMinima());

        oss << "  " << (i + 1) << ". " << t << " | Dir: " << directie << " | Durata: " << std::fixed
            << std::setprecision(1) << durataMin << " min | Calitate: " << calitate << '\n';
    }

    return oss.str();
}

void RaportTreceri::exportaCsv(const std::filesystem::path& caleFisier) const {
    std::ofstream fisier(caleFisier);
    if (!fisier.is_open()) {
        throw EroareFisier(caleFisier.string());
    }

    fisier << "Satelit,Tip,StartUTC,MaximUTC,SfarsitUTC,ElevatieMaxDeg,AzimutMaxDeg,Directie,"
              "MagnitudineMin,DurataSec,Calitate\n";

    for (const auto& t : treceri_) {
        fisier << '"' << t.numeSatelit() << "\"," << '"' << t.tipSatelit() << "\","
               << formateazaOraUtc(t.oraInceput()) << ',' << formateazaOraUtc(t.oraMaxim()) << ','
               << formateazaOraUtc(t.oraSfarsit()) << ',' << std::fixed << std::setprecision(2)
               << t.elevatieMaxima() << ',' << t.azimutMaxim() << ','
               << azimutLaDirectie(t.azimutMaxim()) << ',' << t.magnitudineMinima() << ','
               << durataSecunde(t) << ','
               << clasificaVizibilitate(t.elevatieMaxima(), t.magnitudineMinima()) << '\n';
    }
}

void RaportTreceri::exportaText(const std::filesystem::path& caleFisier) const {
    std::ofstream fisier(caleFisier);
    if (!fisier.is_open()) {
        throw EroareFisier(caleFisier.string());
    }
    fisier << genereazaRezumatText();
}

std::ostream& operator<<(std::ostream& os, const RaportTreceri& raport) {
    os << raport.genereazaRezumatText();
    return os;
}

}
