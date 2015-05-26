#include <bh.h>

enum optimization_name {
    POWER,
    COMMON
};

// Base class
class Optimization 
{
    public:
        Optimization(optimization_name name, std::vector<bh_base*> *basesPtr);
        void optimize(bh_ir *bhir);

    private:
        optimization_name name_;
        std::vector<bh_base*> *basesPtr_;

        void addBaseToGC(bh_base *base) {
            basesPtr_->push_back(base);
        }

        void power(bh_ir *bhir);
        void common(bh_ir *bhir);
};

/*Optimization::Optimization(optimization_name thingy) : thingy_(thingy) {}
void Optimization::optimize() {
    switch(thingy_) {
        case POWER:
            power();
            break;
        case COMMON:
            nogetandet();
            break;
        default:
            exception
    }
}*/

