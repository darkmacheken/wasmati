#include "src/query.h"
#include "src/vulns.h"
using namespace wasmati;

std::pair<Index, std::map<Index, Index>> VulnerabilityChecker::checkBufferSizes(
    Node* func) {
    // Find SUB
    Index val = 0;
    auto subPred = Predicate()
                       .instType(InstType::Binary)
                       .opcode(Opcode::I32Sub)
                       .inPDGEdge("$g0", PDGType::Global)
                       .pdgConstEdgeU32(val)
                       .TEST(val > 0);

    auto subInst = NodeStream(func).instructions(subPred).findFirst();

    if (!subInst.isPresent()) {
        return std::make_pair(0, std::map<Index, Index>());
    }

    Index sizeAlloc = 0;
    Predicate().pdgConstEdgeU32(sizeAlloc).evaluate(subInst.get());

    // FIND ADDS
    std::set<Index> setBuffs;
    setBuffs.insert(0);
    setBuffs.insert(sizeAlloc);
    auto addPred = Predicate()
                       .instType(InstType::Binary)
                       .opcode(Opcode::I32Add)
                       .inPDGEdge("$g0", PDGType::Global)
                       .pdgConstEdgeU32(val)
                       .TEST(val > 0)
                       .EXEC(setBuffs.insert(val));
    NodeStream(func).instructions(addPred);

    // Calculate difference
    std::vector<Index> buffs(setBuffs.begin(), setBuffs.end());
    std::adjacent_difference(buffs.begin(), buffs.end(), buffs.begin());

    std::map<Index, Index> result;
    Index i = 1;  // starts at 1 cuz index 0 = 0
    for (Index buff : setBuffs) {
        if (i < buffs.size()) {
            result[buff] = buffs[i];
            i++;
        } else {
            result[buff] = 0;
        }
    }

    return std::make_pair(sizeAlloc, result);
}
