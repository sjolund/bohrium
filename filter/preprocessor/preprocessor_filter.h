//#include "helper_functions.h"

class preprocessor_filter 
{
    public:
        void preprocess(bh_ir *bhir);
        void gc();

    private:
        std::vector<bh_base*> bases_;
    
};