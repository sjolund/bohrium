#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef Pi 
#define Pi 3.141592653589793238462643 
#endif 

#include <sys/time.h>
struct timeval tv;
struct timezone tz;
unsigned long long start, end, delta;


// The cumulative normal distribution function 
double CND( double X )
{
  
  double L, K, w ;
  
  double const a1 = 0.31938153, a2 = -0.356563782, a3 = 1.781477937;
  double const a4 = -1.821255978, a5 = 1.330274429;

  L = fabs(X);
  K = 1.0 / (1.0 + 0.2316419 * L);
  w = 1.0 - 1.0 / sqrt(2 * Pi) * exp(-L *L / 2) * (a1 * K + a2 * K *K + a3 * pow(K,3) + a4 * pow(K,4) + a5 * pow(K,5));
  
  if (X < 0 ){
    w= 1.0 - w;
  }
  return w;
}


// The Black and Scholes (1973) Stock option formula
double BS(char CallPutFlag, double S, double X, double T, double r, double v)
{
  double d1, d2;


  d1=(log(S/X)+(r+v*v/2)*T)/(v*sqrt(T));
  d2=d1-v*sqrt(T);

  if(CallPutFlag == 'c')
    return (S *CND(d1)-X * exp(-r*T)*CND(d2));
  else if(CallPutFlag == 'p')
    return X * exp(-r * T) * CND(-d2) - S * CND(-d1);
  return 0.0;
}


int main(int argc, char **argv)
{
  int N = 360000;
  int year = 100;
  int i,t,n;

  double *S = malloc(sizeof(double)*N);
  for(i=0; i<N; i++)
    S[i]=drand48();
  for(i=0; i<N; i++)
    S[i] = S[i]*4.0-2.0 + 60.0; //Price is 58-62
  
  double X=65.0;
  double r=0.08;
  double v=0.3;

  double day=1.0/year;
  double T=day;

  double tresult=0.0;

  gettimeofday(&tv, &tz);
  start = (long long)tv.tv_usec + ((long long)tv.tv_sec)*1000000;
  for(t=0; t<year; t++){
    double result=0.0;
    for(n=0; n<N; n++){
      result+=BS('c', S[n], X, T, r, v);
    }
    tresult += result/N;
    T+=day;
  }
  gettimeofday(&tv, &tz);
  end = (long long)tv.tv_usec + ((long long)tv.tv_sec)*1000000;
  delta = end - start;
  
  printf("Iter: %d size: %d time: %lf (ANSI C)\n", year, N, delta/(double)1000000.0);

  printf("Total %lf\n",tresult);

  return 0;
}

