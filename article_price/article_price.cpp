// EurpeanOption.cpp
//
// Author: teng feifei
//
// (C)  20180923
//

#include "article_price.h"
#include <iostream>
//#include "EuropeanOption.h" // Declarations of functions
#include <math.h>			// For mathematical functions, e.g. exp()
void EuropeanOption::bsm_option_greeks(OptionGreeks* og){
    //calldelta
    double tmp = og->sig * sqrt(og->T);
    double d1 = ( log(og->U/og->K) + (og->b+ (og->sig*og->sig)*0.5 ) * og->T )/ tmp;
    double Nd1 = N(d1);
    double tmpExp = exp((og->b-og->r)*og->T);
    double cDelta = tmpExp * Nd1;
    //putdelta
    double pDelta = tmpExp * (Nd1 - 1.0);
    //gamma
    double Nd1_biasis = exp(-d1*d1/2)/(sqrt(2*pi));
    double q=r;
    double gamma = Nd1_biasis*exp(-q*og->T)/(og->U*og->sig*sqrt(og->T));
    og->cDelta=cDelta;
    og->pDelta=pDelta;
    og->gamma=gamma;
}

// Kernel Functions
double EuropeanOption::CallPrice() const
{

    double tmp = sig * sqrt(T);

    double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;
    double d2 = d1 - tmp;


    return (U * exp((b-r)*T) * N(d1)) - (K * exp(-r * T)* N(d2));

}

double EuropeanOption::PutPrice() const
{

    double tmp = sig * sqrt(T);

    double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;
    double d2 = d1 - tmp;

    return (K * exp(-r * T)* N(-d2)) - (U * exp((b-r)*T) * N(-d1));
}

double EuropeanOption::CallDelta() const
{
    double tmp = sig * sqrt(T);

    double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;


    return exp((b-r)*T) * N(d1);
}
double EuropeanOption::bsm_option_gamma(double S0,double K,double T,double r,double sigma){
    double tmp = sigma * sqrt(T);
    double d1 = ( log(S0/K) + ((sigma*sigma)*0.5 ) * T )/ tmp;
    double Nd1_biasis = exp(-d1*d1/2)/(sqrt(2*pi));
    double q=r;
    double gamma = Nd1_biasis*exp(-q*T)/(S0*sigma*sqrt(T));
    return gamma;
}
double EuropeanOption::PutDelta() const
{
    double tmp = sig * sqrt(T);

    double d1 = ( log(U/K) + (b+ (sig*sig)*0.5 ) * T )/ tmp;

    return exp((b-r)*T) * (N(d1) - 1.0);
}
double EuropeanOption::n(double x) const
{ // Gaussian (normal) distribution function
    double A = 1.0/sqrt(2.0 * 3.1415);
    return A * exp(-x*x*0.5);

}

double EuropeanOption::N(double x) const
{ // The approximation to the cumulative normal distribution function


    double a1 = 0.4361836;
    double a2 = -0.1201676;
    double a3 = 0.9372980;

    double k = 1.0/(1.0 + (0.33267 * x));

    if (x >= 0.0)
    {
        return 1.0 - n(x)* (a1*k + (a2*k*k) + (a3*k*k*k));
    }
    else
    {
        return 1.0 - N(-x);
    }

}


void EuropeanOption::init()
{ // Initialise all default values

    // Default values
    r = 0.08;
    sig= 0.30;
    K = 65.0;
    T = 0.25;
    U = 60.0;		// U == stock in this case
    b = r;		// Black and Scholes stock option model (1973)

    optType = "C";	// European Call Option (the default type)

}

void EuropeanOption::copy(const EuropeanOption& o2)
{

    r	= o2.r;
    sig = o2.sig;
    K	= o2.K;
    T	= o2.T;
    U	= o2.U;
    b	= o2.b;

    optType = o2.optType;

}

EuropeanOption::EuropeanOption()
{ // Default call option

    init();
}

EuropeanOption::EuropeanOption(const EuropeanOption& o2)
{ // Copy constructor

    copy(o2);
}

EuropeanOption::EuropeanOption (const string& optionType)
{ // Create option type

    init();
    optType = optionType;

    if (optType == "c")optType = "C";
}


EuropeanOption::~EuropeanOption()
{ // Destructor

}


EuropeanOption& EuropeanOption::operator = (const EuropeanOption& opt2)
{ // Assignment operator (deep copy)

    if (this == &opt2) return *this;

    copy (opt2);

    return *this;
}

// Functions that calculate option price and sensitivities
double EuropeanOption::Price() const
{
    if (optType == "C")return CallPrice();
    else return PutPrice();
}

double EuropeanOption::Delta() const
{
    if (optType == "C")return CallDelta();
    else return PutDelta();
}

// Modifier functions
void EuropeanOption::toggle()
{ // Change option type (C/P, P/C)

    if (optType == "C")optType = "P";
    else optType = "C";
}

Article_price::Article_price()
{
}
void Article_price::test(){
    std::cout<<"This is a test function"<<std::endl;
}
