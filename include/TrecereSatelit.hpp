#pragma once

#include <chrono>
#include <ostream>
#include <string>

namespace orbitwatch {

    class TrecereSatelit {
    private:
        std::string numeSatelit_;
        std::string tipSatelit_;
        std::chrono::system_clock::time_point oraInceput_;
        std::chrono::system_clock::time_point oraMaxim_;
        std::chrono::system_clock::time_point oraSfarsit_;
        double elevatieMaxima_;
        double azimutMaxim_;
        double magnitudineMinima_;

    public:
        TrecereSatelit(std::string numeSatelit,
                       std::string tipSatelit,
                       std::chrono::system_clock::time_point oraInceput,
                       std::chrono::system_clock::time_point oraMaxim,
                       std::chrono::system_clock::time_point oraSfarsit,
                       double elevatieMaxima,
                       double azimutMaxim,
                       double magnitudineMinima);

        [[nodiscard]] const std::string& numeSatelit() const noexcept { return numeSatelit_; }
        [[nodiscard]] const std::string& tipSatelit() const noexcept { return tipSatelit_; }
        [[nodiscard]] std::chrono::system_clock::time_point oraInceput() const noexcept {
            return oraInceput_;
        }
        [[nodiscard]] std::chrono::system_clock::time_point oraMaxim() const noexcept {
            return oraMaxim_;
        }
        [[nodiscard]] std::chrono::system_clock::time_point oraSfarsit() const noexcept {
            return oraSfarsit_;
        }
        [[nodiscard]] double elevatieMaxima() const noexcept { return elevatieMaxima_; }
        [[nodiscard]] double azimutMaxim() const noexcept { return azimutMaxim_; }
        [[nodiscard]] double magnitudineMinima() const noexcept { return magnitudineMinima_; }

        friend std::ostream& operator<<(std::ostream& os, const TrecereSatelit& trecere);
    };

} 