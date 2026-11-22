#include "ElementeOrbitale.hpp"
#include "OrbitExceptii.hpp"

#include <cmath>
#include <string>

namespace orbitwatch {

    ElementeOrbitale::ElementeOrbitale(std::chrono::system_clock::time_point epoch,
                                       double inclinatie,
                                       double raan,
                                       double excentricitate,
                                       double argumentPerigeu,
                                       double anomalieMedie,
                                       double meanMotion)
        : epoch_(epoch),
          inclinatie_(inclinatie),
          raan_(raan),
          excentricitate_(excentricitate),
          argumentPerigeu_(argumentPerigeu),
          anomalieMedie_(anomalieMedie),
          meanMotion_(meanMotion) {
        if (std::isnan(inclinatie_) || inclinatie_ < 0.0 || inclinatie_ > 180.0) {
            throw EroareParsare("Inclinatie invalida: " + std::to_string(inclinatie_));
        }
        if (std::isnan(excentricitate_) || excentricitate_ < 0.0 || excentricitate_ >= 1.0) {
            throw EroareParsare("Excentricitate invalida (trebuie < 1.0): " +
                                std::to_string(excentricitate_));
        }
        if (std::isnan(meanMotion_) || meanMotion_ <= 0.0) {
            throw EroareParsare("Mean motion trebuie sa fie strict pozitiv: " +
                                std::to_string(meanMotion_));
        }
    }

}