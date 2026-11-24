#include "PropagatorOrbital.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace orbitwatch {

namespace {

constexpr double kPi = std::numbers::pi;
constexpr double kDeg2Rad = kPi / 180.0;
constexpr double kRad2Deg = 180.0 / kPi;
constexpr double kMuPamant = 398600.4418; // km^3 / s^2
constexpr double kRazaPamantKm = 6371.0;

struct Vec3 {
    double x;
    double y;
    double z;
};

double produsScalar(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double norma(const Vec3& v) {
    return std::sqrt(produsScalar(v, v));
}

double calculeazaDataIuliana(std::chrono::system_clock::time_point tp) {
    const auto secUnix =
        std::chrono::duration<double>(tp.time_since_epoch()).count();
    return 2440587.5 + secUnix / 86400.0;
}

double calculeazaGmstRad(double jd) {
    const double d = jd - 2451545.0;
    double gmstDeg = std::fmod(280.46061837 + 360.98564736629 * d, 360.0);
    if (gmstDeg < 0.0) {
        gmstDeg += 360.0;
    }
    return gmstDeg * kDeg2Rad;
}

Vec3 pozitieSatelitEci(const ElementeOrbitale& elem, std::chrono::system_clock::time_point moment) {
    const double dtSec =
        std::chrono::duration<double>(moment - elem.epoch()).count();
    const double nRadSec = elem.meanMotion() * 2.0 * kPi / 86400.0;
    const double a = std::cbrt(kMuPamant / (nRadSec * nRadSec));
    const double e = elem.excentricitate();

    double M = elem.anomalieMedie() * kDeg2Rad + nRadSec * dtSec;
    M = std::fmod(M, 2.0 * kPi);
    if (M < 0.0) {
        M += 2.0 * kPi;
    }

    // Rezolvarea ecuatiei lui Kepler: E - e*sin(E) = M
    double E = M;
    for (int iter = 0; iter < 5; ++iter) {
        E -= (E - e * std::sin(E) - M) / (1.0 - e * std::cos(E));
    }

    const double sinNu = std::sqrt(1.0 - e * e) * std::sin(E) / (1.0 - e * std::cos(E));
    const double cosNu = (std::cos(E) - e) / (1.0 - e * std::cos(E));
    const double nu = std::atan2(sinNu, cosNu);
    const double r = a * (1.0 - e * std::cos(E));

    const double u = elem.argumentPerigeu() * kDeg2Rad + nu;
    const double inc = elem.inclinatie() * kDeg2Rad;
    const double raan = elem.raan() * kDeg2Rad;

    const double cosU = std::cos(u);
    const double sinU = std::sin(u);
    const double cosInc = std::cos(inc);
    const double sinInc = std::sin(inc);
    const double cosRaan = std::cos(raan);
    const double sinRaan = std::sin(raan);

    return Vec3{
        r * (cosRaan * cosU - sinRaan * sinU * cosInc),
        r * (sinRaan * cosU + cosRaan * sinU * cosInc),
        r * (sinU * sinInc),
    };
}

Vec3 directieSoareEci(double jd) {
    const double d = jd - 2451545.0;
    const double L = (280.460 + 0.9856474 * d) * kDeg2Rad;
    const double g = (357.528 + 0.9856003 * d) * kDeg2Rad;
    const double lambda = L + (1.915 * std::sin(g) + 0.020 * std::sin(2.0 * g)) * kDeg2Rad;
    const double epsilon = (23.439 - 0.0000004 * d) * kDeg2Rad;

    return Vec3{
        std::cos(lambda),
        std::cos(epsilon) * std::sin(lambda),
        std::sin(epsilon) * std::sin(lambda),
    };
}

Vec3 eciLaEcef(const Vec3& eci, double gmstRad) {
    const double cosG = std::cos(gmstRad);
    const double sinG = std::sin(gmstRad);
    return Vec3{
        eci.x * cosG + eci.y * sinG,
        -eci.x * sinG + eci.y * cosG,
        eci.z,
    };
}

Vec3 observatorEcef(const CoordonateGeo& obs) {
    const double lat = obs.latitudine() * kDeg2Rad;
    const double lon = obs.longitudine() * kDeg2Rad;
    const double r = kRazaPamantKm + obs.altitudineKm();
    return Vec3{
        r * std::cos(lat) * std::cos(lon),
        r * std::cos(lat) * std::sin(lon),
        r * std::sin(lat),
    };
}

CoordonateOrizontale diferentaEcefLaOrizontale(const Vec3& delta, const CoordonateGeo& obs) {
    const double lat = obs.latitudine() * kDeg2Rad;
    const double lon = obs.longitudine() * kDeg2Rad;

    const double sinLat = std::sin(lat);
    const double cosLat = std::cos(lat);
    const double sinLon = std::sin(lon);
    const double cosLon = std::cos(lon);

    const double east = -sinLon * delta.x + cosLon * delta.y;
    const double north = -sinLat * cosLon * delta.x - sinLat * sinLon * delta.y + cosLat * delta.z;
    const double up = cosLat * cosLon * delta.x + cosLat * sinLon * delta.y + sinLat * delta.z;

    const double dist = norma(delta);
    const double elevatie = std::asin(std::clamp(up / std::max(1e-6, dist), -1.0, 1.0)) * kRad2Deg;
    double azimut = std::atan2(east, north) * kRad2Deg;
    if (azimut < 0.0) {
        azimut += 360.0;
    }
    if (azimut >= 360.0) {
        azimut = 0.0;
    }

    return CoordonateOrizontale(azimut, elevatie, dist);
}

} // namespace

CoordonateOrizontale PropagatorOrbital::calculeazaPozitie(
    const Satelit& satelit,
    const CoordonateGeo& observator,
    std::chrono::system_clock::time_point moment) {
    const double jd = calculeazaDataIuliana(moment);
    const double gmst = calculeazaGmstRad(jd);

    const Vec3 satEcef = eciLaEcef(pozitieSatelitEci(satelit.elemente(), moment), gmst);
    const Vec3 obsEcef = observatorEcef(observator);
    const Vec3 delta{satEcef.x - obsEcef.x, satEcef.y - obsEcef.y, satEcef.z - obsEcef.z};

    return diferentaEcefLaOrizontale(delta, observator);
}

double PropagatorOrbital::calculeazaElevatieSoare(const CoordonateGeo& observator,
                                                  std::chrono::system_clock::time_point moment) {
    const double jd = calculeazaDataIuliana(moment);
    const double gmst = calculeazaGmstRad(jd);
    const Vec3 soareEcef = eciLaEcef(directieSoareEci(jd), gmst);

    const double lat = observator.latitudine() * kDeg2Rad;
    const double lon = observator.longitudine() * kDeg2Rad;
    const double up = std::cos(lat) * std::cos(lon) * soareEcef.x +
                      std::cos(lat) * std::sin(lon) * soareEcef.y + std::sin(lat) * soareEcef.z;

    return std::asin(std::clamp(up, -1.0, 1.0)) * kRad2Deg;
}

bool PropagatorOrbital::esteIluminatDeSoare(const Satelit& satelit,
                                            std::chrono::system_clock::time_point moment) {
    const double jd = calculeazaDataIuliana(moment);
    const Vec3 satEci = pozitieSatelitEci(satelit.elemente(), moment);
    const Vec3 dirSoare = directieSoareEci(jd);

    const double proiectie = produsScalar(satEci, dirSoare);
    if (proiectie > 0.0) {
        return true;
    }

    const double distPerpPatrat = produsScalar(satEci, satEci) - proiectie * proiectie;
    return distPerpPatrat > (kRazaPamantKm * kRazaPamantKm);
}

std::vector<TrecereSatelit> PropagatorOrbital::gasesteTreceriVizibile(
    const Satelit& satelit,
    const CoordonateGeo& observator,
    std::chrono::system_clock::time_point start,
    std::chrono::hours durata,
    std::chrono::seconds pas) {
    std::vector<TrecereSatelit> treceri;
    const auto sfarsit = start + durata;

    bool inTrecere = false;
    auto oraInceput = start;
    auto oraMaxim = start;
    double elevatieMax = -90.0;
    double azimutMax = 0.0;
    double magMin = 99.0;

    for (auto t = start; t <= sfarsit; t += pas) {
        const auto poz = calculeazaPozitie(satelit, observator, t);
        const double elevSoare = calculeazaElevatieSoare(observator, t);
        const bool iluminat = esteIluminatDeSoare(satelit, t);

        if (satelit.poateFiObservat(poz, elevSoare, iluminat)) {
            // Unghi de faza aproximativ bazat pe elevatie
            const double unghiFaza = std::clamp(90.0 - poz.elevatie(), 10.0, 160.0);
            const double mag = satelit.calculeazaMagnitudine(poz.distantaKm(), unghiFaza);

            if (!inTrecere) {
                inTrecere = true;
                oraInceput = t;
                oraMaxim = t;
                elevatieMax = poz.elevatie();
                azimutMax = poz.azimut();
                magMin = mag;
            } else {
                if (poz.elevatie() > elevatieMax) {
                    elevatieMax = poz.elevatie();
                    azimutMax = poz.azimut();
                    oraMaxim = t;
                }
                if (mag < magMin) {
                    magMin = mag;
                }
            }
        } else if (inTrecere) {
            treceri.emplace_back(satelit.nume(),
                                 satelit.tipText(),
                                 oraInceput,
                                 oraMaxim,
                                 t - pas,
                                 elevatieMax,
                                 azimutMax,
                                 magMin);
            inTrecere = false;
        }
    }

    if (inTrecere) {
        treceri.emplace_back(satelit.nume(),
                             satelit.tipText(),
                             oraInceput,
                             oraMaxim,
                             sfarsit,
                             elevatieMax,
                             azimutMax,
                             magMin);
    }

    return treceri;
}

} 
