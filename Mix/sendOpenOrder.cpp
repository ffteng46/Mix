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
			if ((*it)->do_hft==10 && false)				//首次开仓了，且持仓且超过N3，开始启动高频探测单
			{
				(*it)->hft_test_order(1);
				(*it)->tradeRecord << "高频探测做多1" << endl;
			}
			else if ((*it)->do_hft == 20 && false)
			{
				(*it)->hft_test_order(1);
				(*it)->tradeRecord << "高频探测做多2" << endl;

			}
			else if ((*it)->do_hft == -10 && false)
			{
				(*it)->hft_test_order(-1);
				(*it)->tradeRecord << "高频探测做空1" << endl;

			}
			else if ((*it)->do_hft == -20 && false)
			{
				(*it)->hft_test_order(-1);
				(*it)->tradeRecord << "高频探测做空2" << endl;

			}

			
		}

		Sleep(500);



	}


}