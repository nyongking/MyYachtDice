#pragma once
#include "Timer.h"

class MainApp
{
public:
	MainApp();
	~MainApp();

public:
	bool Init();
	void Loop();

private:
	uint32_t          m_camID = 0;
	GameEngine::Timer m_timer;

#ifdef _DEBUG
	TCHAR m_szBuf[64];
#endif
};

