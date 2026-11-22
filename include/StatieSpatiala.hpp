#pragma once

#include "Satelit.hpp"

namespace orbitwatch {

    class StatieSpatiala final : public Satelit {
    public:
        StatieSpatiala(int noradId, std::string nume, const ElementeOrbitale& elemente);

        [[nodiscard]] std::unique_ptr<Satelit> clone() const override;
        [[nodiscard]] double elevatieMinimaNecesara() const noexcept override;
        [[nodiscard]] double elevatieMaximaSoare() const noexcept override;
        [[nodiscard]] double calculeazaMagnitudine(double distantaKm,
                                                   double unghiFazaGrade) const override;
        [[nodiscard]] std::string tipText() const override;
    };

}