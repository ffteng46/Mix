#include <iostream>
#include "guava_demo.h"



using std::cout;
using std::cin;
using std::endl;


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

		printf("\n按任意字符退出:\n");
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
	string local_ip = "1.1.1.123";
	string str_temp;
	string str_no = "n";

	cout << "本机地址: " << local_ip << endl;
	cout << "是否使用本机地址 (y/n) ";
	cin >> str_temp;
	if (str_no == str_temp)
	{
		cout << "请输入新的本机地址: ";
		cin >> str_temp;
		local_ip = str_temp;
	}

	string cffex_ip = "233.54.1.100";
	int cffex_port = 30800;
	string cffex_local_ip = local_ip;
	int cffex_local_port = 23501;


	cout << "行情通道组播地址: " << cffex_ip << endl;
	cout << "行情通道组播端口: " << cffex_port << endl;
	cout << "行情通道本机地址: " << cffex_local_ip << endl;
	cout << "行情通道本机端口: " << cffex_local_port << endl;
	cout << "是否使用缺省配置 (y/n) ";
	cin >> str_temp;
	if (str_no == str_temp)
	{
		cout << "是否使用缺省行情通道组播地址 " << cffex_ip << " (y/n) ";
		cin >> str_temp;
		if (str_no == str_temp)
		{
			cout << "请输入新的行情通道组播地址: ";
			cin >> str_temp;
			cffex_ip = str_temp;
		}

		cout << "是否使用缺省行情通道组播端口 " << cffex_port << " (y/n) ";
		cin >> str_temp;
		if (str_no == str_temp)
		{
			cout << "请输入新的行情通道组播端口: ";
			cin >> str_temp;
			cffex_port = atoi(str_temp.c_str());
		}

		cout << "是否使用缺省行情通道本机端口 " << cffex_local_port << " (y/n) ";
		cin >> str_temp;
		if (str_no == str_temp)
		{
			cout << "请输入新的行情通道本机端口: ";
			cin >> str_temp;
			cffex_local_port = atoi(str_temp.c_str());
		}
	}


	/// add by zhou.hu review 2014/4/24 设置具体的参数
	multicast_info temp;

	/// add by zhou.hu review 2014/4/23 中金行情通道
	memset(&temp, 0, sizeof(multicast_info));
	
	strcpy(temp.m_remote_ip, cffex_ip.c_str());
	temp.m_remote_port = cffex_port;
	strcpy(temp.m_local_ip, cffex_local_ip.c_str());
	temp.m_local_port = cffex_local_port;

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

	printf("\n按任意字符继续(输入q将退出):\n");
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

	/// 针对于大商所做出的修改   lisen  2016/10/17
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





