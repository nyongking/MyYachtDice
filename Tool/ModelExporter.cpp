#include "ClientPch.h"
#include "ModelExporter.h"

#include "Model.h"       // MeshFileHeader, MeshSectionHeader, VTXPOSNORMTANUV
#include "BufferStruct.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <filesystem>

static std::string ExtractStem(const std::string& path)
{
	return std::filesystem::path(path).stem().string();
}

static bool ProcessMesh(const aiMesh* mesh,
                        std::vector<Render::VTXPOSNORMTANUV>& outVertices,
                        std::vector<uint32_t>&                outIndices)
{
	outVertices.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		Render::VTXPOSNORMTANUV v = {};

		v.position = { mesh->mVertices[i].x,
		               mesh->mVertices[i].y,
		               mesh->mVertices[i].z };

		if (mesh->mNormals)
			v.normal = { mesh->mNormals[i].x,
			             mesh->mNormals[i].y,
			             mesh->mNormals[i].z };

		if (mesh->mTangents)
			v.tangent = { mesh->mTangents[i].x,
			              mesh->mTangents[i].y,
			              mesh->mTangents[i].z };

		if (mesh->mTextureCoords[0])
			v.texcoord = { mesh->mTextureCoords[0][i].x,
			               mesh->mTextureCoords[0][i].y };

		outVertices.push_back(v);
	}

	outIndices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);

	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		if (face.mNumIndices != 3)
			continue;  // Triangulate 플래그로 보장되지만 방어적으로 처리

		outIndices.push_back(face.mIndices[0]);
		outIndices.push_back(face.mIndices[1]);
		outIndices.push_back(face.mIndices[2]);
	}

	return !outVertices.empty() && !outIndices.empty();
}

bool ModelExporter::Export(const std::string& srcPath,
                           const std::string& dstPath,
                           std::string&       outError,
                           const ExportOptions& opts)
{
	// Assimp 로드
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(srcPath,
		aiProcess_Triangulate          |
		aiProcess_GenNormals           |
		aiProcess_CalcTangentSpace     |
		aiProcess_JoinIdenticalVertices|
		aiProcess_ConvertToLeftHanded);

	if (!scene || !scene->mRootNode || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE))
	{
		outError = importer.GetErrorString();
		return false;
	}

	if (scene->mNumMeshes == 0)
	{
		outError = "No meshes found in file";
		return false;
	}

	// matKey 접두사 결정
	std::string prefix = opts.matKeyPrefix.empty()
	                   ? ExtractStem(srcPath)
	                   : opts.matKeyPrefix;

	// 출력 파일 열기
	std::ofstream file(dstPath, std::ios::binary);
	if (!file)
	{
		outError = "Cannot open output file: " + dstPath;
		return false;
	}

	// 파일 헤더 작성
	GameEngine::MeshFileHeader fileHeader = {};
	memcpy(fileHeader.magic, "MYMESH\0\0", 8);
	fileHeader.version      = 1;
	fileHeader.sectionCount = scene->mNumMeshes;
	file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));

	// 섹션별 처리
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		std::vector<Render::VTXPOSNORMTANUV> vertices;
		std::vector<uint32_t>               indices;

		if (!ProcessMesh(scene->mMeshes[i], vertices, indices))
		{
			outError = "Failed to process mesh index " + std::to_string(i);
			return false;
		}

		// AABB 계산
		float minB[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
		float maxB[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
		for (const auto& v : vertices)
		{
			minB[0] = std::min(minB[0], v.position.x);
			minB[1] = std::min(minB[1], v.position.y);
			minB[2] = std::min(minB[2], v.position.z);
			maxB[0] = std::max(maxB[0], v.position.x);
			maxB[1] = std::max(maxB[1], v.position.y);
			maxB[2] = std::max(maxB[2], v.position.z);
		}

		// 섹션 헤더 작성
		GameEngine::MeshSectionHeader secHeader = {};
		secHeader.vertexCount = static_cast<uint32_t>(vertices.size());
		secHeader.indexCount  = static_cast<uint32_t>(indices.size());
		memcpy(secHeader.minBound, minB, sizeof(minB));
		memcpy(secHeader.maxBound, maxB, sizeof(maxB));

		std::string matKey = prefix + "_" + std::to_string(i);
		strncpy_s(secHeader.matKey, matKey.c_str(), sizeof(secHeader.matKey) - 1);

		file.write(reinterpret_cast<const char*>(&secHeader), sizeof(secHeader));

		// 정점/인덱스 데이터 작성
		file.write(reinterpret_cast<const char*>(vertices.data()),
		           sizeof(Render::VTXPOSNORMTANUV) * vertices.size());
		file.write(reinterpret_cast<const char*>(indices.data()),
		           sizeof(uint32_t) * indices.size());
	}

	return true;
}
