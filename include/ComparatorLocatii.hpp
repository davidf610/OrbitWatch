#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <ostream>
#include <vector>

#include "CatalogLocatii.hpp"
#include "ManagerTreceri.hpp"

namespace orbitwatch {

class RezultatComparatieLocatie {
private:
    LocatieObservare locatie_;
    std::size_t numarTreceri_;
    std::size_t numarTreceriFaraConflicte_;
    double scorMediu_;
    double scorMaxim_;

public:
    RezultatComparatieLocatie(const LocatieObservare& locatie,
                              std::size_t numarTreceri,
                              std::size_t numarTreceriFaraConflicte,
                              double scorMediu,
                              double scorMaxim);

    [[nodiscard]] const LocatieObservare& locatie() const noexcept { return locatie_; }
    [[nodiscard]] std::size_t numarTreceri() const noexcept { return numarTreceri_; }
    [[nodiscard]] std::size_t numarTreceriFaraConflicte() const noexcept {
        return numarTreceriFaraConflicte_;
    }
    [[nodiscard]] double scorMediu() const noexcept { return scorMediu_; }
    [[nodiscard]] double scorMaxim() const noexcept { return scorMaxim_; }

    friend std::ostream& operator<<(std::ostream& os, const RezultatComparatieLocatie& elem);
};

class ComparatorLocatii {
private:
    std::vector<RezultatComparatieLocatie> clasament_;

public:
    ComparatorLocatii(const CatalogLocatii& catalog,
                      const ManagerTreceri& managerBaza,
                      std::chrono::system_clock::time_point start,
                      std::chrono::hours durata);

    [[nodiscard]] std::optional<RezultatComparatieLocatie> ceaMaiBunaLocatie() const;
    [[nodiscard]] std::vector<RezultatComparatieLocatie> filtreazaCuTreceriMinime(
        std::size_t minimTreceri) const;
    [[nodiscard]] const std::vector<RezultatComparatieLocatie>& clasament() const noexcept {
        return clasament_;
    }

    friend std::ostream& operator<<(std::ostream& os, const ComparatorLocatii& comparator);
};

} 