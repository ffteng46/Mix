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

	int countNum;				//���Ե���
	int setOpenTime;			//����ʱ��
	int setTimeError;			//��ǰʱ��
	int setFrequency;				//����Ƶ��
	int setOrderNum;			//��������


	SYSTEMTIME currentTime;
	double waitingOpenTime1;
	double waitingOpenTime2;
	double waitingOpenTime3;

	LARGE_INTEGER m_CurrentTime;
	LARGE_INTEGER m_WaitingOpenBeginTime;
	LARGE_INTEGER m_nFreq;

};





