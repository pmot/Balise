#include "Automate.h"

// Constructeur, prend un mot en entrée
Automate::Automate(char* pattern, char* pattern2) {
  /*  
    pattern contient des caractères alphanumeriques ascii
    sizeof retourne donc le nombre de caractères
    la classe automate ne modifie pas la chaine, une copie
    de la référence suffit.
  */
  this->_pattern = pattern;
  this->_index = 0;
  this->_patternLn = 0;
  if ((this->_patternLn = sizeof(this->_pattern)) > MAX_PTRN_LN)
    this->_patternLn = -1;
}


/* reset
   L'index _index est remis à zéro
*/
void Automate::reset() {
  this->_index = 0;
}

/* iterate, prend un caractère, analyse
   Si le caractère correspond à la grammaire _
   
*/
bool Automate::iterate(char c) {
   if (c == this->_pattern[this->_index]) {
     this->_index++;
     if (this->_index == this->_patternLn) return true;
   }  
   else this->reset();
   return false;
}
