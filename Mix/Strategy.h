#pragma once
#include "EESQuoteDefine.h"
#include <fstream>
#include <vector>
#include<map>
//#include <cstring>
using namespace std;

class Strategy
{
public:
    Strategy();
    ~Strategy();
    EESMarketDepthQuoteData tickData;
    //基础资料
//	TThostFtdcInstrumentIDType InstrumentID;
    //"0":long;"1":short;"3":default
    string mainDirection="3";
    bool genKLine_15S=false;
    bool genKLine_15m=false;
    char exchangeCode[20];
    char code[20];  // 外盘合约名
    int nTask;		// 任务编号
    bool isOn;
    //bool isMarketOpen;
    double tickPrice;		//最小变动单位
    double tickValue;		//最小价值
    int nSec;

    int UpdateTime;				//秒针化的时间

    int timecondition;			//时间控制器：开盘前1小时，收盘前一小时。
    int trend_dirction;			//趋势方向，有高频探测方向
    //
    bool beginK15s=false;
    bool isK15sFirstItem=false;//first item in k list
    //string K15sFirstStatus="0";//tick for k item
    string firstOpenKLineType="0";//0"not init";"1":sun;"2":shadow,"3":flag
    //"10":sublogic
    string stgStatus="0";//this will be valid before first open order;will be invalid after first open.When invalid,priceStatus will become effective.
    string priceStatus="0";
    double limit[2]={0,0};
    double K2=1;
    double K1=1;
    int firstMetricVolume=2;//volume of first open
    double firstOpenPrice;//limit price of first open
    int nTickMoveSL=6;//the first adjust range.default is 3 tick.when satisfy,stop loss price will move up or down 1 tick.
    double stopLossPrice=0;//after first open,when lastPrice touch this price will trigger stop loss action.
    int stopLossPriceTick=1;//stopLossPrice=firstOpenPrice+stopLossPriceTick*tickPrice
    int nJumpTriggerSL=2;//when lastPrice touch n jump trigger stop loss range(tipically 2 double of nTickMoveSL),stop loss stopLossPrice will set to last price -njump*tick
    int oneNormalGap=1;
    int oneSweetGap=1;
    int oneNormalGrade=2;
    int oneSweetGrade=1;
    int oneNormalVolume=2;
    int oneSweetVolume=2;
    int firstToSweetTickNums=3;
    int sweetToNormalTickNums=3;
    int normalToTwoTickNums=3;
    int twoGrade=2;//how mann grade for two status;
    int twoGap=1;//how many ticks in each grade for two status;
    int srsptn=1;//#空头逆向加仓时,空头止盈点数shortReverseStopProfitTickNums
    int lrsptn=3;//#多头逆向加仓时,多头止盈点数longReverseStopProfitTickNums
    bool isFirst=false;//if first open order is executed.
    bool isTestInviron=false;//product or test,default is product
    bool existStock=false;
    double drawbackTickRate=4.0;
    int totalDates=0;
    int preTotalSeconds=0;
    int tonightSecs=7200;//rb from 21:00 to 23:00
    ///after lock,we will start to watch when to unlock when lastPrice over (lockPrice-lockWatchTickNums*tick)(when maindriection=0)
    ///unlockPrice call drawbackPrice=minPrice-drawbackTickNums*tick
    ///drawbackTickNums=round([(lockPrice-minprice)/tick]/drawbackTickRate)
    int lockWatchTickNums=4;
    ///relockprice=minprice-relockTickNums*tick
    int relockTickNums=2;//outside parameter
    double relockPrice=0;//self compute
    bool isAddOrderOpen=false;//if addition fbna order is open.
    double drawbackPrice=0;//when lockWatchTickNums active,then set drawbackPrice
    double maxPrice=0;
    double minPrice=0;
    double additionMinPrice=0;//self compute.use for another grade of fbna.only set in price stage of 6 and used in stage of 7.
    bool shake=false;//
    double unlockPrice=0;
    int gradeToProtect=1;
    int protectVolume=2;//volume of protection order
    int relockATRNums=1;//when to relock after unlock.unlockprice-relockATRNums*ATR60C*tickprice
    int watchUnlockATRNums=1;//when to begin watching unlock price.default 6
    int afterWatchUnlockATRNums=1;//after watching,when price touch the extrame point and then reserve back to n atr,stop loss immediate.
    int watchUnlockAnotherATRNums=1;//when unlock one side ,begin to watch after price breaks n atr.
    int afterWatchUnlockOtherATRNums=1;//When price touch extream price point,we will stop loss another side of order when price reverse back n atr.
    int timesOfStopLoss=4;//price over unlockPrice+timesOfStopLoss*twoGap will stop loss.
    int periodMinOne=15;//first period of minute,default is 15
    int periodMinTwo=13;
    int periodSecOne=15;//first period of second,default is 15
    int periodSecTwo=7;
    //############################################技术指标计算###########################################
    vector<double> bullings;					//布林带数组
    int bulling_window{ 20 };							//输入参数
    double bulling_set{1.5};							//输入参数
    double bulling{ 0 }, up_line{ 0 }, down_line{ 0 };	//输出值
    double tem_std{ 0 };

    vector<double> stoks;						//Kdj
    int kdj_window{ 20 };
    double stok{ 0 }, stod{ 0 };

    vector<double> macd_signs;
    int fast{ 9}, middle{ 12 }, slows{ 26 };							//输入
    double current_diff{ 0 }, current_dea{ 0 }, current_sign{ 0 };					//输出

    double results_RSI{ 0 };
    double RSI_window{ 5 };

    int ATR_window{ 5 };


    //}strategy_info_long,strategy_info_short;


    int set_deepth;						//反向挂单的深度，一般合约是6档
    int set_tick_frequenc;					//反向单间距，一般是3跳



    int do_stopprofit_long{0};								//            10：第一阈值内成交操作，首开价-1||-2*tick；  15：第一阈值内的高频发单；
                                                        //			  20：第二阈值内成交操作；100:高频单;50:信标单; 25： 第二阈值内高频发单；
                                                        //            30：第三阈值内成交操作；						35：第三阈值内高频发单；



    int do_stopprofit_short{ 0 };						// 不同的阈值区间内，开始止盈







    int openVolume;
    double threshold;
    double threshold2;
    double averagePrice1;	// 15分钟内均价
    double averagePrice2;	// 当前内盘均价
    double volatility;		//15分钟波动率






    vector<double> trend_break_num;				//等待突破的计数；
    vector<int> trend_N_data;					//记录前一次趋势状态；
    bool stop_wait;								//轮询：true表示轮询结束，开始止盈平仓；

    int breakwaittime;							//突破第几层网格等待时间，默认30分钟
    bool is_frist;								//false 表示不是第一次，true 表示突破网格，开始启动做市策略
    int stop_profit{2};							//设置止盈价跳,2跳
    double stop_loss;

    ///////////////////////////					//对锁
    int risk_lock;						//风控管理，0表示正常平仓，1表示轮询结束，对价平仓，2表示触发风控，全部锁仓
    bool is_traded;						//30笔报单是否成交
    double traded_price;				//成交价
    //const int fibs[20] = { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };			//斐波那契数列 start_passive =true;



    //######################################  技术指标
    struct tech_result
    {
        double MACD;
        double macd_dif;
        double macd_dea;

        double KDJ;
        double RSI;
        double TR;
        double ATR;

        double bolling_up;
        double bolling_mean;
        double bolling_down;
        double bolling_std;

    } ta_result,result_15,result_1m,result_1h;


    int INDEX_10s=-10;
    int INDEX_15s=-10;

    int INDEX_30s=-10;
    int INDEX_10m=-10;
    int INDEX_15m=-10;
    int INDEX_1h;

    int kPeriod;						//K线周期 60即为1分钟K线
    int kBarNum;						// K线窗口
    int kIndex_10s;						//10 秒
    int kIndex_15s;						//15 秒
    int kIndex_30s;						//30 秒
    int kIndex_10m;						//10分钟
    int kIndex_15m;						//15分钟
    int kIndex_1h;						//1小时

    struct Kdata
    {
        int	updatetimes;
        double openPrice;
        double closePrice;
        double lowPrice;
        double highPrice;
        int nIndex;


        //技术指标
        double TR;				// //K线的波动率
        double ATR;
        double macd_diff;
        double ma5;
        double ma10;
        double ma15;
        double ma20;
        double ma60;

        bool min3_last;
        double macd_dea;
        double macd_sign;
        double bolling_up;
        double bolling_mid;
        double bolling_down;
        double bolling_std;
        double OBV;
        double volume;				//成交量


        double up_range;
        double down_range;




    };
    int k15sLen=10;
    int k15mLen=10;
    int k15sLenForMean=100;
    vector<Kdata > KData_15s, KData_10s,KData_30s, KData_1m, KData_10m, KData_15m, KData_1h,KDataMa_15s,allK15sList;						//k线数组
    Kdata* trueKData15S;
    Kdata* trueKData10S;
    Kdata* trueKData15M;
    Kdata* trueKData10M;
    Kdata* newestData15M;
    //Kdata* lastTwoK;//the last second k line

    // 功能函数 发单
    void RunMarketData(EESMarketDepthQuoteData *pDepthMarketData);	// 处理内盘消息

    //基础策略指标
    void run_tech_lib(vector<Kdata > &vectorKData);							//计算技术指标并保存到结构体：  tech_result &ta_result
    int cal_Kindex(int h, int m, int s, int black_window);					//计算K线
    void creat_Kline(EESMarketDepthQuoteData* tickData, int current_index, vector<Kdata > &vectorKData);		//新增K线
    void update_kline(EESMarketDepthQuoteData* tickData, vector<Kdata > &vectorKData,bool is_15min);
    void setPreTotalSecs(int totalDates);
};


