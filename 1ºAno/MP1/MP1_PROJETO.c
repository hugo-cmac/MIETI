#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
#include "time.h"
#include "ctype.h"

int verification(int n,char m[n])
{
  int i,f=0;
  if (n>=9 && n<=15) {                //verify if there are between 9 and 15 letters,if not f=0
    for (i = 0; i < n; i=i+1) {         //verify if elements from encryptedmessage are between 97 and 122 in the ascii
      if (m[i]<=122 && m[i]>=97) {
        f=f+1;}}                        //if yes, f increments
    if (f==n) {                       //if, f=n(n=sizeencrytedmess), f is 1
      f=1;
    }else{
      f=0;}
  }else{
    f=0;}
  return f;
}

char *numerationwords(int k[3],char w[15])      //transformation of key to word form
{
  int i;
  strcpy(w,"");                                 //emptying the string that will contain the key wordform
  for (i = 2; i >= 0; i=i-1) {                  //using other decrementation because the number were saved backwords
    switch (k[i]) {
      case 0:strcat(w,"ZERO");break;            //for each number we add to the string the corresponding wordform number
      case 1:strcat(w,"ONE");break;
      case 2:strcat(w,"TWO");break;
      case 3:strcat(w,"THREE");break;
      case 4:strcat(w,"FOUR");break;
      case 5:strcat(w,"FIVE");break;
      case 6:strcat(w,"SIX");break;
      case 7:strcat(w,"SEVEN");break;
      case 8:strcat(w,"EIGHT");break;
      case 9:strcat(w,"NINE");break;
    }
  }
  return w;                                     //and return the keyword wordform
}

void decryptionprint(int size,char message[size],char key[size])          //decryption algorithim and print function
{
  int i,m,k,error;
  char decryption[size];

  for (i = 0; i < size; i=i+1) {                                          //decryption algorithim
      m=message[i];
      m=m-96;
      k=key[i];
      k=k-64;
      m=m-k;
      if (m<=0) {
        m=26+m;
      }
      decryption[i]=m+96;                                                 //saves the decrypted letter
  }

  for (i = 0; i < size; i=i+1) {                                          //prints the decrypted word
    printf("%c",decryption[i]);
  }
  printf("->");
  for (i = 0; i < size; i=i+1) {                                         //prints the keyword used
    printf("%c",key[i]);
  }
  printf("\n");
}

void decryptionprocess(int sizeencrytedmess,char encryptedmessage[sizeencrytedmess])      //decryption begining and generation of numbers
{
  int keyword[3],tempkey,n=999,sizekeywordform;
  int i,f;
  char wordformkeyword[15];
  for (i = 0; i <= n; i=i+1) {                                                            //decrementation process (999 to 000)
    tempkey=n-i;
    for (f = 0; f < 3; f=f+1) {                                                           //separation of number into array
      keyword[f]=tempkey%10;
      tempkey=tempkey/10;
    }
    strcpy(wordformkeyword,numerationwords(keyword,wordformkeyword));                     //transformation of key to wordform
    sizekeywordform=strlen(wordformkeyword);                                              //counting number o caracters
    if (sizeencrytedmess==sizekeywordform) {                                              //if number of caracters from encryptedmessage matches the number of caracters from the keyword
      decryptionprint(sizeencrytedmess,encryptedmessage,wordformkeyword);                 //we will finish the decryption and print it
    }
  }
}

int main() {
  char encryptedmessage[20];
  int sizeencrytedmess,flag,i;

  fgets(encryptedmessage,20,stdin);
  encryptedmessage[strlen(encryptedmessage)-1]='\0';    //this function removes the "enter" from the end of the string
  sizeencrytedmess=strlen(encryptedmessage);            //this function counts the number of elements in the string

  for (i = 0; i < sizeencrytedmess; i=i+1) {                        //this cicle puts capital words in small letters
    if (encryptedmessage[i]<=90 && encryptedmessage[i]>=65 ) {
      encryptedmessage[i]=encryptedmessage[i]+32;
    }
  }

  flag=verification(sizeencrytedmess,encryptedmessage); //verification of message, in search for caraters diferent from letters

  if (flag==1) {                                        //if flag=1  =  (f=1), the letters are correct, so we can proceed to decryption
    decryptionprocess(sizeencrytedmess,encryptedmessage);
  }else{
    printf("INVALID ENCRYPTED MESSAGE\n");                //if flag diferent from 1 or (f≃0), the caracters are invalid
  }
  return 0;
}
