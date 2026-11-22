#pragma once

#include "Satelit.hpp"

namespace orbitwatch {

    class SatelitStarlink final : public Satelit {
    private:
        bool inTrenLansare_;

    public:
        SatelitStarlink(int noradId,
                        std::string nume,
                        const ElementeOrbitale& elemente,
                        bool inTrenLansare = false);

        [[nodiscard]] std::unique_ptr<Satelit> clone() const override;
        [[nodiscard]] double elevatieMinimaNecesara() const noexcept override;
        [[nodiscard]] double elevatieMaximaSoare() const noexcept override;
        [[nodiscard]] double calculeazaMagnitudine(double distantaKm,
                                                   double unghiFazaGrade) const override;
        [[nodiscard]] std::string tipText() const override;

        [[nodiscard]] bool esteInTrenLansare() const noexcept { return inTrenLansare_; }
    };

} // namespace orbitwatch