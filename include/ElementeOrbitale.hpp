#pragma once
#include <chrono>
#include <ostream>

namespace orbitwatch {

    class ElementeOrbitale {
    private:
        std::chrono::system_clock::time_point epoch_;
        double inclinatie_;       // grade
        double raan_;             // ascensia dreapta a nodului ascendent (grade)
        double excentricitate_;   // [0, 1)
        double argumentPerigeu_;  // grade
        double anomalieMedie_;    // grade
        double meanMotion_;       // rotatii pe zi

    public:
        ElementeOrbitale(std::chrono::system_clock::time_point epoch,
                         double inclinatie,
                         double raan,
                         double excentricitate,
                         double argumentPerigeu,
                         double anomalieMedie,
                         double meanMotion);

        [[nodiscard]] std::chrono::system_clock::time_point epoch() const noexcept { return epoch_; }
        [[nodiscard]] double inclinatie() const noexcept { return inclinatie_; }
        [[nodiscard]] double raan() const noexcept { return raan_; }
        [[nodiscard]] double excentricitate() const noexcept { return excentricitate_; }
        [[nodiscard]] double argumentPerigeu() const noexcept { return argumentPerigeu_; }
        [[nodiscard]] double anomalieMedie() const noexcept { return anomalieMedie_; }
        [[nodiscard]] double meanMotion() const noexcept { return meanMotion_; }
        friend std::ostream& operator<<(std::ostream& os, const ElementeOrbitale& elem);
    };

} 
