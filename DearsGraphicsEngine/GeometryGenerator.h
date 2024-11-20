#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>
#include <string>

#include "ModelInfo.h"
struct MeshData
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class GeometryGenerator {
public:

	static MeshData MakeSquare(float scale);
	static MeshData BillboradSquare(float scale);
	static MeshData MakeBox(const float scale = 1.0f);

	static MeshData MakeSphere(const float radius, const int numSlices,
		const int numStacks);

};
