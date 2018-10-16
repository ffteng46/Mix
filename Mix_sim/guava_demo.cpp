#include <iostream>
#include "guava_demo.h"



using std::cout;
using std::cin;
using std::endl;

extern string slLocalIP;
extern string slGuavaIP;
extern int slGuavaPort;
guava_demo::guava_demo(void)
{
	m_quit_flag = false;
}


guava_demo::~guava_demo(void)
{
}


void guava_demo::run()
{
	input_param();

	bool ret = init();
	if (!ret)
	{
		string str_temp;

		printf("\n�������ַ��˳�:\n");
		cin >> str_temp;

		return;
	}

	while(!m_quit_flag)
	{
		pause();
	}
	
	close();
}


void guava_demo::input_param()
{
    string local_ip = slLocalIP;
    cout << "SHENGLI GUAVA LOCALIP: " << local_ip << endl;
    string guava_ip = slGuavaIP;
    int guava_port = slGuavaPort;
    string guava_local_ip = local_ip;
    int guava_local_port = 23501;


    cout << "mk multicast ip: " << guava_ip << endl;
    cout << "mk multicast port: " << guava_port << endl;
    cout << "mk multicast local ip: " << guava_local_ip << endl;
    cout << "mk multicast local port: " << guava_local_port << endl;

	/// add by zhou.hu review 2014/4/24 ���þ���Ĳ���
	multicast_info temp;

	/// add by zhou.hu review 2014/4/23 �н�����ͨ��
	memset(&temp, 0, sizeof(multicast_info));
	
    strcpy(temp.m_remote_ip, guava_ip.c_str());
    temp.m_remote_port = guava_port;
    strcpy(temp.m_local_ip, guava_local_ip.c_str());
    temp.m_local_port = guava_local_port;

	m_cffex_info = temp;
}


bool guava_demo::init()
{
	return m_guava.init(m_cffex_info, this);
}

void guava_demo::close()
{
	m_guava.close();
}

void guava_demo::pause()
{
	string str_temp;

	printf("\n�������ַ�����(����q���˳�):\n");
	cin >> str_temp;
	if (str_temp == "q")
	{
		m_quit_flag = true;
	}
}

void guava_demo::on_receive_nomal(guava_udp_normal* data)
{
	string str_body = to_string(data);

	cout << "receive nomal msg: " << str_body << endl;
}

string guava_demo::to_string(guava_udp_normal* ptr)
{
	char buff[8192];

	/// ����ڴ������������޸�   lisen  2016/10/17
	long long	dd_pos_temp = *(long long*)(&ptr->m_total_pos);
	double		dd_pos		= static_cast<double>(dd_pos_temp);

	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%u,%d,%d,%s,%s,%d,%d,%.4f,%d,%.4f,%.4f,%.4f,%d,%.4f,%d"
		,ptr->m_sequence
		,(int)(ptr->m_exchange_id)
		,(int)(ptr->m_channel_id)
		,ptr->m_symbol
		,ptr->m_update_time
		, ptr->m_millisecond
		, (int)(ptr->m_quote_flag)

		,ptr->m_last_px
		,ptr->m_last_share
		,ptr->m_total_value
		//,ptr->m_total_pos
		,dd_pos
		,ptr->m_bid_px
		,ptr->m_bid_share
		,ptr->m_ask_px
		,ptr->m_ask_share
		);

	string str = buff;
	return str;
}




