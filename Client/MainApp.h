#pragma once
//#include "FrameTimer.h"

class MainApp
{
public:
	MainApp();
	~MainApp();

public:
	bool Init();
	void Loop();

private:
#ifdef _DEBUG
	TCHAR								m_szBuf[64];
#endif
	//FrameTimer							m_frameTimer;
};

