#pragma once
#include<directxTK/SimpleMath.h>
#include <string>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Quaternion;

#define MAX_BONES_INDICE 8
#define MAX_NUM_TEXCOORD 1
class Vertex
{
public:
	Vertex();
	Vertex(Vector3 a, Vector3 b, Vector2 c)
	{
		mPosition = a;
		mNormal = b;
		mTexcoords[0] = c;
	};
	~Vertex();
public:
	Vector3 mPosition;
	Vector3 mNormal;
	Vector2 mTexcoords[MAX_NUM_TEXCOORD];
	UINT mBoneIndex[MAX_BONES_INDICE] = { UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX };
	float mBoneWeights[MAX_BONES_INDICE] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
};

//메쉬들은 모여 노드를 이룬다.
class Node
{
public:
	Node();
	~Node();
public:
	std::string mNodeName;			//노드 이름
	Matrix mTransformation;			//노드의 변환 행렬 / SimpleMath는 오른손 좌표계를 쓴다.
	Matrix mWorldTransformation;	//월드좌표

	unsigned int mNumNextNode;
	Node* mNextNode;
	//이 노드의 메쉬 수
	unsigned int mNumMeshes;

	//이 노드의 메쉬, 각 항목은 Model의 메쉬 목록에 대한 인덱스
	unsigned int* mMeshes;
};

struct VertexWeight
{
	unsigned int mVertexId = UINT_MAX;
	float mWeight = 0.0f;
};

///본은 주로 스키닝 애니메이션을 구현할때 사용된다.
class Bone
{
public:
	Bone();
	~Bone();

public:
	std::string mBoneName;		//본 이름
	Node* mTargetNode;			//본 이름과 노드의 이름이 같을 경우 타겟 노드로 지정해준다. 그래야 노드의tranform을 가지고 올수 있기 때문
	unsigned int mNumWeight = 0;
	VertexWeight* mpVertexWeight;
	Matrix mOffsetMatrix;		//메쉬와 본사이의 오프셋 행렬 -> 본의 공간에서 메쉬의 정점들을 본의 로컬 공간으로 변환하는데 사용
};

class AABB
{
public:
	Vector3 mMin = { FLT_MAX, FLT_MAX, FLT_MAX };
	Vector3 mMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
};

class Mesh
{
public:
	Mesh();
	~Mesh();
public:
	std::string mMeshName;				// 메쉬의 이름

	unsigned int  mNumVertices;			// 버텍스의 수
	Vertex* mVertices = nullptr;					// Vertex배열

	unsigned int mNumIndices;			// 인덱스의 수
	unsigned int* mIndices = nullptr;				// Index 배열

	AABB mAABB;

	unsigned int mNumBones;
	Bone* mBone;						//메쉬에 해당하는 본
};


struct VecKey
{
	double mTime;
	Vector3 mValue;
};

struct QuatKey
{
	double mTime;
	Quaternion mValue;
};

class NodeAnimation
{
public:
	NodeAnimation();
	~NodeAnimation();
public:
	unsigned int mNumPosKeys;
	VecKey* mPosKey;			//Positon Key

	unsigned int mNumRotKeys;
	QuatKey* mRotKey;			//Rotation Key

	unsigned int mNumScaKeys;
	VecKey* mScaKey;			//Scale Key

	std::string mName;			//이 애니메이션의 영향을 받는 노드의 이름
};

class Animation
{
public:
	Animation();
	~Animation();
public:
	std::string mName;						//애니메이션의 이름
	double mDuration;						//애니메이션의 지속시간(틱)
	double mTicksPerSecond;					//초당 틱, 가져온 파일에 지정되지 않은 경우 -> 0

	unsigned int mNumChannels;				//본 애니메이션 채널 수
	NodeAnimation* mChannels;				//노드 애니메이션의 채널, 객 채널의 단일 노드에 영향을 준다. 크기는 mNumChannels;

	///나중에 메시애니메이션을 추가하려면 여기다 추가하면된다.
};

//노드들은 모여 모델을 이룬다. 근데 하나의 노드로만 구성된 모델이 있을 수도 있음
class Model
{
public:
	Model();
	~Model();
public:
	Node* mRootNode;

	unsigned int mNumMesh;
	Mesh* mMeshData;

	//	std::string mBasePath;

	// 	unsigned int mNumAnimation;
	// 	Animation* mAnimation;
};
