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
    Nin(), id_out(), out_circ(), ports()
{

}


Circuito::~Circuito()
{
    Nin = 0;
    id_out.clear();
    //out_circ().clear();
    //ports().clear();
}

// Redimensiona o circuito para passar a ter NI entradas, NO saidas e NP ports
// Inicialmente checa os parametros. Caso sejam validos,
// depois de limpar conteudo anterior (clear), altera Nin; os vetores tem as dimensoes
// alteradas (resize) e sao inicializados com valores iniciais neutros ou invalidos:
// id_out[i] <- 0
// out_circ[i] <- UNDEF
// ports[i] <- nullptr
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

// Entrada dos dados de um circuito via teclado
// O usuario digita o numero de entradas, saidas e portas
// apos o que, se os valores estiverem corretos (>0), redimensiona o circuito
// Em seguida, para cada porta o usuario digita o tipo (NT,AN,NA,OR,NO,XO,NX) que eh conferido
// Apos criada dinamicamente (new) a porta do tipo correto, chama a
// funcao digitar na porta recem-criada.

// A porta digitada eh conferida (validPort).
// Em seguida, o usuario digita as ids de todas as saidas, que sao conferidas (validIdOrig).
// Se o usuario digitar um dado invalido, o metodo deve pedir que ele digite novamente
// Deve utilizar o metodo digitar da classe Port

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
