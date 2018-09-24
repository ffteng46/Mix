#pragma once
#include "./ctpapi/ThostFtdcMdApi.h"
#include "./ctpapi/ThostFtdcTraderApi.h"
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
	//��������
	TThostFtdcInstrumentIDType InstrumentID;
    //"0":long;"1":short;"3":default
    string mainDirection="3";
    bool genKLine_15S=false;
    bool genKLine_15m=false;
	char exchangeCode[20];
	char code[20];  // ���̺�Լ��
	int nTask;		// ������
	bool isOn;
	//bool isMarketOpen;
	double tickPrice;		//��С�䶯��λ
	double tickValue;		//��С��ֵ
	int nSec;

	int UpdateTime;				//���뻯��ʱ��
	
	int timecondition;			//ʱ�������������ǰ1Сʱ������ǰһСʱ��
	int trend_dirction;			//���Ʒ����и�Ƶ̽�ⷽ��
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
    int firstMetricVolume=1;//volume of first open
    double firstOpenPrice;//limit price of first open
    int nTickMoveSL=3;//the first adjust range.default is 3 tick.when satisfy,stop loss price will move up or down 1 tick.
    double stopLossPrice=0;//after first open,when lastPrice touch this price will trigger stop loss action.
    int stopLossPriceTick=1;//stopLossPrice=firstOpenPrice+stopLossPriceTick*tickPrice
    int nJumpTriggerSL=2;//when lastPrice touch n jump trigger stop loss range(tipically 2 double of nTickMoveSL),stop loss stopLossPrice will set to last price -njump*tick
    int oneNormalGap=2;
    int oneSweetGap=2;
    int oneNormalGrade=3;
    int oneSweetGrade=2;
    int oneNormalVolume=1;
    int twoGrade=8;//how mann grade for two status;
    int twoGap=1;//how many ticks in each grade for two status;
    int srsptn=1;//#��ͷ����Ӳ�ʱ,��ͷֹӯ����shortReverseStopProfitTickNums
    int lrsptn=1;//#��ͷ����Ӳ�ʱ,��ͷֹӯ����longReverseStopProfitTickNums
    bool isFirst=false;//if first open order is executed.
    bool existStock=false;
    double maxPrice=0;
    double minPrice=0;
    bool shake=false;//
    double unlockPrice=0;
    int protectVolume=1;//volume of protection order
    int relockATRNums=1;//when to relock after unlock.unlockprice-relockATRNums*ATR60C*tickprice
    //############################################����ָ�����
	vector<double> bullings;					//���ִ�����
	int bulling_window{ 20 };							//�������
	double bulling_set{1.5};							//�������
	double bulling{ 0 }, up_line{ 0 }, down_line{ 0 };	//���ֵ
	double tem_std{ 0 };

	vector<double> stoks;						//Kdj
	int kdj_window{ 20 };
	double stok{ 0 }, stod{ 0 };

	vector<double> macd_signs;
	int fast{ 9}, middle{ 12 }, slows{ 26 };							//����
	double current_diff{ 0 }, current_dea{ 0 }, current_sign{ 0 };					//���

	double results_RSI{ 0 };
	double RSI_window{ 5 };

	int ATR_window{ 5 };


	//}strategy_info_long,strategy_info_short;


	int set_deepth;						//����ҵ�����ȣ�һ���Լ��6��
	int set_tick_frequenc;					//���򵥼�࣬һ����3��

	

	int do_stopprofit_long{0};								//            10����һ��ֵ�ڳɽ��������׿���-1||-2*tick��  15����һ��ֵ�ڵĸ�Ƶ������
														//			  20���ڶ���ֵ�ڳɽ�������100:��Ƶ��;50:�ű굥; 25�� �ڶ���ֵ�ڸ�Ƶ������
														//            30��������ֵ�ڳɽ�������						35��������ֵ�ڸ�Ƶ������



	int do_stopprofit_short{ 0 };						// ��ͬ����ֵ�����ڣ���ʼֹӯ







	int openVolume;
	double threshold;
	double threshold2;
	double averagePrice1;	// 15�����ھ���
	double averagePrice2;	// ��ǰ���̾���
	double volatility;		//15���Ӳ�����



	


	vector<double> trend_break_num;				//�ȴ�ͻ�Ƶļ�����
	vector<int> trend_N_data;					//��¼ǰһ������״̬��
	bool stop_wait;								//��ѯ��true��ʾ��ѯ��������ʼֹӯƽ�֣�

	int breakwaittime;							//ͻ�Ƶڼ�������ȴ�ʱ�䣬Ĭ��30����
	bool is_frist;								//false ��ʾ���ǵ�һ�Σ�true ��ʾͻ�����񣬿�ʼ�������в���
	int stop_profit{2};							//����ֹӯ����,2��
	double stop_loss;

	///////////////////////////					//����
	int risk_lock;						//��ع���0��ʾ����ƽ�֣�1��ʾ��ѯ�������Լ�ƽ�֣�2��ʾ������أ�ȫ������
	bool is_traded;						//30�ʱ����Ƿ�ɽ�
	double traded_price;				//�ɽ���
	//const int fibs[20] = { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89 };			//쳲��������� start_passive =true;
	


	//######################################  ����ָ��
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



	int INDEX_15s;
	int INDEX_30s;
	int INDEX_15m;
	int INDEX_1h;

	int kPeriod;						//K������ 60��Ϊ1����K��
	int kBarNum;						// K�ߴ���
	int kIndex_15s;						//15 ��
	int kIndex_30s;						//30 ��
	int kIndex_15m;						//15����
	int kIndex_1h;						//1Сʱ

	struct Kdata
	{
		int	updatetimes;
		double openPrice;
		double closePrice;
		double lowPrice;
		double highPrice;
		int nIndex;


		//����ָ��
		double TR;				// //K�ߵĲ�����
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
		double volume;				//�ɽ���


		double up_range;
		double down_range;




	};
    int k15sLen=10;
    int k15mLen=10;
    int k15sLenForMean=100;
    vector<Kdata > KData_15s, KData_30s, KData_1m, KData_10m, KData_15m, KData_1h,KDataMa_15s;						//k������
    Kdata* trueKData15S;
    Kdata* trueKData15M;
    Kdata* newestData15M;
    //Kdata* lastTwoK;//the last second k line

	// ���ܺ��� ����
    void RunMarketData(EESMarketDepthQuoteData *pDepthMarketData);	// ����������Ϣ

	//��������ָ��  
	void run_tech_lib(vector<Kdata > &vectorKData);							//���㼼��ָ�겢���浽�ṹ�壺  tech_result &ta_result
	int cal_Kindex(int h, int m, int s, int black_window);					//����K��
    void creat_Kline(EESMarketDepthQuoteData* tickData, int current_index, vector<Kdata > &vectorKData);		//����K��
    void update_kline(EESMarketDepthQuoteData* tickData, vector<Kdata > &vectorKData,bool is_15min);

};


