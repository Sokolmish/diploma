#ifndef __CFG_HPP__
#define __CFG_HPP__

#include <vector>
#include <memory>
#include <map>
#include "nodes.hpp"


class ControlFlowGraph {
public:
    struct Function {
    private:
        int id;
        int entryBlockId;
        friend class ControlFlowGraph;

    public:
        IR_StorageSpecifier storage;
        bool isInline;
        std::shared_ptr<IR_Type> fullType;

        Function clone() const;
        int getId() const;
        int getEntryBlockId() const;
        IR_TypeFunc const& getFuncType() const;
    };


private:
    int blocksCounter = 0;
    uint64_t regs_counter = 0;
    int funcsCounter = 0;

    std::map<int, IR_Block> blocks;
    std::map<int, Function> funcs;
    std::map<string_id_t, std::shared_ptr<IR_TypeStruct>> structs;

    friend class IR_Generator;
    friend class VarsVirtualizer;

public:
    ControlFlowGraph() = default;

    ControlFlowGraph(ControlFlowGraph const &oth);

    IR_Block& createBlock();
    void linkBlocks(IR_Block &prev, IR_Block &next);
    Function& createFunction(IR_StorageSpecifier stor, bool isInline, std::shared_ptr<IR_Type> fullType);
    IRval createReg(std::shared_ptr<IR_Type> type);

    /** get block by id */
    IR_Block& block(int id);
    Function& getFunction(int id);

    void printBlocks() const;
};

#endif /* __CFG_HPP__ */
