#pragma once
class sendOpenOrder
{
public:
	sendOpenOrder();
	~sendOpenOrder();


	void sendOrder();
	bool isdeal;
	bool start_thread;

	bool isWaitSendOrder1;
	bool isWaitSendOrder2;
	bool isWaitSendOrder3;

	SYSTEMTIME OpenTime;

	int countNum;				//测试单数
	int setOpenTime;			//开盘时间
	int setTimeError;			//提前时间
	int setFrequency;				//发单频率
	int setOrderNum;			//发单手数


	SYSTEMTIME currentTime;
	double waitingOpenTime1;
	double waitingOpenTime2;
	double waitingOpenTime3;

	LARGE_INTEGER m_CurrentTime;
	LARGE_INTEGER m_WaitingOpenBeginTime;
	LARGE_INTEGER m_nFreq;

};





