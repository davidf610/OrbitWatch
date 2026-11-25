#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "Coordonate.hpp"
#include "TrecereSatelit.hpp"

namespace orbitwatch {

    class RaportTreceri {
    private:
        std::string titlu_;
        CoordonateGeo observator_;
        std::vector<TrecereSatelit> treceri_;

        [[nodiscard]] static std::string azimutLaDirectie(double azimutDeg);
        [[nodiscard]] static std::string clasificaVizibilitate(double elevatieMax, double magMin);
        [[nodiscard]] static long long durataSecunde(const TrecereSatelit& trecere);
        [[nodiscard]] static std::string formateazaOraUtc(std::chrono::system_clock::time_point tp);

    public:
        RaportTreceri(std::string titlu,
                      const CoordonateGeo& observator,
                      std::vector<TrecereSatelit> treceri);

        [[nodiscard]] std::map<std::string, std::size_t> statisticiPeTip() const;
        [[nodiscard]] double durataMedieSecunde() const;
        [[nodiscard]] std::size_t numarTreceriDupaCalitate(const std::string& calitate) const;

        [[nodiscard]] std::string genereazaRezumatText() const;
        void exportaCsv(const std::filesystem::path& caleFisier) const;
        void exportaText(const std::filesystem::path& caleFisier) const;

        [[nodiscard]] const std::string& titlu() const noexcept { return titlu_; }
        [[nodiscard]] std::size_t numarTreceri() const noexcept { return treceri_.size(); }

        friend std::ostream& operator<<(std::ostream& os, const RaportTreceri& raport);
    };

} 
