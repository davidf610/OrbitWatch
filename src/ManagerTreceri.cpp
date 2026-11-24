#include "ManagerTreceri.hpp"

#include <algorithm>
#include <utility>

#include "PropagatorOrbital.hpp"

namespace orbitwatch {

void ManagerTreceri::sorteazaTreceri() {
    std::sort(ultimeleTreceri_.begin(),
              ultimeleTreceri_.end(),
              [](const TrecereSatelit& a, const TrecereSatelit& b) {
                  return a.oraInceput() < b.oraInceput();
              });
}

ManagerTreceri::ManagerTreceri(const CoordonateGeo& observator,
                               std::vector<std::unique_ptr<Satelit>> sateliti)
    : observator_(observator), sateliti_(std::move(sateliti)) {}

ManagerTreceri::ManagerTreceri(const ManagerTreceri& other)
    : observator_(other.observator_), ultimeleTreceri_(other.ultimeleTreceri_) {
    sateliti_.reserve(other.sateliti_.size());
    for (const auto& sat : other.sateliti_) {
        if (sat) {
            sateliti_.push_back(sat->clone());
        }
    }
}

ManagerTreceri& ManagerTreceri::operator=(const ManagerTreceri& other) {
    if (this == &other) {
        return *this;
    }
    ManagerTreceri copie(other);
    std::swap(observator_, copie.observator_);
    std::swap(sateliti_, copie.sateliti_);
    std::swap(ultimeleTreceri_, copie.ultimeleTreceri_);
    return *this;
}

ManagerTreceri::~ManagerTreceri() = default;

void ManagerTreceri::actualizeazaObservator(const CoordonateGeo& nouObservator) {
    observator_ = nouObservator;
    ultimeleTreceri_.clear();
}

const std::vector<TrecereSatelit>& ManagerTreceri::calculeazaTreceri(
    std::chrono::system_clock::time_point start,
    std::chrono::hours durata,
    std::chrono::seconds pas) {
    ultimeleTreceri_.clear();
    for (const auto& sat : sateliti_) {
        if (!sat) {
            continue;
        }
        auto treceri =
            PropagatorOrbital::gasesteTreceriVizibile(*sat, observator_, start, durata, pas);
        ultimeleTreceri_.insert(ultimeleTreceri_.end(), treceri.begin(), treceri.end());
    }
    sorteazaTreceri();
    return ultimeleTreceri_;
}

std::vector<TrecereSatelit> ManagerTreceri::filtreazaDupaElevatie(double elevatieMinima) const {
    std::vector<TrecereSatelit> rezultat;
    for (const auto& trecere : ultimeleTreceri_) {
        if (trecere.elevatieMaxima() >= elevatieMinima) {
            rezultat.push_back(trecere);
        }
    }
    return rezultat;
}

std::optional<TrecereSatelit> ManagerTreceri::ceaMaiBunaTrecere() const {
    if (ultimeleTreceri_.empty()) {
        return std::nullopt;
    }
    const auto it = std::min_element(
        ultimeleTreceri_.begin(),
        ultimeleTreceri_.end(),
        [](const TrecereSatelit& a, const TrecereSatelit& b) {
            return a.magnitudineMinima() < b.magnitudineMinima();
        });
    return *it;
}

std::ostream& operator<<(std::ostream& os, const ManagerTreceri& manager) {
    os << "Observator: " << manager.observator_ << "\nSateliti monitorizati ("
       << manager.sateliti_.size() << "):\n";
    for (const auto& sat : manager.sateliti_) {
        if (sat) {
            os << "  * " << *sat << '\n';
        }
    }
    os << "Treceri calculate (" << manager.ultimeleTreceri_.size() << "):\n";
    for (const auto& trecere : manager.ultimeleTreceri_) {
        os << "  - " << trecere << '\n';
    }
    return os;
}

} 