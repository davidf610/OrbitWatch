#include "TrecereSatelit.hpp"

#include <iomanip>
#include <utility>

namespace orbitwatch {

namespace {

void scrieOraUtc(std::ostream& os, std::chrono::system_clock::time_point tp) {
    const auto sec = std::chrono::floor<std::chrono::seconds>(tp);
    const auto zile = std::chrono::floor<std::chrono::days>(sec);
    const std::chrono::year_month_day ymd{zile};
    const std::chrono::hh_mm_ss hms{sec - zile};

    os << std::setfill('0') << std::setw(4) << static_cast<int>(ymd.year()) << '-' << std::setw(2)
       << static_cast<unsigned>(ymd.month()) << '-' << std::setw(2)
       << static_cast<unsigned>(ymd.day()) << ' ' << std::setw(2) << hms.hours().count() << ':'
       << std::setw(2) << hms.minutes().count() << ':' << std::setw(2) << hms.seconds().count()
       << " UTC";
}

}
    
TrecereSatelit::TrecereSatelit(std::string numeSatelit,
                               std::string tipSatelit,
                               std::chrono::system_clock::time_point oraInceput,
                               std::chrono::system_clock::time_point oraMaxim,
                               std::chrono::system_clock::time_point oraSfarsit,
                               double elevatieMaxima,
                               double azimutMaxim,
                               double magnitudineMinima)
    : numeSatelit_(std::move(numeSatelit)),
      tipSatelit_(std::move(tipSatelit)),
      oraInceput_(oraInceput),
      oraMaxim_(oraMaxim),
      oraSfarsit_(oraSfarsit),
      elevatieMaxima_(elevatieMaxima),
      azimutMaxim_(azimutMaxim),
      magnitudineMinima_(magnitudineMinima) {}

std::ostream& operator<<(std::ostream& os, const TrecereSatelit& trecere) {
    os << "[" << trecere.tipSatelit_ << "] " << trecere.numeSatelit_ << " | Start: ";
    scrieOraUtc(os, trecere.oraInceput_);
    os << " | Max: " << std::fixed << std::setprecision(1) << trecere.elevatieMaxima_
       << " deg (Az " << trecere.azimutMaxim_ << " deg) | Mag: " << std::setprecision(2)
       << trecere.magnitudineMinima_;
    return os;
}

}