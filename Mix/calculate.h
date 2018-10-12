#pragma once
#include "Strategy.h"

#include <fstream>
#include <vector>
#include <list>
#include <cmath>
#include <string.h>
using namespace std;

//using namespace std;						//ΪʲôҪ���������Ȼ�������ò��ˣ�����
using std::vector;							// ����Ҫ���������Ȼ��֪����ô����
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
    //	double vn;				// //K�ߵĲ�����
    //};
    //vector<Kdata > vectorKData;




    void MACD(vector<Strategy::Kdata> &vectorKData, int n_fast, int n_middle, int n_slow);				//����MACD,���������

    double RSI(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    double KDJ(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    void Bulling(vector<Strategy::Kdata> &vectorKData, int look_back_window, double bulling_set);			//ֱ����K�����޸�

    double EWMA(const vector<Strategy::Kdata> &vectorKData, int look_back_window);				//ǰN��K�ߵļ�Ȩƽ��ֵ�����ص�ǰ��ֵ

    void MA(vector<Strategy::Kdata> &vectorKData);

    void ATR(vector<Strategy::Kdata> &vectorKData, int look_back_window);

    void OBV(vector<Strategy::Kdata> &vectorKData);									//����OBV


    void dural_trust(vector<Strategy::Kdata> &vectorKData,double ks);						//������һ�������ʼ�������ͻ����


    //double double_ema(vector<double> df, int look_back_window);					//�����ƶ���ֵ

    double double_ema(vector<double> double_data, int look_back_window);

    double double_std(vector<Strategy::Kdata>vectorKData, int look_back_window);					//�ƶ���׼��





};

