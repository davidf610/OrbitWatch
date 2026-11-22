#include "TelescopSpatial.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <utility>

namespace orbitwatch {

    TelescopSpatial::TelescopSpatial(int noradId, std::string nume, const ElementeOrbitale& elemente)
        : Satelit(noradId, std::move(nume), elemente) {}

    std::unique_ptr<Satelit> TelescopSpatial::clone() const {
        return std::make_unique<TelescopSpatial>(*this);
    }

    double TelescopSpatial::elevatieMinimaNecesara() const noexcept {
        return 15.0;
    }

    double TelescopSpatial::elevatieMaximaSoare() const noexcept {
        return -12.0;
    }

    double TelescopSpatial::calculeazaMagnitudine(double distantaKm, double unghiFazaGrade) const {
        constexpr double kMagnitudineStd = 2.0;
        constexpr double kDistantaRefKm = 540.0;

        const double distantaSigura = std::max(1.0, distantaKm);
        const double unghiRad = unghiFazaGrade * std::numbers::pi / 180.0;
        const double fractiuneIluminata = std::max(0.1, (1.0 + std::cos(unghiRad)) * 0.5);

        return kMagnitudineStd + 5.0 * std::log10(distantaSigura / kDistantaRefKm) -
               2.5 * std::log10(fractiuneIluminata);
    }

    std::string TelescopSpatial::tipText() const {
        return "Telescop Spatial";
    }

} 
