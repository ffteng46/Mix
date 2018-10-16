#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <string.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <math.h>
#include "LockFreeQueue2.h"
#include "property.h"
#include "DFITCMdXQN.h"
#include "TimeProcesser.h"
using namespace LOCK_FREE;
extern unordered_map<string, InstrumentInfo*> instruments;			//合约信息
extern unordered_map<string, MarketData*> instrinfo;//market data
//接收缓冲区大小
#define MAXLINE 2048

int sockfd;//服务器套接字
struct sockaddr_in servaddr;//服务器地址
struct sockaddr_in cliaddr;//客户端地址
socklen_t len = sizeof(cliaddr);//客户端地址长度

double checkDouble(double val)
{
    return (fabs(val) > 9999999999)?0:val;
}


#pragma pack()

typedef ArrayLockFreeQueue<DFITCMarketDataFieldXQN,	MULTI_THREAD_FALSE, MULTI_THREAD_FALSE>	ArrayDataQueue;
ArrayDataQueue queue,testQ;			// the queue for recved data from socket
pthread_t pid;

static void socketRecv()
{
	DFITCMarketDataFieldXQN recvMarketData;//行情结构体数据
	int n = 0;
	memset(&recvMarketData,0,sizeof(recvMarketData));
	while(1)
	{
		n = recvfrom(sockfd, &recvMarketData, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
		if(n < 0)
			continue;
		queue.push(recvMarketData);
		memset(&recvMarketData,0,sizeof(recvMarketData));
	}
	return;
}
double upperPrice=0;
double lowerPrice=0;
void addMarketData(DFITCMarketDataFieldXQN &marketdata){
    if(upperPrice < marketdata.LastPrice){
        upperPrice = marketdata.LastPrice;
    }
    if(lowerPrice > marketdata.LastPrice){
        lowerPrice = marketdata.LastPrice;
    }
    //assemble marketdata
    string instrumentID=boost::lexical_cast<string>(marketdata.Instrument);
    int volume = marketdata.Volume;//成交量
    double turnover = marketdata.Turnover;
    double multiply = getMultipler(instrumentID);
    double tickPrice = getPriceTick(instrumentID);

    MarketData* marketdatainfo;
    unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
    if(it == instrinfo.end()){
        marketdatainfo = new MarketData();//保存当前行情数据
        marketdatainfo->updateTime = string(marketdata.UpdateTime);
        strcpy(marketdatainfo->updateTime_char,marketdata.UpdateTime);
        marketdatainfo->instrumentID = instrumentID;
        marketdatainfo->volume = volume;
        marketdatainfo->bidPrice = marketdata.BidPrice;
        marketdatainfo->askPrice = marketdata.AskPrice;
        marketdatainfo->lastPrice = marketdata.LastPrice;
        marketdatainfo->highestPrice = upperPrice;
        marketdatainfo->lowestPrice = lowerPrice;
        marketdatainfo->turnover = turnover;
        if(volume !=0 && multiply != 0){
            marketdatainfo->simPrice = turnover/volume/multiply;
            //marketdatainfo->simPrice = turnover/volume;
        }else{
            LOG(ERROR) << "OnRtnDepthMarketData:volume or tickPrice is zero!!!";
            return;
        }
        instrinfo[instrumentID] = marketdatainfo;
    }else{
        marketdatainfo = it->second;
        int prevolume = marketdatainfo->volume;
        double preTurnover = marketdatainfo->turnover;
        int tmpVolume = volume - prevolume;
        double tmpTurnover = turnover - preTurnover;
        marketdatainfo->volume = volume;
        marketdatainfo->turnover = turnover;
        marketdatainfo->updateTime = string(marketdata.UpdateTime);
        strcpy(marketdatainfo->updateTime_char,marketdata.UpdateTime);
        marketdatainfo->bidPrice = marketdata.BidPrice;
        marketdatainfo->askPrice = marketdata.AskPrice;
        marketdatainfo->lastPrice = marketdata.LastPrice;
        marketdatainfo->highestPrice = upperPrice;
        marketdatainfo->lowestPrice = lowerPrice;
        if(multiply != 0&&tmpVolume != 0){
            //marketdatainfo->simPrice = tmpTurnover/tmpVolume;//multiply needed in shfe
            marketdatainfo->simPrice = tmpTurnover/tmpVolume/multiply;
        }
    }
    metricProcesserForSingleThread(marketdatainfo);
    //testQ.push(marketdatainfo);
}


void *XQNprintQueue(void *)
{
	DFITCMarketDataFieldXQN recvMarketData;
	memset(&recvMarketData,0,sizeof(recvMarketData));
	char buf[512] = {0};
	while (1)
	{
        if (testQ.pop(recvMarketData))
        {/*
			sprintf(buf, "[市场说明]:%s, [合约代码]:%s, [更新时间]:%s, [毫秒]:%03d, [最新价]:%lf, [成交量]:%d, [成交金额]:%lf, [持仓量]:%lf, [买一价]:%lf, [买一量]:%d, [卖一价]:%lf, [卖一量]:%d\n",
				recvMarketData.Market,//市场说明
				recvMarketData.Instrument,//合约代码
				recvMarketData.UpdateTime,//更新时间
				recvMarketData.UpdateMillisec,//毫秒
				checkDouble(recvMarketData.LastPrice),//最新价
				recvMarketData.Volume,//成交量
				checkDouble(recvMarketData.Turnover),//成交金额
				checkDouble(recvMarketData.OpenInterest),//持仓量
				checkDouble(recvMarketData.BidPrice),//买一价
				recvMarketData.BidVolume,//买一量
				checkDouble(recvMarketData.AskPrice),//卖一价
				recvMarketData.AskVolume//卖一量
				);
            printf("%s\n", buf);*/
            ////
            /// \brief memset
            ///
            ///
            string tmpmkdata=

                    ";instrumentID="+string(recvMarketData.Instrument)+
                    ";updateTime="+string(recvMarketData.UpdateTime)+
                    ";bidPrice1="+boost::lexical_cast<string>(recvMarketData.BidPrice)+
                    ";bidVolume1="+boost::lexical_cast<string>(recvMarketData.BidVolume)+
                    ";askPrice1="+boost::lexical_cast<string>(recvMarketData.AskPrice)+
                    ";askVolume1="+boost::lexical_cast<string>(recvMarketData.AskVolume)+
                    ";lastPrice=" + boost::lexical_cast<string>(recvMarketData.LastPrice) +
                    ";volume="+boost::lexical_cast<string>(recvMarketData.Volume)+
                    ";turnover="+boost::lexical_cast<string>(recvMarketData.Turnover);
                    //";avgprice=" + boost::lexical_cast<string>(recvMarketData.AskPrice) +
                    //";simPrice=" + boost::lexical_cast<string>(simPrice);
            LOG(INFO) << tmpmkdata;
			memset(buf, 0, 512);
            if(upperPrice < recvMarketData.LastPrice){
                upperPrice = recvMarketData.LastPrice;
            }
            if(lowerPrice > recvMarketData.LastPrice){
                lowerPrice = recvMarketData.LastPrice;
            }
            //assemble marketdata
            string instrumentID=boost::lexical_cast<string>(recvMarketData.Instrument);
            int volume = recvMarketData.Volume;//成交量
            double turnover = recvMarketData.Turnover;
            double multiply = getMultipler(instrumentID);
            double tickPrice = getPriceTick(instrumentID);

            MarketData* marketdatainfo;
            unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
            if(it == instrinfo.end()){
                marketdatainfo = new MarketData();//保存当前行情数据
                marketdatainfo->updateTime = string(recvMarketData.UpdateTime);
                strcpy(marketdatainfo->updateTime_char,recvMarketData.UpdateTime);
                marketdatainfo->instrumentID = instrumentID;
                marketdatainfo->volume = recvMarketData.Volume;
                marketdatainfo->bidPrice = recvMarketData.BidPrice;
                marketdatainfo->askPrice = recvMarketData.AskPrice;
                marketdatainfo->lastPrice = recvMarketData.LastPrice;
                marketdatainfo->highestPrice = upperPrice;
                marketdatainfo->lowestPrice = lowerPrice;
                marketdatainfo->turnover = turnover;
                if(volume !=0 && multiply != 0){
                    marketdatainfo->simPrice = turnover/volume/multiply;
                    //marketdatainfo->simPrice = turnover/volume;
                }else{
                    LOG(ERROR) << "OnRtnDepthMarketData:volume or tickPrice is zero!!!";
                    continue;
                }
                instrinfo[instrumentID] = marketdatainfo;
            }else{
                marketdatainfo = it->second;
                int prevolume = marketdatainfo->volume;
                double preTurnover = marketdatainfo->turnover;
                int tmpVolume = volume - prevolume;
                double tmpTurnover = turnover - preTurnover;
                marketdatainfo->volume = volume;
                marketdatainfo->turnover = turnover;
                marketdatainfo->updateTime = string(recvMarketData.UpdateTime);
                strcpy(marketdatainfo->updateTime_char,recvMarketData.UpdateTime);
                marketdatainfo->bidPrice = recvMarketData.BidPrice;
                marketdatainfo->askPrice = recvMarketData.AskPrice;
                marketdatainfo->lastPrice = recvMarketData.LastPrice;
                marketdatainfo->highestPrice = upperPrice;
                marketdatainfo->lowestPrice = lowerPrice;
                if(multiply != 0&&tmpVolume != 0){
                    //marketdatainfo->simPrice = tmpTurnover/tmpVolume;//multiply needed in shfe
                    marketdatainfo->simPrice = tmpTurnover/tmpVolume/multiply;
                }
            }
            metricProcesserForSingleThread(marketdatainfo);
		}
		else
			sched_yield();
	}
}

int startXQN()
{
		char multicast_addr[20] = "226.100.100.100";
		char local_addr[20] = "10.10.10.145";
		long port = 10008;
		printf("port = %ld, multicast_addr = %s local_addr = %s\n",port, multicast_addr, local_addr);

		sockfd = socket(AF_INET, SOCK_DGRAM, 0); //创建UDP套接字
	
		//初始化服务器地址
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//接收本机任何网卡数据
		servaddr.sin_port = htons(port);//本机接收数据端口
	
		//设定SO_REUSEADDR，允许多个应用绑定同一个本地端口接收数据包
		int reuse = 1;
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
		{
			perror("set SO_REUSEADDR error");
			//close(sockfd);
			//exit(-3);
		}
		printf("step %d\n", 1);
		//将服务器地址和服务器套接字绑定
		if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
		{
			perror("bind socket error");
			//exit(-4);
		}
		printf("step %d\n", 2);
		struct in_addr addr = {0};
		addr.s_addr = inet_addr(local_addr);

		if(-1 == setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addr, sizeof(addr)))
		{
			printf("set error IP_MULTICAST_IF\n");
		}
		printf("step %d\n", 3);
		//设置回环许可
		int loop = 1;
		if(setsockopt(sockfd,IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
		{
			perror("setsockopt():IP_MULTICAST_LOOP");
			//exit(-5);
		}
		printf("step %d\n", 4);
		//设置要加入组播的地址
		struct ip_mreq mreq;
		bzero(&mreq, sizeof (struct ip_mreq)); 
		mreq.imr_multiaddr.s_addr = inet_addr(multicast_addr); //广播地址
		//设置发送组播消息的源主机的地址信息
		//mreq.imr_interface.s_addr = htonl (INADDR_ANY);
		mreq.imr_interface.s_addr = addr.s_addr;
		//把本机加入组播地址，即本机网卡作为组播成员，只有加入组才能收到组播消息
		int errid = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
		if (errid != 0)
		{
			printf ("setsockopt error=%d.",errid);
			//exit(-6);
		}
		printf("step %d\n", 5);
		//设置TTL
		unsigned char ttl = 255;
		if(setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl)) < 0)
		{
			perror("setsockopt():IP_MULTICAST_TTL\n");
		}
		printf("step %d\n", 6);
		//设置非阻塞recv
		int flag;
		if (flag = fcntl(sockfd, F_GETFL, 0) < 0)
			perror("get flag error");
		flag |= O_NONBLOCK;
		if (fcntl(sockfd, F_SETFL, flag) < 0)
			perror("set flag error");
		//创建打印线程
		int err = pthread_create(&pid, NULL, XQNprintQueue, NULL);
		if (0 != err)
			printf("can't create thread: %s\n", strerror(err));
		socketRecv();
		return 0;
}
