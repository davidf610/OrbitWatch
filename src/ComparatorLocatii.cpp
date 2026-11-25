#include "ComparatorLocatii.hpp"

#include <algorithm>
#include <iomanip>

#include "PlanificatorObservatii.hpp"

namespace orbitwatch {

RezultatComparatieLocatie::RezultatComparatieLocatie(const LocatieObservare& locatie,
                                                     std::size_t numarTreceri,
                                                     std::size_t numarTreceriFaraConflicte,
                                                     double scorMediu,
                                                     double scorMaxim)
    : locatie_(locatie),
      numarTreceri_(numarTreceri),
      numarTreceriFaraConflicte_(numarTreceriFaraConflicte),
      scorMediu_(scorMediu),
      scorMaxim_(scorMaxim) {}

std::ostream& operator<<(std::ostream& os, const RezultatComparatieLocatie& elem) {
    os << elem.locatie_.nume() << " (Bortle " << elem.locatie_.scalaBortle()
       << ") | Treceri: " << elem.numarTreceri_
       << " (fara conflicte: " << elem.numarTreceriFaraConflicte_
       << ") | Scor mediu: " << std::fixed << std::setprecision(1) << elem.scorMediu_
       << "/100 | Scor max: " << elem.scorMaxim_ << "/100";
    return os;
}

ComparatorLocatii::ComparatorLocatii(const CatalogLocatii& catalog,
                                     const ManagerTreceri& managerBaza,
                                     std::chrono::system_clock::time_point start,
                                     std::chrono::hours durata) {
    clasament_.reserve(catalog.dimensiune());

    for (const auto& loc : catalog.locatii()) {
        ManagerTreceri copie(managerBaza);
        copie.actualizeazaObservator(loc.coordonate());
        const auto& treceri = copie.calculeazaTreceri(start, durata);

        PlanificatorObservatii planificator(loc, treceri);
        const auto faraConflicte = planificator.selecteazaProgramFaraConflicte();

        double scorMax = 0.0;
        for (const auto& item : planificator.plan()) {
            if (item.scorObservare() > scorMax) {
                scorMax = item.scorObservare();
            }
        }

        clasament_.emplace_back(
            loc, treceri.size(), faraConflicte.size(), planificator.scorMediu(), scorMax);
    }

    std::sort(clasament_.begin(),
              clasament_.end(),
              [](const RezultatComparatieLocatie& a, const RezultatComparatieLocatie& b) {
                  if (a.scorMediu() != b.scorMediu()) {
                      return a.scorMediu() > b.scorMediu();
                  }
                  if (a.numarTreceri() != b.numarTreceri()) {
                      return a.numarTreceri() > b.numarTreceri();
                  }
                  return a.locatie().scalaBortle() < b.locatie().scalaBortle();
              });
}

std::optional<RezultatComparatieLocatie> ComparatorLocatii::ceaMaiBunaLocatie() const {
    if (clasament_.empty()) {
        return std::nullopt;
    }
    return clasament_.front();
}

std::vector<RezultatComparatieLocatie> ComparatorLocatii::filtreazaCuTreceriMinime(
    std::size_t minimTreceri) const {
    std::vector<RezultatComparatieLocatie> filtrate;
    for (const auto& elem : clasament_) {
        if (elem.numarTreceri() >= minimTreceri) {
            filtrate.push_back(elem);
        }
    }
    return filtrate;
}

std::ostream& operator<<(std::ostream& os, const ComparatorLocatii& comparator) {
    os << "Clasament locatii (" << comparator.clasament_.size() << "):\n";
    for (std::size_t i = 0; i < comparator.clasament_.size(); ++i) {
        os << "  " << (i + 1) << ". " << comparator.clasament_[i] << '\n';
    }
    return os;
}

} 
