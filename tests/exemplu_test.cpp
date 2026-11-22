#include <cassert>
#include <chrono>
#include <cmath>

#include "Coordonate.hpp"
#include "ElementeOrbitale.hpp"
#include "OrbitExceptii.hpp"

namespace {

    bool aproapeEgal(double a, double b, double eps = 1e-6) {
        return std::abs(a - b) < eps;
    }

    void testeazaCoordonate() {
        // 1. Caz valid (ex: Bragadiru)
        const orbitwatch::CoordonateGeo loc(44.37, 25.98, 0.08);
        assert(aproapeEgal(loc.latitudine(), 44.37));
        assert(aproapeEgal(loc.longitudine(), 25.98));
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
            15.4956   // mean motion
        );

        assert(elem.epoch() == acum);
        assert(aproapeEgal(elem.inclinatie(), 51.6416));
        assert(aproapeEgal(elem.raan(), 247.4627));
        assert(aproapeEgal(elem.excentricitate(), 0.00067));
        assert(aproapeEgal(elem.argumentPerigeu(), 130.5360));
        assert(aproapeEgal(elem.anomalieMedie(), 325.0288));
        assert(aproapeEgal(elem.meanMotion(), 15.4956));
    }

}

int main() {
    testeazaCoordonate();
    testeazaElementeOrbitale();
    return 0;
}