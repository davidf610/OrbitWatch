#pragma once

#include <chrono>
#include <vector>

#include "Coordonate.hpp"
#include "Satelit.hpp"
#include "TrecereSatelit.hpp"

namespace orbitwatch {

    class PropagatorOrbital {
    public:
        [[nodiscard]] static CoordonateOrizontale calculeazaPozitie(
            const Satelit& satelit,
            const CoordonateGeo& observator,
            std::chrono::system_clock::time_point moment);

        [[nodiscard]] static double calculeazaElevatieSoare(
            const CoordonateGeo& observator,
            std::chrono::system_clock::time_point moment);

        [[nodiscard]] static bool esteIluminatDeSoare(const Satelit& satelit,
                                                      std::chrono::system_clock::time_point moment);

        [[nodiscard]] static std::vector<TrecereSatelit> gasesteTreceriVizibile(
            const Satelit& satelit,
            const CoordonateGeo& observator,
            std::chrono::system_clock::time_point start,
            std::chrono::hours durata,
            std::chrono::seconds pas = std::chrono::seconds{60});
    };

} 