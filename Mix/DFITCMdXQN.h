#ifndef DFITCMDXQN_H
#define DFITCMDXQN_H
#include "LockFreeQueue2.h"

#pragma pack(1)

typedef struct DFITCMarketDataFieldXQN
{
    char Market[3];					//市场说明
    char Status;					//字段状态
    char Instrument[7];				//合约代码
    char UpdateTime[9];				//更新时间
    int UpdateMillisec;				//最后更新时间毫秒
    double LastPrice;				//最新价
    int	Volume;					//成交量
    double Turnover;				//成交金额
    double OpenInterest;				//持仓量
    double BidPrice;				//买一价
    int BidVolume;					//买一量
    double AskPrice;				//卖一价
    int AskVolume;					//卖一量
}DFITCMarketDataFieldXQN;
int startXQN();
void addMarketData(DFITCMarketDataFieldXQN &marketdatainfo);
#endif // DFITCMDXQN_H
