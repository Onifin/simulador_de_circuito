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
/// Inicializacao e finalizacao @ian909041 dev
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
/// ***********************
/// Funcoes de testagem
/// ***********************

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
// funcao digitar na porta recem-criada. A porta digitada eh conferida (validPort).
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
                (*ports[i]).digitar();
            }
        }
        while(!validType(tipo));


        }
}
