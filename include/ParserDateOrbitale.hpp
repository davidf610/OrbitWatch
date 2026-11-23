#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "Satelit.hpp"

namespace orbitwatch {

    class ParserDateOrbitale {
    public:
        [[nodiscard]] static std::vector<std::unique_ptr<Satelit>> incarcaDinFisier(
            const std::filesystem::path& caleFisier);

        [[nodiscard]] static std::vector<std::unique_ptr<Satelit>> incarcaDinString(
            const std::string& continutJson);
    };

} 