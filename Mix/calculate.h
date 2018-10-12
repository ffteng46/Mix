#pragma once
#include "Strategy.h"

#include <fstream>
#include <vector>
#include <list>
#include <cmath>
#include <string.h>
using namespace std;

//using namespace std;						//为什么要加这个，不然容器就用不了？？？
using std::vector;							// 必须要加这个，不然不知道怎么引用
extern vector<Strategy* > allRunningTasks;

class calculate
{
public:
    calculate();
    ~calculate();
public:


    long product(long a, long b);
    //long sum(long a, long b);
    //void GetData(int& number, char name[]);

    //double sumarray(const double data[], size_t len, double(*pfun)(double));
    //double cubed(double x);
    //double squared(double x);


    char* parse(const char* str);
    int num;

    //struct Kdata
    //{
    //	double openPrice;
    //	double closePrice;
    //	double lowPrice;
    //	double highPrice;
    //	int nIndex;
    //	double vn;				// //K线的波动率
    //};
    //vector<Kdata > vectorKData;




    void MACD(vector<Strategy::Kdata> &vectorKData, int n_fast, int n_middle, int n_slow);				//返回MACD,滚动计算后，

    double RSI(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    double KDJ(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    void Bulling(vector<Strategy::Kdata> &vectorKData, int look_back_window, double bulling_set);			//直接在K线上修改

    double EWMA(const vector<Strategy::Kdata> &vectorKData, int look_back_window);				//前N根K线的加权平均值，返回当前的值

    void MA(vector<Strategy::Kdata> &vectorKData);

    void ATR(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    void OBV(vector<Strategy::Kdata> &vectorKData);									//计算OBV


    void dural_trust(vector<Strategy::Kdata> &vectorKData,double ks);						//根据上一根波动率计算上下突破线


    //double double_ema(vector<double> df, int look_back_window);					//计算移动均值

    double double_ema(vector<double> double_data, int look_back_window);

    double double_std(vector<Strategy::Kdata>vectorKData, int look_back_window);					//移动标准差





};

