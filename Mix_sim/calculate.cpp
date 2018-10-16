#pragma once
#include <cstring>
#include "calculate.h"
//#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
calculate::calculate()
{
	num = 20;
}


calculate::~calculate()
{
}
//当前k线没走完，要不要实时计算当前k的技术指标。还是更新下一个K线时，马上计算上一根K线并得出结论
void calculate::ATR(vector<Strategy::Kdata> &vectorKData, int look_back_window)
{
    double result[2] = {0};
    int vkdSize = vectorKData.size();
    if (vkdSize >= 3)
    {
        double t1=vectorKData[vkdSize - 2].highPrice - vectorKData[vkdSize - 2].lowPrice;
        double t2=abs(vectorKData[vkdSize - 2].highPrice - vectorKData[vkdSize - 3].closePrice);
        double t3=abs(vectorKData[vkdSize - 3].closePrice - vectorKData[vkdSize - 2].lowPrice);
        double maxt=max(t1,t2);
        maxt=max(maxt,t3);
        vectorKData[vkdSize - 2].TR = maxt;
    }
    if ((vkdSize-1) >= look_back_window)
    {
        double  sumP{ 0 }, meanp{0};
        for (int i = look_back_window; i > 0; i--)				//当前K的技术指标要计算么？？i=1，表示从上一根K线开始计算
        {
            sumP += vectorKData[vkdSize - 1 - i].TR;

        }
        meanp = sumP / look_back_window;
        vectorKData[vkdSize - 2].ATR = meanp;
    }
}
void calculate::MACD(vector<Strategy::Kdata> &vectorKData, int n_fast, int n_middle, int n_slow)			//n_fast:9, n_middle ：12，n_slow：26
{
    double tem_fast{ 0 }, tem_middle{ 0 }, tem_slow{ 0 };
    double result_macd =  { 0 };					// result_macd[0],result_sign[1], result_diff[2]
    vector <double> macd_data;


    if (int(vectorKData.size()) >= n_slow)
    {

        tem_middle = EWMA(vectorKData, n_middle);
        tem_slow = EWMA(vectorKData, n_slow);

        vectorKData[vectorKData.size() - 2].macd_diff = tem_middle - tem_slow;
        std::cout << "fast:" << tem_middle << "| slow:" << tem_slow << endl;
        LOG(INFO)<<"fast="+boost::lexical_cast<string>(tem_middle)+",slow="
                   +boost::lexical_cast<string>(tem_slow)+",diff="
                   +boost::lexical_cast<string>(vectorKData[vectorKData.size() - 2].macd_diff);

        if (int(vectorKData.size()) >= n_slow + n_fast)
        {
            double  sumP{ 0 }, meanp{0};
            for (int i = 0; i < n_fast; i++)
            {
                sumP += vectorKData[vectorKData.size() - 2 - i].macd_diff;

            }
            meanp = sumP / n_fast;
            vectorKData[vectorKData.size() - 2].macd_dea = meanp;
            vectorKData[vectorKData.size() - 2].macd_sign = vectorKData[vectorKData.size() - 2].macd_diff - meanp;
            LOG(INFO)<<"macd_dea="+boost::lexical_cast<string>(vectorKData[vectorKData.size() - 2].macd_dea)+",diff="
                       +boost::lexical_cast<string>(vectorKData[vectorKData.size() - 2].macd_diff);
        }else{
            LOG(INFO)<<"vectorKData.size()="+boost::lexical_cast<string>(vectorKData.size())+"<n_slow + n_fast="
                       +boost::lexical_cast<string>(n_slow + n_fast);
        }
    }


}
double calculate::EWMA(const vector<Strategy::Kdata> &vectorKData, int look_back_window)
{
    double meanP{ 0 };
    int vkdSize = vectorKData.size();
    if (vkdSize >= look_back_window+1)
    {
        double  sumP{ 0 };
        for (int i = 1; i < look_back_window+1; i++)				//当前K的技术指标要计算么？？i=1，表示从上一根K线开始计算
        {
            sumP += vectorKData[vkdSize - 1 - i].closePrice;
            //cout <<"i: "<<i<<" close: "<< vectorKData[vectorKData.size() - 1 - i].closePrice << endl;

        }
        meanP = sumP / (look_back_window);
        LOG(INFO)<<"MAC:sum="+boost::lexical_cast<string>(sumP)+",mean="+boost::lexical_cast<string>(meanP);
    }
    //cout << "mean " << look_back_window << " is " << meanP << endl;
    return meanP;
}
void calculate::MA(vector<Strategy::Kdata> &vectorKData)
{

    if(vectorKData.size() < 2){
        return;
    }
    vectorKData[vectorKData.size() - 2].ma5 = EWMA(vectorKData,5);
    vectorKData[vectorKData.size() - 2].ma10 = EWMA(vectorKData,10);
    vectorKData[vectorKData.size() - 2].ma20 = EWMA(vectorKData, 20);
    //vectorKData[vectorKData.size() - 1].ma15 = EWMA(vectorKData,15);
    //
    //

}
double calculate::KDJ(vector<Strategy::Kdata> &vectorKData, int look_back_window)
{
    double stok{ 0 };
    if (vectorKData.size() >= look_back_window)
    {
        stok = (vectorKData[vectorKData.size() - 1].closePrice - vectorKData[vectorKData.size() - 1].lowPrice) / (vectorKData[vectorKData.size() - 1].highPrice - vectorKData[vectorKData.size() - 1].lowPrice);
    }
    return stok;
}
double calculate::double_ema(vector<double> double_data, int look_back_window)			//计算窗口平均值
{
    double meanP{ 0 };

    if (double_data.size() >= look_back_window)
    {
        double  sumP{ 0 };
        for (int i = 0; i < look_back_window; i++)
        {
            sumP += (double_data[double_data.size() - 1 - i]);

        }
        meanP = sumP / look_back_window;

    }
    /*else
    {
        meanP = (vectorKData[vectorKData.size() - 1].closePrice);
    }*/
    //cout << "mean " << look_back_window << " is " << meanP << std::endl;
    return meanP;
}

