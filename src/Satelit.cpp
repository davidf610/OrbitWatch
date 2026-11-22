#include "Satelit.hpp"
#include "OrbitExceptii.hpp"

#include <utility>

namespace orbitwatch {

    Satelit::Satelit(int noradId, std::string nume, const ElementeOrbitale& elemente)
        : noradId_(noradId), nume_(std::move(nume)), elemente_(elemente) {
        if (noradId_ <= 0) {
            throw EroareParsare("NORAD ID trebuie sa fie strict pozitiv, primit: " +
                                std::to_string(noradId_));
        }
        if (nume_.empty()) {
            throw EroareParsare("Numele satelitului nu poate fi gol.");
        }
    }

    bool Satelit::poateFiObservat(const CoordonateOrizontale& pozitie,
                                  double elevatieSoareGrade,
                                  bool esteIluminatDeSoare) const {
        // 1. Conditii fizice universale:
        //    - Satelitul trebuie sa fie iluminat de Soare (sa nu fie in umbra Pamantului)
        //    - Trebuie sa fie deasupra orizontului si peste pragul specific tipului
        //    - Observatorul trebuie sa fie in intuneric/crepuscul (Soarele sub pragul specific)
        if (!esteIluminatDeSoare) {
            return false;
        }
        if (pozitie.elevatie() < elevatieMinimaNecesara()) {
            return false;
        }
        if (elevatieSoareGrade > elevatieMaximaSoare()) {
            return false;
        }
        return true;
    }

}