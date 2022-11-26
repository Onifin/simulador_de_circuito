#include <iostream>
#include <string>
#include "circuito.h"
#include "port.h"

using namespace std;

int main(void)
{
    std::vector<bool3S> in_vec;
    //in_vec.push_back(bool3S::UNDEF);
    in_vec.push_back(bool3S::TRUE);
    in_vec.push_back(bool3S::TRUE);

    Port_NXOR teste;

    ptr_Port a = teste.clone();

    //(*a).digitar();
    (*a).simular(in_vec);

    //cout << (*a).getNumInputs();
    //cout << (*a).getName();
    cout << toChar((*a).getOutput());

}
