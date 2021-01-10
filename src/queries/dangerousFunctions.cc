#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void wasmati::VulnerabilityChecker::DangerousFunctions() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> dangFuncs(config.at(DANGEROUS_FUNCTIONS).begin(),
                                    config.at(DANGEROUS_FUNCTIONS).end());
    for (auto func : Query::functions()) {
        auto dangFunc = Predicate()
                            .instType(ExprType::Call)
                            .TEST(dangFuncs.count(node->label()) == 1);

        NodeStream(func).instructions(dangFunc).forEach([&](Node* node) {
            vulns.emplace_back(VulnType::DangFunc, func->name(), node->label(),
                               "");
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["dangerousFunctions"] = time;
}
