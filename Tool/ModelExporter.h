#pragma once
#include <string>

class ModelExporter
{
public:
	struct ExportOptions
	{
		std::string matKeyPrefix;  // 비어있으면 소스 파일명에서 자동 추출
	};

	// srcPath: .obj/.fbx 등 Assimp가 지원하는 포맷
	// dstPath: .mymesh 출력 경로
	// 반환: 성공 여부, outError에 실패 원인 기록
	bool Export(const std::string& srcPath,
	            const std::string& dstPath,
	            std::string&       outError,
	            const ExportOptions& opts = {});
};
