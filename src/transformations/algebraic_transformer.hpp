#ifndef __ALGEBRAIC_TRANSFORMER_HPP__
#define __ALGEBRAIC_TRANSFORMER_HPP__

#include <memory>
#include "ir/cfg.hpp"

class AlgebraicTransformer {
private:
    std::shared_ptr<ControlFlowGraph> cfg;

    void processNode(IR_Node *node);

public:
    AlgebraicTransformer(std::shared_ptr<ControlFlowGraph> rawCfg);
    std::shared_ptr<ControlFlowGraph> getCfg();
};

#endif /* __ALGEBRAIC_TRANSFORMER_HPP__ */