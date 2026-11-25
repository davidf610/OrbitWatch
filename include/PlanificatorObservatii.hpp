#pragma once

#include <chrono>
#include <map>
#include <ostream>
#include <string>
#include <vector>

#include "CatalogLocatii.hpp"
#include "TrecereSatelit.hpp"

namespace orbitwatch {

class ElementPlanObservare {
private:
    TrecereSatelit trecere_;
    double scorObservare_;
    bool areConflict_;
    std::string recomandare_;

public:
    ElementPlanObservare(const TrecereSatelit& trecere,
                         double scorObservare,
                         bool areConflict,
                         std::string recomandare);

    [[nodiscard]] const TrecereSatelit& trecere() const noexcept { return trecere_; }
    [[nodiscard]] double scorObservare() const noexcept { return scorObservare_; }
    [[nodiscard]] bool areConflict() const noexcept { return areConflict_; }
    [[nodiscard]] const std::string& recomandare() const noexcept { return recomandare_; }

    friend std::ostream& operator<<(std::ostream& os, const ElementPlanObservare& elem);
};

class PlanificatorObservatii {
private:
    LocatieObservare locatie_;
    std::vector<ElementPlanObservare> plan_;

    [[nodiscard]] double calculeazaScor(const TrecereSatelit& trecere) const noexcept;
    [[nodiscard]] static std::string genereazaRecomandare(double scor, bool vizibilOchiLiber);
    [[nodiscard]] static bool seSuprapun(const TrecereSatelit& a,
                                         const TrecereSatelit& b,
                                         std::chrono::seconds marjaReorientare);
    [[nodiscard]] static std::string cheieZiUtc(std::chrono::system_clock::time_point tp);

public:
    PlanificatorObservatii(const LocatieObservare& locatie,
                           const std::vector<TrecereSatelit>& treceri,
                           std::chrono::seconds marjaReorientare = std::chrono::seconds{120});

    [[nodiscard]] std::vector<ElementPlanObservare> selecteazaProgramFaraConflicte(
        std::chrono::seconds marjaReorientare = std::chrono::seconds{120}) const;

    [[nodiscard]] std::vector<ElementPlanObservare> filtreazaDupaScorMinim(double scorMinim) const;
    [[nodiscard]] std::map<std::string, std::size_t> numarTreceriPeZi() const;
    [[nodiscard]] std::size_t numarConflicte() const noexcept;
    [[nodiscard]] double scorMediu() const noexcept;

    [[nodiscard]] const LocatieObservare& locatie() const noexcept { return locatie_; }
    [[nodiscard]] const std::vector<ElementPlanObservare>& plan() const noexcept { return plan_; }

    friend std::ostream& operator<<(std::ostream& os, const PlanificatorObservatii& planificator);
};

} 
