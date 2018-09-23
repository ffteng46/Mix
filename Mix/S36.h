
// S36.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CS36App:
// See S36.cpp for the implementation of this class
//

class CS36App : public CWinApp
{
public:
	CS36App();

// Overrides
public:
	virtual BOOL InitInstance();

public:
	void initctp();
	

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CS36App theApp;