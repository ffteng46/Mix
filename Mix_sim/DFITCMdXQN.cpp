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
extern unordered_map<string, InstrumentInfo*> instruments;			//��Լ��Ϣ
extern unordered_map<string, MarketData*> instrinfo;//market data
//���ջ�������С
#define MAXLINE 2048

int sockfd;//�������׽���
struct sockaddr_in servaddr;//��������ַ
struct sockaddr_in cliaddr;//�ͻ��˵�ַ
socklen_t len = sizeof(cliaddr);//�ͻ��˵�ַ����

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
	DFITCMarketDataFieldXQN recvMarketData;//����ṹ������
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
    int volume = marketdata.Volume;//�ɽ���
    double turnover = marketdata.Turnover;
    double multiply = getMultipler(instrumentID);
    double tickPrice = getPriceTick(instrumentID);

    MarketData* marketdatainfo;
    unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
    if(it == instrinfo.end()){
        marketdatainfo = new MarketData();//���浱ǰ��������
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
			sprintf(buf, "[�г�˵��]:%s, [��Լ����]:%s, [����ʱ��]:%s, [����]:%03d, [���¼�]:%lf, [�ɽ���]:%d, [�ɽ����]:%lf, [�ֲ���]:%lf, [��һ��]:%lf, [��һ��]:%d, [��һ��]:%lf, [��һ��]:%d\n",
				recvMarketData.Market,//�г�˵��
				recvMarketData.Instrument,//��Լ����
				recvMarketData.UpdateTime,//����ʱ��
				recvMarketData.UpdateMillisec,//����
				checkDouble(recvMarketData.LastPrice),//���¼�
				recvMarketData.Volume,//�ɽ���
				checkDouble(recvMarketData.Turnover),//�ɽ����
				checkDouble(recvMarketData.OpenInterest),//�ֲ���
				checkDouble(recvMarketData.BidPrice),//��һ��
				recvMarketData.BidVolume,//��һ��
				checkDouble(recvMarketData.AskPrice),//��һ��
				recvMarketData.AskVolume//��һ��
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
            int volume = recvMarketData.Volume;//�ɽ���
            double turnover = recvMarketData.Turnover;
            double multiply = getMultipler(instrumentID);
            double tickPrice = getPriceTick(instrumentID);

            MarketData* marketdatainfo;
            unordered_map<string, MarketData*>::iterator it = instrinfo.find(instrumentID);
            if(it == instrinfo.end()){
                marketdatainfo = new MarketData();//���浱ǰ��������
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

		sockfd = socket(AF_INET, SOCK_DGRAM, 0); //����UDP�׽���
	
		//��ʼ����������ַ
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);//���ձ����κ���������
		servaddr.sin_port = htons(port);//�����������ݶ˿�
	
		//�趨SO_REUSEADDR��������Ӧ�ð�ͬһ�����ض˿ڽ������ݰ�
		int reuse = 1;
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
		{
			perror("set SO_REUSEADDR error");
			//close(sockfd);
			//exit(-3);
		}
		printf("step %d\n", 1);
		//����������ַ�ͷ������׽��ְ�
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
		//���ûػ����
		int loop = 1;
		if(setsockopt(sockfd,IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop)) < 0)
		{
			perror("setsockopt():IP_MULTICAST_LOOP");
			//exit(-5);
		}
		printf("step %d\n", 4);
		//����Ҫ�����鲥�ĵ�ַ
		struct ip_mreq mreq;
		bzero(&mreq, sizeof (struct ip_mreq)); 
		mreq.imr_multiaddr.s_addr = inet_addr(multicast_addr); //�㲥��ַ
		//���÷����鲥��Ϣ��Դ�����ĵ�ַ��Ϣ
		//mreq.imr_interface.s_addr = htonl (INADDR_ANY);
		mreq.imr_interface.s_addr = addr.s_addr;
		//�ѱ��������鲥��ַ��������������Ϊ�鲥��Ա��ֻ�м���������յ��鲥��Ϣ
		int errid = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
		if (errid != 0)
		{
			printf ("setsockopt error=%d.",errid);
			//exit(-6);
		}
		printf("step %d\n", 5);
		//����TTL
		unsigned char ttl = 255;
		if(setsockopt(sockfd,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl)) < 0)
		{
			perror("setsockopt():IP_MULTICAST_TTL\n");
		}
		printf("step %d\n", 6);
		//���÷�����recv
		int flag;
		if (flag = fcntl(sockfd, F_GETFL, 0) < 0)
			perror("get flag error");
		flag |= O_NONBLOCK;
		if (fcntl(sockfd, F_SETFL, flag) < 0)
			perror("set flag error");
		//������ӡ�߳�
		int err = pthread_create(&pid, NULL, XQNprintQueue, NULL);
		if (0 != err)
			printf("can't create thread: %s\n", strerror(err));
		socketRecv();
		return 0;
}
