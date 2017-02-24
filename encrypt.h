#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check(char sign)
{
  //jesli jest mala litera
  if(sign >= 'a' && sign <= 'z') return 0;
  //jesli jest duza litera
  if(sign >= 'A' && sign <= 'Z') return 1;
  // jesli cyfra
  if(sign >= '%' && sign <= '?') return 2;
  //inna niż litera
  return 3;
}

void encrypt(int key, char tab[])
{
  //sprawdzenie, czy key miesci sie w zakresie
  if(!(key >= -26 && key <= 26)) return;

  int type;
  char a, z;

  for(int i = 0; i < strlen(tab); i++)
  {
    type = check(tab[i]);
    //ustalienie wielkosci litery
    if(type < 2)
    {
      if(type == 0)
        a = 'a', z = 'z';
      else
        a = 'A', z = 'Z';

      if(key >= 0)

        if(tab[i] + key <= z)
          tab[i] += key;
        else
          tab[i] = tab[i] + key - 26;
      else
        if(tab[i] + key >= a)
          tab[i] += key;
        else
          tab[i] = tab[i] + key + 26;
    }
    if(type == 2)
    {
      a = '%';
      z = '?';
      if(key >= 0)

        if(tab[i] + key <= z)
          tab[i] += key;
        else
          tab[i] = tab[i] + key - 26;
      else
        if(tab[i] + key >= a)
          tab[i] += key;
        else
          tab[i] = tab[i] + key + 26;
    }
  }
}
