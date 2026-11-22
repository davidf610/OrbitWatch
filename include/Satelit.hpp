#pragma once

#include <memory>
#include <string>

#include "Coordonate.hpp"
#include "ElementeOrbitale.hpp"

namespace orbitwatch {

    class Satelit {
    private:
        int noradId_;
        std::string nume_;
        ElementeOrbitale elemente_;

    protected:
        // Protected pentru a preveni object slicing la copiere prin valoare,
        // permitand totodata derivatelor sa isi implementeze metoda clone().
        Satelit(const Satelit&) = default;
        Satelit& operator=(const Satelit&) = default;

    public:
        Satelit(int noradId, std::string nume, const ElementeOrbitale& elemente);
        virtual ~Satelit() = default;

        // Copiere polimorfica
        [[nodiscard]] virtual std::unique_ptr<Satelit> clone() const = 0;

        // Interfata NVI (Non-Virtual Interface): combina conditiile universale
        // cu pragurile specifice fiecarui tip de satelit.
        [[nodiscard]] bool poateFiObservat(const CoordonateOrizontale& pozitie,
                                           double elevatieSoareGrade,
                                           bool esteIluminatDeSoare) const;

        // Metode virtuale pure care definesc comportamentul specific derivatelor
        [[nodiscard]] virtual double elevatieMinimaNecesara() const noexcept = 0;
        [[nodiscard]] virtual double elevatieMaximaSoare() const noexcept = 0;
        [[nodiscard]] virtual double calculeazaMagnitudine(double distantaKm,
                                                           double unghiFazaGrade) const = 0;
        [[nodiscard]] virtual std::string tipText() const = 0;

        // Getteri (doar cei folositi efectiv in aplicatie/teste)
        [[nodiscard]] int noradId() const noexcept { return noradId_; }
        [[nodiscard]] const std::string& nume() const noexcept { return nume_; }
        [[nodiscard]] const ElementeOrbitale& elemente() const noexcept { return elemente_; }
    };

} 