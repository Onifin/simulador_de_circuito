#include <iostream>
#include <string>
#include "circuito.h"

using namespace std;

void gerarTabela(Circuito& C);

int main(void)
{
  Port_NOT C;

  cout << C.getName();

  C.setOutput(bool3S::FALSE);

  vector<bool3S> in_circ(C.getNumInputs());

  C.simular(in_circ);

  cout << C.getOutput();
}

