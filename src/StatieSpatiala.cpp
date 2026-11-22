#include "StatieSpatiala.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

namespace orbitwatch {

    StatieSpatiala::StatieSpatiala(int noradId, std::string nume, const ElementeOrbitale& elemente)
        : Satelit(noradId, std::move(nume), elemente) {}

    std::unique_ptr<Satelit> StatieSpatiala::clone() const {
        return std::make_unique<StatieSpatiala>(*this);
    }

    double StatieSpatiala::elevatieMinimaNecesara() const noexcept {
        return 10.0;
    }

    double StatieSpatiala::elevatieMaximaSoare() const noexcept {
        return -6.0;
    }

    double StatieSpatiala::calculeazaMagnitudine(double distantaKm, double unghiFazaGrade) const {
        constexpr double kMagnitudineStd = -1.8;
        constexpr double kDistantaRefKm = 400.0;

        const double distantaSigura = std::max(1.0, distantaKm);
        const double unghiRad = unghiFazaGrade * std::numbers::pi / 180.0;
        const double fractiuneIluminata = std::max(0.1, (1.0 + std::cos(unghiRad)) * 0.5);

        return kMagnitudineStd + 5.0 * std::log10(distantaSigura / kDistantaRefKm) -
               2.5 * std::log10(fractiuneIluminata);
    }

    std::string StatieSpatiala::tipText() const {
        return "Statie Spatiala";
    }

} // namespace orbitwatch//
// Created by floroiu on 11/23/26.
//
