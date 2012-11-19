// c++ testing stuff

#include <iostream>
#include <stdio.h>   
#include "jit_ast.h"

using namespace std;


void test_out(int in, int* out) {
    
    int t = in + 10;
    int* s = &t;
    cout << out << "\n";
    cout << &t << "\n";
    // out = s; does not work!!
    *out = *s; // works!!
    cout << *out << "\n";
}










int main() {

    cout << "hello world\n";
    int out = 1;
    test_out(5,&out);
    
    cout << out << "\n";
    
    //test_constant_to_string();
    
    return 0;
}
