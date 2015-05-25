#ifndef Automate_h
#define Automate_h

#define MAX_PTRN_LN  256
#define MAX_DATA_LN  1024


class Automate {
  private:
    char* _pattern;
    int _patternLn;
    int _index;
    void reset();  // Remet l'automate à zéro
  public:
    Automate(char*, char*); // Constructeur
    bool iterate(char); // Prend un caractere en plus
};  

#endif
