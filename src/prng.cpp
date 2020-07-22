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

// TODO: these don't have to be classes, they could just be functions that return lambdas
float ExponentialGenerator::generate() const
{
   // will modify seed
   return one_over_lambda_*expdev(&seed_);
}

float UniformGenerator::generate() const
{
   return ran0(&seed_);
}

double BoundedParetoGenerator::generate() const
{
   // compute using inverse of CDF and uniform var
   double uniform;

   uniform = ran0(&seed_);

   // uniform being 1 makes nan
   // (-1e-11) ^ (-.909091) is imaginary
   uniform -= uniform == 1;

   double base = (h_to_the_alpha_ - l_to_the_alpha_plus_h_to_the_alpha_*uniform)
                 / l_to_the_alpha_times_h_to_the_alpha_;

   return pow(base, negative_one_over_alpha_);
}