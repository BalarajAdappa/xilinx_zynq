/*************************************************************************
    > File Name: audio.c
    > Author: dooon
    > Mail: tangxu314@gmail.com 
    > Created Time: 2017年03月09日 星期四 16时57分19秒
 ************************************************************************/

#include<stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
 
/* M_PI is declared in math.h */
#define PI M_PI
 
 
typedef unsigned int    UI;
typedef unsigned long int   UL;
typedef unsigned short int  US;
typedef unsigned char   UC;
typedef signed int      SI;
typedef signed long int SL;
typedef signed short int    SS;
typedef signed char SC;
 
 
#define attr(a) __attribute__((a))
 
#define packed attr(packed)
 
/* WAV header, 44-byte total */
typedef struct{
 UL riff    packed;
 UL len packed;
 UL wave    packed;
 UL fmt packed;
 UL flen    packed;
 US one packed;
 US chan    packed;
 UL hz  packed;
 UL bpsec   packed;
 US bpsmp   packed;
 US bitpsmp packed;
 UL dat packed;
 UL dlen    packed;
}WAVHDR;
 
 
 
int savefile(const char*const s,const void*const m,const int ml){
 FILE*f=fopen(s,"wb");
 int ok=0;
 if(f){
  ok=fwrite(m,1,ml,f)==ml;
  fclose(f);
 }
 return ok;
}
 
 
/* "converts" 4-char string to long int */
#define dw(a) (*(UL*)(a))
 
 
/* Makes 44-byte header for 8-bit WAV in memory
usage: wavhdr(pointer,sampleRate,dataLength) */
 
void wavhdr(void*m,UL hz,UL dlen){
 WAVHDR*p=m;
 p->riff=dw("RIFF");
 p->len=dlen;
 p->wave=dw("WAVE");
 p->fmt=dw("fmt ");
 p->flen=0x10;
 p->one=1;
 p->chan=1;
 p->hz=hz;
 p->bpsec=hz;
 p->bpsmp=1;
 p->bitpsmp=8;
 p->dat=dw("data");
 p->dlen=dlen;
}
 
 
/* returns 8-bit sample for a sine wave */
US sinewave(UL rate,float freq,UC amp,UL z){
 return sin(z*((PI*2/rate)*freq))*amp+128;
}
 
 
/* make arbitrary audio data here */
void makeaud(UC*p,const UL rate,UL z){
 float freq=2000;
 UC amp=120;
 while(z--){
  *p++=sinewave(rate,freq,amp,z);
 }
}
 
 
/* makes wav file */
void makewav(const UL rate,const UL dlen){
 const UL mlen=dlen+44;
 UC*const m=malloc(mlen);
 if(m){
  wavhdr(m,rate,dlen);
  makeaud(m+44,rate,dlen);
  savefile("out.wav",m,mlen);
 }
}
 
 
int main(){
 if(sizeof(WAVHDR)!=44)puts("bad struct");
 makewav(44100,640000);
 return 0;
}
