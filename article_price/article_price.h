#ifndef ARTICLE_PRICE_H
#define ARTICLE_PRICE_H
#include <string>
#include "article_price_global.h"
using namespace std;
//class ARTICLE_PRICESHARED_EXPORT Article_price
class  Article_price
{

public:
    Article_price();
    void test();
};
class OptionGreeks{
public:
    int optVolume;
    double U;
    double K;
    double r;
    double T;
    double b;
    double sig;
    double cDelta;
    double pDelta;
    double gamma;
    double theta;
    double vega;
    string optype;
    string direction;
    string instrumentID;
};
class EuropeanOption
{
private:

    void init();	// Initialise all default values
    void copy(const EuropeanOption& o2);




public:
    // Public member data for convenience only
    double r;		// Interest rate
    double sig;		// Volatility
    double K;		// Strike price
    double T;		// Expiry date
    double U;		// Current underlying price
    double b;		// Cost of carry
    double pi=3.1415;
    string optType;	// Option name (call, put)


public:
    // 'Kernel' functions for option calculations
    double CallPrice() const;
    double PutPrice() const;
    double CallDelta() const;
    double PutDelta() const;
    // Constructors
    EuropeanOption();	 // Default call option
    EuropeanOption(const EuropeanOption& option2);	// Copy constructor
    EuropeanOption (const string& optionType);	// Create option type

    // Destructor
    virtual ~EuropeanOption();

    // Assignment operator
    EuropeanOption& operator = (const EuropeanOption& option2);

    // Functions that calculate option price and (some) sensitivities
    double Price() const;
    double Delta() const;

    // Modifier functions
    void toggle();		// Change option type (C/P, P/C)
    // Gaussian (normal) distribution function
    double n(double x)const;
    // The approximation to the cumulative normal distribution function
    double N(double x)const;
    double bsm_option_gamma(double S0,double K,double T,double r,double sigma);
    void bsm_option_greeks(OptionGreeks* og);
};


#endif // ARTICLE_PRICE_H
