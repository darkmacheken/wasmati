#include "src/graph.h"
#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

void wasmati::VulnerabilityChecker::DangerousFunctions() {
    auto start = std::chrono::high_resolution_clock::now();
    std::set<std::string> dangFuncs(config.at(DANGEROUS_FUNCTIONS).begin(),
                                    config.at(DANGEROUS_FUNCTIONS).end());

    // Get all nodes
    for (auto func : Query::functions()) {
        auto dangFunc = Predicate()
                            .instType(InstType::Call)
                            .TEST(dangFuncs.count(node->label()) == 1);

        NodeStream(func).instructions(dangFunc).forEach([&](Node* node) {
            NodeSet set;
            set.insert(node);

            auto fullPaths = Query::BFS_paths(set, Query::ALL_NODES,
            [&](Edge *e) {
              return e->type() == EdgeType::CFG || e->type() == EdgeType::CG || (e->dest()->_type == NodeType::Instructions && e->type() == EdgeType::AST);
            });

            std::list<json> paths;

            for(GraphPath path: *fullPaths) {
              paths.push_back(path.toJson());
            }

            json j = json{
                {"type", VulnType::DangFunc},
                {"function", func->name()},
                {"caller", node->label()},
                {
                  "paths", paths
                },
            };

            vulns.emplace_back(VulnType::DangFunc, j);
        });
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto time =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    info["dangerousFunctions"] = time;
}
