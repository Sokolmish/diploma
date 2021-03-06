#include "graph_info.hpp"
#include <stack>
#include <sstream>
#include <fmt/ostream.h>
#include "utils.hpp"

GraphInfo::UtilNode::UtilNode(int id, DomNode *dom) : id(id), dom(dom) {}

GraphInfo::GraphInfo(CFGraph const &graph) : cfg(graph) {
    auto domIt = domData.emplace(cfg.entryBlockId, DomNode()).first;
    UtilNode unode = UtilNode(cfg.entryBlockId, &domIt->second);
    auto utilIt = utilNodes.emplace(cfg.entryBlockId, std::move(unode)).first;
    UtilNode &newNode = utilIt->second;
    dfs(newNode);

    classifyArcs();

    std::sort(funBlocksIds.begin(), funBlocksIds.end(), [this](int a, int b) {
        return utilNodes.at(a).timeIn < utilNodes.at(b).timeIn;
    });
    dominators(funBlocksIds);
    domData.at(cfg.entryBlockId).dominatorId = -1;

    funBlocksIds.clear();
    utilNodes.clear();

    for (auto &[nodeId, node] : domData) {
        if (node.dominatorId != -1)
            domData.at(node.dominatorId).childrenIds.push_back(nodeId);
    }
}

void GraphInfo::dfs(UtilNode &node) {
    funBlocksIds.push_back(node.id);
    node.timeIn = globalTime++;

    IR_Block const &block = cfg.block(node.id);
    for (int nextId : block.next) {
        auto it = utilNodes.lower_bound(nextId);
        if (it == utilNodes.end() || it->first != nextId) {
            arcsClasses.emplace(std::make_pair(node.id, nextId), GraphInfo::TREE);

            auto &domNode = domData.emplace(nextId, DomNode()).first->second;
            auto &newNode = utilNodes.emplace_hint(it, nextId, UtilNode(nextId, &domNode))->second;

            newNode.parent = &node;
            dfs(newNode);
        }
        else {
            unprocArcs.emplace_back(&node, &it->second);
        }
    }
    node.timeOut = globalTime++;
}

void GraphInfo::classifyArcs() {
    for (auto &[from, to] : unprocArcs) {
        if (from->timeIn < to->timeIn && to->timeOut < from->timeOut)
            arcsClasses.emplace(std::make_pair(from->id, to->id), GraphInfo::FWD);
        else if (to->timeIn < from->timeIn && from->timeOut < to->timeOut)
            arcsClasses.emplace(std::make_pair(from->id, to->id), GraphInfo::BACK);
        else // ? to->timeOut < from->timeIn
            arcsClasses.emplace(std::make_pair(from->id, to->id), GraphInfo::CROSS);
    }
    unprocArcs.clear();
}

GraphInfo::UtilNode& GraphInfo::findMin(UtilNode &node) {
    if (node.ancestor == nullptr)
        return node;

    std::stack<UtilNode*> stack;
    UtilNode *uNode = &node;
    while (uNode->ancestor->ancestor != nullptr) {
        stack.push(uNode);
        uNode = uNode->ancestor;
    }

    UtilNode *curNode = &node;
    UtilNode *lastLabel = curNode->label; // Suppress CLion bug
    while (!stack.empty()) {
        curNode = stack.top();
        int lTime = curNode->ancestor->label->sdom->timeIn;
        int rTime = curNode->label->sdom->timeIn;
        if (lTime < rTime)
            curNode->label = curNode->ancestor->label;
        curNode->ancestor = uNode->ancestor;
        lastLabel = curNode->label;
        stack.pop();
    }
    return *lastLabel;
}

// NB: nodes must be sorted in ascending order
// TODO: set?
void GraphInfo::dominators(std::vector<int> const &nodes) {
    for (auto &[nodeId, node] : utilNodes) {
        node.label = &node;
        node.sdom = &node;
    }

    for (auto it = nodes.crbegin(); it != nodes.crend(); ++it) {
        auto &wNode = utilNodes.at(*it);
        if (wNode.parent == nullptr || wNode.timeIn == -1)
            continue;

        for (auto vId : cfg.block(wNode.id).prev) {
            UtilNode &vNode = utilNodes.at(vId);
            if (vNode.timeIn == -1)
                continue;
            auto &uNode = findMin(vNode);
            if (uNode.sdom->timeIn < wNode.sdom->timeIn)
                wNode.sdom = uNode.sdom;
        }
        wNode.ancestor = wNode.parent;
        wNode.sdom->bucket.push_back(&wNode);

        auto &bucket = wNode.parent->bucket;
        for (UtilNode *vNode : bucket) {
            auto &uNode = findMin(*vNode);
            if (uNode.sdom == vNode->sdom)
                vNode->dom->dominatorId = vNode->sdom->id;
            else
                vNode->dom->dominatorId = uNode.id;
        }
        bucket.clear();
    }

    for (auto it = nodes.cbegin(); it != nodes.cend(); ++it) { // ascending order
        auto &wNode = utilNodes.at(*it);
        if (wNode.parent == nullptr || wNode.timeIn == -1)
            continue;
        if (wNode.dom->dominatorId != wNode.sdom->id)
            wNode.dom->dominatorId = domData.at(wNode.dom->dominatorId).dominatorId;
    }
}


// Getters

GraphInfo::ArcClass GraphInfo::getArcClass(int from, int to) {
    auto it = arcsClasses.find(std::make_pair(from, to));
    return it == arcsClasses.end() ? GraphInfo::NONE : it->second;
}

std::vector<std::pair<int, int>> GraphInfo::getArcs(GraphInfo::ArcClass cl) {
    std::vector<std::pair<int, int>> res;
    for (auto const &[arc, aCl] : arcsClasses)
        if (aCl == cl)
            res.push_back(arc);
    return res;
}

bool GraphInfo::isIdom(int x, int y) const {
    return domData.at(y).dominatorId == x;
}

bool GraphInfo::isDom(int x, int y) const {
    int curId = y;
    while (curId != -1) {
        if (curId == x)
            return true;
        curId = domData.at(curId).dominatorId;
    }
    return false;
}

bool GraphInfo::isSdom(int x, int y) const {
    return x != y && isDom(x, y);
}

int GraphInfo::getIdom(int x) const {
    return domData.at(x).dominatorId;
}

std::vector<int> GraphInfo::getChildren(int x) const {
    return domData.at(x).childrenIds;
}


// Printers

std::string GraphInfo::drawDominators() const {
    std::ostringstream ss;
    ss << "digraph {" << std::endl;
    for (auto const &[nodeId, node] : domData) {
        if (node.dominatorId != -1) {
            ss << node.dominatorId << "->";
            ss << nodeId << std::endl;
        }
    }
    ss << "}";
    return ss.str();
}

std::string GraphInfo::drawArcsClasses() const {
    std::stringstream ss;
    fmt::print(ss, "digraph{{ ");
    for (auto &[arc, cl] : arcsClasses) {
        std::string fstr;
        switch (cl) {
            case GraphInfo::NONE:
                fstr = "{}->{}[style=\"dotted,bold\",color=purple]; ";
                break;
            case GraphInfo::TREE:
                fstr = "{}->{}[style=\"bold\",color=black]; ";
                break;
            case GraphInfo::BACK:
                fstr = "{}->{}[style=\"solid\",color=darkred,constraint=false]; ";
                break;
            case GraphInfo::CROSS:
                fstr = "{}->{}[style=\"dashed\",color=darkblue,constraint=false]; ";
                break;
            case GraphInfo::FWD:
                fstr = "{}->{}[style=\"dashed,bold\",color=darkgreen,constraint=false]; ";
                break;
            default:
                throw cw39_internal_error("Wrong arc class");
        }
        fmt::print(ss, fstr, arc.first, arc.second);
    }
    fmt::print(ss, "}}\n");
    return ss.str();
}

// Loops detector

LoopsDetector::LoopNode::LoopNode(IR_Block const &head) : head(head) {}

LoopsDetector::LoopsDetector(CFGraph &in) : cfg(in), gInfo(cfg) {
    // Find loops heads using back arcs
    for (auto const &[from, to] : gInfo.getArcs(GraphInfo::BACK)) {
        if (!gInfo.isDom(to, from)) { // Improper loop (because of goto)
            improperLoops = true;
            return;
        }

        auto it = loops.lower_bound(to);
        if (it == loops.end() || it->first != to)
            it = loops.emplace_hint(it, to, cfg.block(to));
        it->second.ends.insert(from);
    }

    // Find loops bodies
    for (auto &[loopId, loop] : loops) {
        for (int tailId : loop.ends) {
            traverseLoopBranch(loop, tailId);
        }
    }

    std::set<std::pair<int, int>> loopsRels;

    // Set associated blocks
    for (auto &[loopId, loop] : loops) {
        for (int blockId : loop.blocks) {
            lblocks[blockId].loops.insert(loopId);

            for (int othLoop : lblocks[blockId].loops) {
                if (othLoop != loopId) {
                    loopsRels.insert({ std::min(othLoop, loopId), std::max(othLoop, loopId) });
                }
            }
        }
    }

    // Set loops relations (who is nested in whom)
    for (auto const &[l1, l2] : loopsRels) {
        if (!testLoopsRelation(loops.at(l1), loops.at(l2))) {
            improperLoops = true;
            return;
        }
    }
}

void LoopsDetector::traverseLoopBranch(LoopNode &loop, int tailId) {
    std::set<int> visited;
    std::stack<int> stack;

    visited.insert(loop.head.id);
    visited.insert(tailId);
    stack.push(tailId);

    while (!stack.empty()) {
        IR_Block const &block = cfg.block(stack.top());
        stack.pop();
        for (int next : block.prev) {
            if (!visited.contains(next)) {
                visited.insert(next);
                stack.push(next);
            }
        }
    }

    loop.blocks.merge(visited);
}

void LoopsDetector::printInfo() const {
    for (auto &[loopId, loop] : loops) {
        if (loop.parent)
            fmt::print("loop {} (par {}):", loop.head.id, loop.parent->head.id);
        else
            fmt::print("loop {}:", loop.head.id);
        for (int blockId : loop.blocks)
            fmt::print(" {}", blockId);
        fmt::print("\n");
    }
}

bool LoopsDetector::testLoopsRelation(LoopNode &sLoop, LoopNode &lLoop) {
    if (sLoop.blocks.size() > lLoop.blocks.size()) {
        return testLoopsRelation(lLoop, sLoop);
    }

    /* Probably this test is always true because of proper structure */
    bool test = std::includes(lLoop.blocks.cbegin(), lLoop.blocks.cend(),
                              sLoop.blocks.cbegin(), sLoop.blocks.cend());
    if (test) {
        sLoop.parent = &lLoop;
        lLoop.children.push_back(&sLoop);
        return true;
    }
    else {
        return false;
    }
}

bool LoopsDetector::hasLoops() const {
    return !improperLoops && !loops.empty();
}

std::set<int> LoopsDetector::getBlockLoops(int blockId) const {
    auto it = lblocks.find(blockId);
    if (it == lblocks.end())
        return {};
    return it->second.loops;
}

const std::map<int, LoopsDetector::LoopNode>& LoopsDetector::getLoops() const {
    return loops;
}

GraphInfo const &LoopsDetector::getGraphInfo() const {
    return gInfo;
}

/** If visitor returns false, traversing is stopped */
void LoopsDetector::traverseLoop(LoopNode const &loop, std::function<bool(IR_Block&)> const &visitor)  {
    std::stack<int> stack;
    std::set<int> visited;
    stack.push(loop.head.id);
    visited.insert(loop.head.id);
    while (!stack.empty()) {
        int curId = stack.top();
        stack.pop();
        IR_Block &block = cfg.block(curId);
        if (!visitor(block))
            return;
        for (int nextId : block.next) {
            if (visited.contains(nextId) || !loop.blocks.contains(nextId))
                continue;
            stack.push(nextId);
            visited.insert(nextId);
        }
    }
}