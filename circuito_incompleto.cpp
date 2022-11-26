#include <fstream>
#include <utility> // para std::swap
#include "circuito.h"

///
/// As strings que definem os tipos de porta
///

// Funcao auxiliar que testa se uma string com nome de porta eh valida
// Caso necessario, converte os caracteres da string para maiusculas
bool validType(std::string& Tipo)
{
  if (Tipo.size()!=2) return false;
  Tipo.at(0) = toupper(Tipo.at(0));
  Tipo.at(1) = toupper(Tipo.at(1));
  if (Tipo=="NT" ||
      Tipo=="AN" || Tipo=="NA" ||
      Tipo=="OR" || Tipo=="NO" ||
      Tipo=="XO" || Tipo=="NX") return true;
  return false;
}

// Funcao auxiliar que retorna um ponteiro que aponta para uma porta alocada dinamicamente
// O tipo da porta alocada depende do parametro string de entrada (AN, OR, etc.)
// Caso o tipo nao seja nenhum dos validos, retorna nullptr
// Pode ser utilizada nas funcoes: Circuito::setPort, Circuito::digitar e Circuito::ler
ptr_Port allocPort(std::string& Tipo)
{
  if (!validType(Tipo)) return nullptr;

  if (Tipo=="NT") return new Port_NOT;
  if (Tipo=="AN") return new Port_AND;
  if (Tipo=="NA") return new Port_NAND;
  if (Tipo=="OR") return new Port_OR;
  if (Tipo=="NO") return new Port_NOR;
  if (Tipo=="XO") return new Port_XOR;
  if (Tipo=="NX") return new Port_NXOR;

  // Nunca deve chegar aqui...
  return nullptr;
}

///
/// CLASSE CIRCUITO
///

/// ***********************
/// Inicializacao e finalizacao
/// ***********************

Circuito::Circuito():
    // Nin(), id_out(nullptr), out_circ(nullptr), ports(nullptr)
    Nin(), id_out(), out_circ(), ports()
{

}

Circuito::Circuito(const Circuito& C):
    // Nin(), id_out(nullptr), out_circ(nullptr), ports(nullptr)
    Nin(), id_out(), out_circ(), ports()
{
    Nin = C.getNumInputs();
    id_out = C.id_out;
    out_circ = C.out_circ;
    ports = C.ports;
}

Circuito::Circuito(Circuito&& C)
{

}

Circuito::~Circuito()
{
    clear();
}

void Circuito::clear()
{
    Nin = 0;
    id_out.clear();
    out_circ.clear();
    for (int i = 0; i < ports.size(); i++) delete ports[i];
    ports.clear();
}

void Circuito::operator=(const Circuito& C)
{
    // falta fazer
}

void Circuito::operator=(Circuito&& C)
{
    // falta fazer
}


void Circuito::resize(int NI, int NO, int NP)
{
    if(NI < 0 || NO < 0 || NP < 0) return;

    out_circ.clear();
    id_out.clear();
    ports.clear();

    Nin = NI;

    out_circ.resize(NO);
    id_out.resize(NO);
    ports.resize(NP);

    for(int i = 0; i < NO; i++)
    {
        id_out[i] = 0;
        out_circ[i] = bool3S::UNDEF;
    }

    for(int i = 0; i < NP; i++)
    {
        ports[i] = nullptr;
    }
}

/// ***********************
/// Funcoes de testagem
/// ***********************

// Retorna true se IdInput eh uma id de entrada do circuito valida (entre -1 e -NInput)
bool Circuito::validIdInput(int IdInput) const
{
  return (IdInput<=-1 && IdInput>=-getNumInputs());
}

// Retorna true se IdOutput eh uma id de saida do circuito valida (entre 1 e NOutput)
bool Circuito::validIdOutput(int IdOutput) const
{
  return (IdOutput>=1 && IdOutput<=getNumOutputs());
}

// Retorna true se IdPort eh uma id de porta do circuito valida (entre 1 e NPort)
bool Circuito::validIdPort(int IdPort) const
{
  return (IdPort>=1 && IdPort<=getNumPorts());
}

// Retorna true se IdOrig eh uma id valida para a origem do sinal de uma entrada de porta ou
// para a origem de uma saida do circuito (podem vir de uma entrada do circuito ou de uma porta)
// validIdOrig == validIdInput OR validIdPort
bool Circuito::validIdOrig(int IdOrig) const
{
  return validIdInput(IdOrig) || validIdPort(IdOrig);
}

// Retorna true se IdPort eh uma id de porta valida (validIdPort) e
// a porta estah definida (estah alocada, ou seja, != nullptr)
bool Circuito::definedPort(int IdPort) const
{
  if (!validIdPort(IdPort)) return false;
  if (ports.at(IdPort-1)==nullptr) return false;
  return true;
}

// Retorna true se IdPort eh uma porta existente (definedPort) e
// todas as entradas da porta com Id de origem valida (usa getId_inPort e validIdOrig)
bool Circuito::validPort(int IdPort) const
{
  if (!definedPort(IdPort)) return false;
  for (int j=0; j<getNumInputsPort(IdPort); j++)
  {
    if (!validIdOrig(getId_inPort(IdPort,j))) return false;
  }
  return true;
}

// Retorna true se o circuito eh valido (estah com todos os dados corretos):
// - numero de entradas, saidas e portas valido (> 0)
// - todas as portas validas (usa validPort)
// - todas as saidas com Id de origem validas (usa getIdOutput e validIdOrig)
// Essa funcao deve ser usada antes de salvar ou simular um circuito
bool Circuito::valid() const
{
  if (getNumInputs()<=0) return false;
  if (getNumOutputs()<=0) return false;
  if (getNumPorts()<=0) return false;
  for (int i=0; i<getNumPorts(); i++)
  {
    if (!validPort(i+1)) return false;
  }
  for (int i=0; i<getNumOutputs(); i++)
  {
    if (!validIdOrig(getIdOutput(i+1))) return false;
  }
  return true;
}

/// ***********************
/// Funcoes de consulta
/// ***********************


int Circuito::getNumInputs() const
{
  return Nin;
}

int Circuito::getNumOutputs() const
{
  return this->id_out.size();
}

int Circuito::getNumPorts() const
{
  return this->ports.size();
}

  // Retorna a origem (a id) do sinal de saida cuja id eh IdOutput
  // Depois de testar o parametro (validIdOutput), retorna id_out[IdOutput-1]
  // ou 0 se parametro invalido
int Circuito::getIdOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput -1)) return id_out[IdOutput -1];
  return false;
}

bool3S Circuito::getOutput(int IdOutput) const
{
  if (validIdOutput(IdOutput-1)) return out_circ[IdOutput-1];
  return bool3S::UNDEF;
}

std::string Circuito::getNamePort(int IdPort) const
{
  if (definedPort(IdPort)) return ports[IdPort-1]->getName();
  return "??";
}

int Circuito::getNumInputsPort(int IdPort) const
{
  if (definedPort(IdPort)) return ports[IdPort-1]->getNumInputs();
  return false;
}

  // Retorna a origem (a id) da I-esima entrada da porta cuja id eh IdPort
  // Depois de testar se a porta existe (definedPort) e o indice da entrada I,
  // retorna ports[IdPort-1]->getId_in(I)
  // ou 0 se parametro invalido
int Circuito::getId_inPort(int IdPort, int I) const
{
  if (definedPort(IdPort) && validIdInput(I)) return ports[IdPort-1]->getId_in(I);
  return false;
}

/// ***********************
/// Funcoes de modificacao
/// ***********************

  void Circuito::setIdOutput(int IdOut, int IdOrig)
  {
    // fazer falta
    
  }

  void Circuito::setPort(int IdPort, std::string Tipo, int NIn)
  {
    if (validIdPort(IdPort) && validType(Tipo)) //falta fazer teste NIn
    {
        delete ports[IdPort -1];
        ports[IdPort -1] = allocPort(Tipo);
        (*ports[IdPort -1]).setNumInputs(NIn);
    }
  }

/// ***********************
/// E/S de dados
/// ***********************

void Circuito::digitar()
{
    int Nin;
    int Nout;
    int Nports;

    std::string tipo;
    do
    {
        std::cout << "  Numero de entradas da porta: ";
        std::cin >> Nin;
    }
    while(Nin < 0);

    do
    {
        std::cout << "  Numero de saidas da porta: ";
        std::cin >> Nout;
    }
    while(Nout < 0);

    do
    {
        std::cout << "  Numero de portas: ";
        std::cin >> Nports;
    }
    while(Nports < 0);

    resize(Nin, Nout, Nports);

    for(int i = 0; i < Nports; i++)
    {
        do
        {
            std::cout << "  Digite o tipo da porta " << i+1 << std::endl;
            std::cin >> tipo;

            if(validType(tipo))
            {
                ports[i] = allocPort(tipo);
            }
            do
            {
                (*ports[i]).digitar();
            }while(validPort(i+1));

        }
        while(!validType(tipo));

    }

    for(int i = 0; i < Nout; i++)
    {
        do
        {
            std::cout << "  Digite o id de saida da porta " << i+1 << std::endl;
            std::cin >> id_out[i];
        }while(validIdOrig(id_out[i]));
    }

}

bool Circuito::ler(const std::string& arq)
{
    std::ifstream ArqCircuito;
    ArqCircuito.open(arq);
    try
    {
        std::string titulo, stPorta;
        int Nin, Nout, Np;
        ArqCircuito >> titulo;
        ArqCircuito >> Nin >> Nout >> Np;
        ArqCircuito >> stPorta;

        if (!ArqCircuito.good() || titulo != "CIRCUITO" || stPorta != "PORTA") throw 1;

        for (int i = 0; i < Np; i++)
        {
            int index;
            std::string prnts, portTipo;
            ArqCircuito >> index >> prnts >> portTipo;

            if (!ArqCircuito.good() || index != i + 1 || prnts != ")" || validType(portTipo)) throw 2;

            Port *P = allocPort(portTipo);
            if ((*P).ler(ArqCircuito) && validPort((*P).getId_in(index)));
        }

        std::string saidas;
        ArqCircuito >> saidas;

        if (!ArqCircuito.good() || saidas != "SAIDAS") throw 3;

        for (int i = 0; i < Nout; i++)
        {
            int index;
            std::string prnts;
            ArqCircuito >> index >> prnts;

            if (!ArqCircuito.good() || index != i + 1 || prnts != ")" || validIdOrig(index)) throw 4;
        }
    }

    catch (int erro)
    {
        ArqCircuito.close();
        return false;
    }

    ArqCircuito.close();
    return true;
}

std::ostream& Circuito::imprimir(std::ostream& O=std::cout) const
{
    O << "CIRCUITO " << getNumInputs() << " " << getNumOutputs() << " " << getNumPorts() << std::endl;
    O << "PORTAS" << std::endl;
    for (int i = 0; i< ports.size(); i++)
    {
        O << (*ports[i]).getId_in(i) << ")" << (*ports[i]) << std::endl;
    }
    O << "SAIDAS" << std::endl;
    for (int k; k < getNumOutputs(); k++)
    {
        O << getIdOutput(k) << ") " <<  (*ports[k]).getOutput();
    }
    return O;
}

 bool Circuito::salvar(const std::string& arq) const
  {
    if(!valid) return false;
    std::ofstream Of(arq);
    Of.open(arq);
    if (!Of.good())
    {
        Of.close();
        return false;
    }
    imprimir(Of);
    Of.close();
    return true;
  }

/// ***********************
/// SIMULACAO (funcao principal do circuito)
/// ***********************

bool Circuito::simular(const std::vector<bool3S>& in_circ)
{
    // VARIÁVEIS LOCAIS:
    bool tudo_def, alguma_def;
    int id;
    std::vector<bool3S> in_port;

    // SIMULAÇÃO DAS PORTAS

    for(int i = 0; i <= getNumPorts()-1; i++)
    {
        (*ports[i]).setOutput(bool3S::UNDEF);
    }

    do
    {
        tudo_def = true;
        alguma_def = false;

        for(int i = 0; i <= getNumPorts()-1; i++)
        {
            if((*ports[i]).getOutput() == bool3S::UNDEF)
            {
                in_port.resize((*ports[i]).getNumInputs());
                for(int j = 0; (*ports[i]).getNumInputs()-1; j++)
                {

                    id = (*ports[i]).getId_in(j);

                    if(id>0)
                    {
                        in_port[j] = (*ports[id-1]).getOutput();
                    }else
                    {
                        in_port[j] = in_circ[-id-1];
                    }
                }

                (*ports[i]).simular(in_port);

                if((*ports[id-1]).getOutput() == bool3S::UNDEF)
                {
                    tudo_def = false;
                }else
                {
                    alguma_def = true;
                }
            }
        }
    }while(!tudo_def && alguma_def);

    for(int j = 0; j <= getNumOutputs()-1; j++)
    {
        id = id_out[j];

        if(id>0)
        {
             out_circ[j] = (*ports[id-1]).getOutput();
        }else
        {
            out_circ[j] = in_circ[-id-1];
        }
    }
}




std::ostream& operator<<(std::ostream& O, const Circuito& C)
{
    C.imprimir(O);
    return O;
}