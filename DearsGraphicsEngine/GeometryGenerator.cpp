#include "GeometryGenerator.h"

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;

MeshData GeometryGenerator::MakeSquare(float scale) {
	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector3> normals;
	vector<Vector2> texcoords; // �ؽ��� ��ǥ

	// �ո�
	positions.push_back(Vector3(-1.0f, 0.f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 0.f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 0.f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 0.0f, -1.0f) * scale);
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));

	// Texture Coordinates (Direct3D 9)
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	MeshData meshData;

	for (size_t i = 0; i < positions.size(); i++) {
		Vertex v;
		v.mPosition = positions[i];
		v.mNormal = normals[i];
		v.mTexcoords[0] = texcoords[i];

		// v.color = colors[i];

		meshData.vertices.push_back(v);
	}
	meshData.indices = {
		0, 1, 2, 0, 2, 3, // �ո�
	};

	return meshData;
}

MeshData GeometryGenerator::BillboradSquare(float scale)
{
	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector3> normals;
	vector<Vector2> texcoords; // �ؽ��� ��ǥ

	// �ո�
	positions.push_back(Vector3(-1.0f, 1.0f, 0.f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 0.f) * scale);
	positions.push_back(Vector3(1.0f, -1.f, 0.f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, 0.0f) * scale);
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));

	// Texture Coordinates (Direct3D 9)
	// https://learn.microsoft.com/en-us/windows/win32/direct3d9/texture-coordinates
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	MeshData meshData;

	for (size_t i = 0; i < positions.size(); i++) {
		Vertex v;
		v.mPosition = positions[i];
		v.mNormal = normals[i];
		v.mTexcoords[0] = texcoords[i];

		// v.color = colors[i];

		meshData.vertices.push_back(v);
	}
	meshData.indices = {
		0, 1, 2, 0, 2, 3, // �ո�
	};

	return meshData;
}

MeshData GeometryGenerator::MakeBox(const float scale) {

	vector<Vector3> positions;
	vector<Vector3> colors;
	vector<Vector3> normals;
	vector<Vector2> texcoords; // �ؽ��� ��ǥ

	// ����
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// �Ʒ���
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	normals.push_back(Vector3(0.0f, -1.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// �ո�
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, -1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// �޸�
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	colors.push_back(Vector3(0.0f, 1.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(0.0f, 0.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ����
	positions.push_back(Vector3(-1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, 1.0f) * scale);
	positions.push_back(Vector3(-1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(-1.0f, -1.0f, -1.0f) * scale);
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	colors.push_back(Vector3(1.0f, 1.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(-1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	// ������
	positions.push_back(Vector3(1.0f, -1.0f, 1.0f) * scale);
	positions.push_back(Vector3(1.0f, -1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, -1.0f) * scale);
	positions.push_back(Vector3(1.0f, 1.0f, 1.0f) * scale);
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	colors.push_back(Vector3(1.0f, 0.0f, 1.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	normals.push_back(Vector3(1.0f, 0.0f, 0.0f));
	texcoords.push_back(Vector2(0.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 0.0f));
	texcoords.push_back(Vector2(1.0f, 1.0f));
	texcoords.push_back(Vector2(0.0f, 1.0f));

	MeshData meshData;
	for (size_t i = 0; i < positions.size(); i++) {
		Vertex v;
		v.mPosition = positions[i];
		v.mNormal = normals[i];
		v.mTexcoords[0] = texcoords[i];
		meshData.vertices.push_back(v);
	}

	meshData.indices = {
		0,  1,  2,  0,  2,  3,  // ����
		4,  5,  6,  4,  6,  7,  // �Ʒ���
		8,  9,  10, 8,  10, 11, // �ո�
		12, 13, 14, 12, 14, 15, // �޸�
		16, 17, 18, 16, 18, 19, // ����
		20, 21, 22, 20, 22, 23  // ������
	};

	return meshData;
}

MeshData GeometryGenerator::MakeSphere(const float radius, const int numSlices,
	const int numStacks) {

	// ����: OpenGL Sphere
	// http://www.songho.ca/opengl/gl_sphere.html
	// Texture ��ǥ�趧���� (numSlices + 1) ���� ���ؽ� ��� (�������� �ݾ��ִ�
	// ���ؽ��� �ߺ�) Stack�� y ���� �������� �׾ư��� ���

	const float dTheta = -XM_2PI / float(numSlices);
	const float dPhi = -XM_PI / float(numStacks);

	MeshData meshData;

	vector<Vertex>& vertices = meshData.vertices;

	for (int j = 0; j <= numStacks; j++) {

		// ���ÿ� ���� ���� �������� x-y ��鿡�� ȸ�� ���Ѽ� ���� �ø��� ����
		Vector3 stackStartPoint = Vector3::Transform(
			Vector3(0.0f, -radius, 0.0f), Matrix::CreateRotationZ(dPhi * j));

		for (int i = 0; i <= numSlices; i++) {
			Vertex v;

			// �������� x-z ��鿡�� ȸ����Ű�鼭 ���� ����� ����
			v.mPosition = Vector3::Transform(
				stackStartPoint, Matrix::CreateRotationY(dTheta * float(i)));

			v.mNormal = v.mPosition; // ������ ���� �߽�
			v.mNormal.Normalize();
			v.mTexcoords[0] =
				Vector2(float(i) / numSlices, 1.0f - float(j) / numStacks);

			vertices.push_back(v);
		}
	}

	// cout << vertices.size() << endl;

	vector<unsigned int>& indices = meshData.indices;

	for (int j = 0; j < numStacks; j++) {

		const int offset = (numSlices + 1) * j;

		for (int i = 0; i < numSlices; i++) {

			indices.push_back(offset + i);
			indices.push_back(offset + i + numSlices + 1);
			indices.push_back(offset + i + 1 + numSlices + 1);

			indices.push_back(offset + i);
			indices.push_back(offset + i + 1 + numSlices + 1);
			indices.push_back(offset + i + 1);
		}
	}

	// cout << indices.size() << endl;
	// for (int i = 0; i < indices.size(); i++) {
	//     cout << indices[i] << " ";
	// }
	// cout << endl;

	return meshData;
}