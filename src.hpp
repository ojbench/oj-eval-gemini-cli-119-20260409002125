#ifndef LINEARSCAN_HPP
#define LINEARSCAN_HPP

// don't include other headfiles
#include <string>
#include <vector>
#include <set>

class Location {
public:
    // return a string that represents the location
    virtual std::string show() const = 0;
    virtual int getId() const = 0;
};

class Register : public Location {
private:
    int id;
public:
    Register(int regId) : id(regId) {
    }
    virtual std::string show() const {
        return "reg" + std::to_string(id);
    }
    virtual int getId() const {
        return id;
    }
};

class StackSlot : public Location {
public:
    StackSlot() {}
    virtual std::string show() const {
        return "stack";
    }
    virtual int getId() const {
        return -1;
    }
};

struct LiveInterval {
    int startpoint;
    int endpoint;
    Location* location = nullptr;
};

class LinearScanRegisterAllocator {
private:
    int regNum;
    std::vector<int> free_registers;

    struct CompareEndpoint {
        bool operator()(const LiveInterval* a, const LiveInterval* b) const {
            return a->endpoint < b->endpoint;
        }
    };
    std::set<LiveInterval*, CompareEndpoint> active;

    void expireOldIntervals(LiveInterval& i) {
        auto it = active.begin();
        while (it != active.end()) {
            if ((*it)->endpoint >= i.startpoint) {
                break;
            }
            free_registers.push_back((*it)->location->getId());
            it = active.erase(it);
        }
    }

    void spillAtInterval(LiveInterval& i) {
        if (active.empty()) {
            i.location = new StackSlot();
            return;
        }
        auto last_it = active.end();
        --last_it;
        LiveInterval* spill = *last_it;
        if (spill->endpoint > i.endpoint) {
            i.location = new Register(spill->location->getId());
            spill->location = new StackSlot();
            active.erase(last_it);
            active.insert(&i);
        } else {
            i.location = new StackSlot();
        }
    }

public:
    LinearScanRegisterAllocator(int regNum) : regNum(regNum) {
        for (int i = regNum - 1; i >= 0; --i) {
            free_registers.push_back(i);
        }
    }

    void linearScanRegisterAllocate(std::vector<LiveInterval>& intervalList) {
        for (auto& i : intervalList) {
            expireOldIntervals(i);
            if (active.size() == (size_t)regNum) {
                spillAtInterval(i);
            } else {
                int reg = free_registers.back();
                free_registers.pop_back();
                i.location = new Register(reg);
                active.insert(&i);
            }
        }
    }
};

#endif
