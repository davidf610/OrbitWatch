#pragma once

#include <ostream>

namespace orbitwatch {

    class CoordonateGeo {
    private:
        double latitudine_;   // [-90.0, 90.0] grade
        double longitudine_;  // [-180.0, 180.0] grade
        double altitudineKm_; // km deasupra nivelului marii

    public:
        CoordonateGeo(double latitudine, double longitudine, double altitudineKm = 0.0);

        [[nodiscard]] double latitudine() const noexcept { return latitudine_; }
        [[nodiscard]] double longitudine() const noexcept { return longitudine_; }
        [[nodiscard]] double altitudineKm() const noexcept { return altitudineKm_; }
        friend std::ostream& operator<<(std::ostream& os, const CoordonateGeo& coord);
    };

    class CoordonateOrizontale {
    private:
        double azimut_;     // [0.0, 360.0) grade (0 = Nord, 90 = Est)
        double elevatie_;   // [-90.0, 90.0] grade (> 0 = deasupra orizontului)
        double distantaKm_; // distanta de la observator la satelit

    public:
        CoordonateOrizontale(double azimut, double elevatie, double distantaKm);

        [[nodiscard]] double azimut() const noexcept { return azimut_; }
        [[nodiscard]] double elevatie() const noexcept { return elevatie_; }
        [[nodiscard]] double distantaKm() const noexcept { return distantaKm_; }
        friend std::ostream& operator<<(std::ostream& os, const CoordonateOrizontale& coord);
    };

}

