#include <math.h>
#include "prng.h"

// ------------------- CODE FROM LECTURE MATERIAL -----------
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876

float ran0(long * idum){
    long k;
    float ans;
    *idum ^= MASK; // allows zero as seed
    k=(*idum)/IQ;
    *idum=IA*(*idum-k*IQ)-IR*k;
    if (*idum<0) *idum+=IM;
    ans=AM*(*idum);
    *idum ^= MASK;
    return ans;
}

float expdev(long *idum) {
   float ran0(long *idum);
   float dummy;
do
   dummy=ran0(idum);
while (dummy == 0.0);
   return -log(dummy);
}
// ------------------- END CODE FROM LECTURE MATERIAL -----------

float ExponentialGenerator::generate() const
{
   // will modify seed
   return one_over_lambda_*expdev(&seed_);
}

float UniformGenerator::generate() const
{
   return ran0(&seed_);
}