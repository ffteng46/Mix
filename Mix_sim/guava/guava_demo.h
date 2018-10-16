/*!****************************************************************************
 @note   Copyright (coffee), 2005-2014, Shengli Tech. Co., Ltd.
 @file   guava_demo.h
 @date   2014/8/27   12:44
 @author zhou.hu
 
 @brief   本文件是EFH期货版行情组播接口的示例程序.
 
 注意: 本示例仅是提供一种接收我公司EFH组播行情的一种通用方法，如果客户有其它
 更高效的接收EFH组播行情的方式推荐还是采用自己更高效的接收我公司EFH的数据。

 @note 
******************************************************************************/
#pragma once
#include <vector>
#include "guava_quote.h"

using std::vector;


class guava_demo :public guava_quote_event
{
public:
	guava_demo(void);
	~guava_demo(void);

	/// \brief 示例函数入口函数
	void run();

private:
	virtual void on_receive_nomal(guava_udp_normal* data);

	string to_string(guava_udp_normal* ptr);


private:
	/// \brief 初始化参数调整方法
	void input_param();

	/// \brief 初始化
	bool init();

	/// \brief 关闭
	void close();

	/// \brief 暂停
	void pause();



private:
	multicast_info			m_cffex_info;		///< 中金接UDP信息
	guava_quote				m_guava;			///< 行情接收对象
	bool					m_quit_flag;		///< 退出标志
};

