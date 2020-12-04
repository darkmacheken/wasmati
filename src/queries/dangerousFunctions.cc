#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void wasmati::VulnerabilityChecker::DangerousFunctions() {
    std::set<std::string> dangFuncs(config.at(DANGEROUS_FUNCTIONS).begin(),
        config.at(DANGEROUS_FUNCTIONS).end());
    for (auto func : Query::functions()) {
        auto dangFunc = Predicate()
            .instType(ExprType::Call)
            .TEST(dangFuncs.count(node->label()) == 1);

        NodeStream(func).instructions(dangFunc).forEach([&](Node* node) {
            vulns.emplace_back(VulnType::BufferOverflow, func->name(),
                node->label(), "");
            });
    }
}