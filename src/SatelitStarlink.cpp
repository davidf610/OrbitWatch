#include "SatelitStarlink.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

namespace orbitwatch {

    SatelitStarlink::SatelitStarlink(int noradId,
                                     std::string nume,
                                     const ElementeOrbitale& elemente,
                                     bool inTrenLansare)
        : Satelit(noradId, std::move(nume), elemente), inTrenLansare_(inTrenLansare) {}

    std::unique_ptr<Satelit> SatelitStarlink::clone() const {
        return std::make_unique<SatelitStarlink>(*this);
    }

    double SatelitStarlink::elevatieMinimaNecesara() const noexcept {
        return 20.0;
    }

    double SatelitStarlink::elevatieMaximaSoare() const noexcept {
        return -10.0;
    }

    double SatelitStarlink::calculeazaMagnitudine(double distantaKm, double unghiFazaGrade) const {
        constexpr double kDistantaRefKm = 550.0;
        const double magnitudineStd = inTrenLansare_ ? 1.5 : 4.8;

        const double distantaSigura = std::max(1.0, distantaKm);
        const double unghiRad = unghiFazaGrade * std::numbers::pi / 180.0;
        const double fractiuneIluminata = std::max(0.1, (1.0 + std::cos(unghiRad)) * 0.5);

        return magnitudineStd + 5.0 * std::log10(distantaSigura / kDistantaRefKm) -
               2.5 * std::log10(fractiuneIluminata);
    }

    std::string SatelitStarlink::tipText() const {
        return inTrenLansare_ ? "Starlink (Tren Lansare)" : "Starlink";
    }

}
