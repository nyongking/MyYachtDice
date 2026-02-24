#pragma once
#include "ConcurrentJobQueue.h"
// 동기 방식 & 유사 비동기 방식을 둘 다 이용할 수 있는 ResourceLoader

// deferred == true -> 유사 비동기
//
// deferred == false -> 동기 방식

class ResourceLoader : public ConcurrentJobQueue
{
public:
	ResourceLoader(uint32 asyncCount = 5);

public:
	bool LoadVertexShader(bool deferred, const std::wstring& path, const std::string& entry = "main", const std::string& profile = "vs_5_0");

private:

};

