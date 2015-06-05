#include <stdexcept>
#include <optimization.h>

using namespace std;

Optimization::Optimization(optimization_name name, std::vector<bh_base*> *basesPtr) : name_(name), basesPtr_(basesPtr) {}
void Optimization::optimize(bh_ir *bhir) {
    switch(name_) {
        case POWER:
            power(bhir);
            break;
        case POWER_CACHE:
            power_cache(bhir);
            break;
        case COMMON:
            common(bhir);
            break;
        default:
            throw std::runtime_error( "Optimization not implemented" );
    }
}
