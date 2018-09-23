#include "stdafx.h"
#include "sendOpenOrder.h"
#include "strategy.h"
#include "api_config_extern.h"

#include <iostream>



extern vector<Strategy* > allRunningTasks;



sendOpenOrder::sendOpenOrder()
{
	
		
}


sendOpenOrder::~sendOpenOrder()
{

}


void sendOpenOrder::sendOrder()
{
	int set_num{ 0 };
	while (true)
	{
		for (vector<Strategy* >::iterator it = allRunningTasks.begin(); it != allRunningTasks.end(); it++)
		{
			if ((*it)->do_hft==10 && false)				//�״ο����ˣ��ҳֲ��ҳ���N3����ʼ������Ƶ̽�ⵥ
			{
				(*it)->hft_test_order(1);
				(*it)->tradeRecord << "��Ƶ̽������1" << endl;
			}
			else if ((*it)->do_hft == 20 && false)
			{
				(*it)->hft_test_order(1);
				(*it)->tradeRecord << "��Ƶ̽������2" << endl;

			}
			else if ((*it)->do_hft == -10 && false)
			{
				(*it)->hft_test_order(-1);
				(*it)->tradeRecord << "��Ƶ̽������1" << endl;

			}
			else if ((*it)->do_hft == -20 && false)
			{
				(*it)->hft_test_order(-1);
				(*it)->tradeRecord << "��Ƶ̽������2" << endl;

			}

			
		}

		Sleep(500);



	}


}