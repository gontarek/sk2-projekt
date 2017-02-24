#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

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

int main()
{
  char tab[BUF_SIZE];

  int key;

  printf("Podaj zdanie do zaszyfrowania: ");
  scanf("%s", tab);

  printf("Podaj key z przedziału [-26..26]: ");
  scanf("%d", &key);

  encrypt(key,tab); //szyfrowanie

  printf("Po zaszyfrowaniu: %s\n", tab);

  encrypt(-key,tab); //deszyfrowanie

  printf("Po rozszyfrowaniu: %s\n", tab);

  return 0;
}
