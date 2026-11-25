#pragma once

#include <filesystem>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "Coordonate.hpp"

namespace orbitwatch {

class LocatieObservare {
private:
    std::string nume_;
    std::string tara_;
    CoordonateGeo coordonate_;
    int scalaBortle_;

    [[nodiscard]] static std::string descriereBortle(int bortle);

public:
    LocatieObservare(std::string nume,
                     std::string tara,
                     const CoordonateGeo& coordonate,
                     int scalaBortle);

    [[nodiscard]] double magnitudineLimitaVizibila() const noexcept;
    [[nodiscard]] double distantaPanaLaKm(const CoordonateGeo& tinta) const noexcept;
    [[nodiscard]] bool esteCerIntunecat() const noexcept { return scalaBortle_ <= 4; }

    [[nodiscard]] const std::string& nume() const noexcept { return nume_; }
    [[nodiscard]] const std::string& tara() const noexcept { return tara_; }
    [[nodiscard]] const CoordonateGeo& coordonate() const noexcept { return coordonate_; }
    [[nodiscard]] int scalaBortle() const noexcept { return scalaBortle_; }

    friend std::ostream& operator<<(std::ostream& os, const LocatieObservare& locatie);
};

class CatalogLocatii {
private:
    std::vector<LocatieObservare> locatii_;

    [[nodiscard]] static std::string normalizeazaText(const std::string& text);

public:
    CatalogLocatii() = default;
    explicit CatalogLocatii(std::vector<LocatieObservare> locatii);

    [[nodiscard]] static CatalogLocatii incarcaDinFisier(const std::filesystem::path& caleFisier);
    [[nodiscard]] std::optional<LocatieObservare> cautaDupaNume(const std::string& nume) const;
    [[nodiscard]] std::vector<LocatieObservare> filtreazaDupaBortleMaxim(int bortleMax) const;
    [[nodiscard]] std::optional<LocatieObservare> ceaMaiApropiataDe(
        const CoordonateGeo& referinta) const;
    [[nodiscard]] std::optional<LocatieObservare> celMaiIntunecatCer() const;

    [[nodiscard]] const std::vector<LocatieObservare>& locatii() const noexcept { return locatii_; }
    [[nodiscard]] std::size_t dimensiune() const noexcept { return locatii_.size(); }
    [[nodiscard]] bool gol() const noexcept { return locatii_.empty(); }

    friend std::ostream& operator<<(std::ostream& os, const CatalogLocatii& catalog);
};

}