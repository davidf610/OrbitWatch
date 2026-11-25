#include "PlanificatorObservatii.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace orbitwatch {

ElementPlanObservare::ElementPlanObservare(const TrecereSatelit& trecere,
                                           double scorObservare,
                                           bool areConflict,
                                           std::string recomandare)
    : trecere_(trecere),
      scorObservare_(scorObservare),
      areConflict_(areConflict),
      recomandare_(std::move(recomandare)) {}

std::ostream& operator<<(std::ostream& os, const ElementPlanObservare& elem) {
    os << elem.trecere_ << " | Scor: " << std::fixed << std::setprecision(1) << elem.scorObservare_
       << "/100";
    if (elem.areConflict_) {
        os << " [CONFLICT]";
    }
    os << " | " << elem.recomandare_;
    return os;
}

double PlanificatorObservatii::calculeazaScor(const TrecereSatelit& trecere) const noexcept {
    // 1. Componenta de elevatie (0..45 puncte)
    const double scorElevatie = std::clamp(trecere.elevatieMaxima() / 90.0, 0.0, 1.0) * 45.0;

    // 2. Componenta de contrast fata de limita vizibila a locatiei (0..40 puncte)
    const double marjaMag = locatie_.magnitudineLimitaVizibila() - trecere.magnitudineMinima();
    const double scorContrast = std::clamp(marjaMag / 6.0, 0.0, 1.0) * 40.0;

    // 3. Componenta de durata (0..15 puncte, maxim la >= 5 minute)
    const auto durataSec = std::chrono::duration_cast<std::chrono::seconds>(
                               trecere.oraSfarsit() - trecere.oraInceput())
                               .count();
    const double scorDurata =
        std::clamp(static_cast<double>(durataSec) / 300.0, 0.0, 1.0) * 15.0;

    return scorElevatie + scorContrast + scorDurata;
}

std::string PlanificatorObservatii::genereazaRecomandare(double scor, bool vizibilOchiLiber) {
    if (!vizibilOchiLiber) {
        return "Necesita binoclu/telescop (sub limita cerului local)";
    }
    if (scor >= 75.0) {
        return "Prioritate maxima - conditii excelente";
    }
    if (scor >= 50.0) {
        return "Recomandat pentru observare cu ochiul liber";
    }
    return "Vizibilitate modesta - optional";
}

bool PlanificatorObservatii::seSuprapun(const TrecereSatelit& a,
                                        const TrecereSatelit& b,
                                        std::chrono::seconds marjaReorientare) {
    return a.oraInceput() < (b.oraSfarsit() + marjaReorientare) &&
           b.oraInceput() < (a.oraSfarsit() + marjaReorientare);
}

std::string PlanificatorObservatii::cheieZiUtc(std::chrono::system_clock::time_point tp) {
    const auto zile = std::chrono::floor<std::chrono::days>(tp);
    const std::chrono::year_month_day ymd{zile};
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(4) << static_cast<int>(ymd.year()) << '-' << std::setw(2)
        << static_cast<unsigned>(ymd.month()) << '-' << std::setw(2)
        << static_cast<unsigned>(ymd.day());
    return oss.str();
}

PlanificatorObservatii::PlanificatorObservatii(const LocatieObservare& locatie,
                                               const std::vector<TrecereSatelit>& treceri,
                                               std::chrono::seconds marjaReorientare)
    : locatie_(locatie) {
    plan_.reserve(treceri.size());
    const double nelm = locatie_.magnitudineLimitaVizibila();

    for (std::size_t i = 0; i < treceri.size(); ++i) {
        bool conflict = false;
        for (std::size_t j = 0; j < treceri.size(); ++j) {
            if (i != j && seSuprapun(treceri[i], treceri[j], marjaReorientare)) {
                conflict = true;
                break;
            }
        }
        const double scor = calculeazaScor(treceri[i]);
        const bool vizibil = treceri[i].magnitudineMinima() <= nelm;
        plan_.emplace_back(treceri[i], scor, conflict, genereazaRecomandare(scor, vizibil));
    }

    std::sort(plan_.begin(),
              plan_.end(),
              [](const ElementPlanObservare& a, const ElementPlanObservare& b) {
                  return a.trecere().oraInceput() < b.trecere().oraInceput();
              });
}

std::vector<ElementPlanObservare> PlanificatorObservatii::selecteazaProgramFaraConflicte(
    std::chrono::seconds marjaReorientare) const {
    // Selectie greedy dupa scor descrescator, pastrand doar trecerile fara suprapuneri
    std::vector<ElementPlanObservare> candidati = plan_;
    std::sort(candidati.begin(),
              candidati.end(),
              [](const ElementPlanObservare& a, const ElementPlanObservare& b) {
                  return a.scorObservare() > b.scorObservare();
              });

    std::vector<ElementPlanObservare> selectate;
    for (const auto& candidat : candidati) {
        bool compatibil = true;
        for (const auto& ales : selectate) {
            if (seSuprapun(candidat.trecere(), ales.trecere(), marjaReorientare)) {
                compatibil = false;
                break;
            }
        }
        if (compatibil) {
            selectate.push_back(candidat);
        }
    }

    std::sort(selectate.begin(),
              selectate.end(),
              [](const ElementPlanObservare& a, const ElementPlanObservare& b) {
                  return a.trecere().oraInceput() < b.trecere().oraInceput();
              });
    return selectate;
}

std::vector<ElementPlanObservare> PlanificatorObservatii::filtreazaDupaScorMinim(
    double scorMinim) const {
    std::vector<ElementPlanObservare> rezultat;
    for (const auto& elem : plan_) {
        if (elem.scorObservare() >= scorMinim) {
            rezultat.push_back(elem);
        }
    }
    return rezultat;
}

std::map<std::string, std::size_t> PlanificatorObservatii::numarTreceriPeZi() const {
    std::map<std::string, std::size_t> peZi;
    for (const auto& elem : plan_) {
        ++peZi[cheieZiUtc(elem.trecere().oraInceput())];
    }
    return peZi;
}

std::size_t PlanificatorObservatii::numarConflicte() const noexcept {
    std::size_t total = 0;
    for (const auto& elem : plan_) {
        if (elem.areConflict()) {
            ++total;
        }
    }
    return total;
}

double PlanificatorObservatii::scorMediu() const noexcept {
    if (plan_.empty()) {
        return 0.0;
    }
    double suma = 0.0;
    for (const auto& elem : plan_) {
        suma += elem.scorObservare();
    }
    return suma / static_cast<double>(plan_.size());
}

std::ostream& operator<<(std::ostream& os, const PlanificatorObservatii& planificator) {
    os << "Plan observatii pentru: " << planificator.locatie_ << '\n'
       << "Total treceri analizate: " << planificator.plan_.size()
       << " | Conflicte: " << planificator.numarConflicte() << " | Scor mediu: " << std::fixed
       << std::setprecision(1) << planificator.scorMediu() << "/100\n";

    for (const auto& [zi, numar] : planificator.numarTreceriPeZi()) {
        os << "  [" << zi << "]: " << numar << " treceri\n";
    }
    for (std::size_t i = 0; i < planificator.plan_.size(); ++i) {
        os << "  " << (i + 1) << ". " << planificator.plan_[i] << '\n';
    }
    return os;
}

} 