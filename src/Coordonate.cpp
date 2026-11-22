#include "Coordonate.hpp"
#include "OrbitExceptii.hpp"

#include <cmath>
#include <string>

namespace orbitwatch {

    CoordonateGeo::CoordonateGeo(double latitudine, double longitudine, double altitudineKm)
        : latitudine_(latitudine), longitudine_(longitudine), altitudineKm_(altitudineKm) {
        if (std::isnan(latitudine_) || latitudine_ < -90.0 || latitudine_ > 90.0) {
            throw CoordonateInvalide("Latitudinea trebuie sa fie in [-90, 90], primit: " +
                                     std::to_string(latitudine_));
        }
        if (std::isnan(longitudine_) || longitudine_ < -180.0 || longitudine_ > 180.0) {
            throw CoordonateInvalide("Longitudinea trebuie sa fie in [-180, 180], primit: " +
                                     std::to_string(longitudine_));
        }
        if (std::isnan(altitudineKm_) || altitudineKm_ < -0.5 || altitudineKm_ > 100.0) {
            throw CoordonateInvalide("Altitudinea trebuie sa fie in [-0.5, 100] km, primit: " +
                                     std::to_string(altitudineKm_));
        }
    }

    CoordonateOrizontale::CoordonateOrizontale(double azimut, double elevatie, double distantaKm)
        : azimut_(azimut), elevatie_(elevatie), distantaKm_(distantaKm) {
        if (std::isnan(azimut_) || azimut_ < 0.0 || azimut_ >= 360.0) {
            throw CoordonateInvalide("Azimutul trebuie sa fie in [0, 360), primit: " +
                                     std::to_string(azimut_));
        }
        if (std::isnan(elevatie_) || elevatie_ < -90.0 || elevatie_ > 90.0) {
            throw CoordonateInvalide("Elevatia trebuie sa fie in [-90, 90], primit: " +
                                     std::to_string(elevatie_));
        }
        if (std::isnan(distantaKm_) || distantaKm_ < 0.0) {
            throw CoordonateInvalide("Distanta nu poate fi negativa, primit: " +
                                     std::to_string(distantaKm_));
        }
    }

}