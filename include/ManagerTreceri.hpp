#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <ostream>
#include <vector>

#include "Coordonate.hpp"
#include "Satelit.hpp"
#include "TrecereSatelit.hpp"

namespace orbitwatch {

    class ManagerTreceri {
    private:
        CoordonateGeo observator_;
        std::vector<std::unique_ptr<Satelit>> sateliti_;
        std::vector<TrecereSatelit> ultimeleTreceri_;

        void sorteazaTreceri();

    public:
        ManagerTreceri(const CoordonateGeo& observator,
                       std::vector<std::unique_ptr<Satelit>> sateliti);

        // Rule of Three
        ManagerTreceri(const ManagerTreceri& other);
        ManagerTreceri& operator=(const ManagerTreceri& other);
        ~ManagerTreceri();

        void actualizeazaObservator(const CoordonateGeo& nouObservator);

        const std::vector<TrecereSatelit>& calculeazaTreceri(
            std::chrono::system_clock::time_point start,
            std::chrono::hours durata,
            std::chrono::seconds pas = std::chrono::seconds{60});

        [[nodiscard]] std::vector<TrecereSatelit> filtreazaDupaElevatie(
            double elevatieMinima) const;

        [[nodiscard]] std::optional<TrecereSatelit> ceaMaiBunaTrecere() const;

        [[nodiscard]] const CoordonateGeo& observator() const noexcept { return observator_; }
        [[nodiscard]] std::size_t numarSateliti() const noexcept { return sateliti_.size(); }

        friend std::ostream& operator<<(std::ostream& os, const ManagerTreceri& manager);
    };

} 