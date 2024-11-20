#pragma once
#include "ModelInfo.h"

NodeAnimation::NodeAnimation() : mNumPosKeys(0), mNumRotKeys(0), mNumScaKeys(0),
mPosKey(nullptr), mRotKey(nullptr), mScaKey(nullptr)/*, mTargetNode(nullptr)*/
{
}

#include<iostream>
NodeAnimation::~NodeAnimation()
{
	static int i = 0;
	if (mPosKey)
	{
		delete[] mPosKey;
		mPosKey = nullptr;
	}
	if (mRotKey)
	{
		delete[] mRotKey;
		mRotKey = nullptr;
	}
	if (mScaKey)
	{
		delete[] mScaKey;
		mScaKey = nullptr;
	}
}

Animation::Animation() : mName{}, mDuration{ 0 }, mTicksPerSecond{ 0 }, mNumChannels{ 0 }, mChannels{ nullptr }
{
}

Animation::~Animation()
{
	if (mChannels != nullptr)
	{
		delete[] mChannels;
	}
}

Node::Node() : mNodeName{}, mTransformation{}, mWorldTransformation{}, mNumNextNode{ 0 }, mNextNode{ nullptr }
{
}

Node::~Node()
{
	if (mNextNode)
	{
		delete[] mNextNode;
	}
}

Bone::Bone() : mBoneName{}, mTargetNode{ nullptr }, mOffsetMatrix{}, mNumWeight{ 0 }, mpVertexWeight{ nullptr }
{
}

Bone::~Bone()
{
	//delete와 delete[]는 nullptr에 대해 안전하게 동작되므로 굳이 if문을 쓸 필요는 없다.
	delete[] mpVertexWeight;
}

Mesh::Mesh() : mMeshName{}, mNumVertices{ 0 }, mVertices{ nullptr }, mNumIndices{ 0 }, mIndices{ nullptr },
/*mTextureName{ nullptr },*/ mNumBones{ 0 }, mBone{ nullptr }
{
}

Mesh::~Mesh()
{
	if (mVertices)
	{
		delete[] mVertices;
	}
	if (mIndices)
	{
		delete[] mIndices;
	}
	if (mBone)
	{
		delete[] mBone;
	}
}

Model::Model() : mRootNode{ nullptr }, mNumMesh{ 0 }, mMeshData{ nullptr }//, mNumAnimation{ 0 }, mAnimation{ nullptr }
{
}

Model::~Model()
{
	if (mRootNode)
	{
		delete mRootNode;
	}

	if (mMeshData)
	{
		delete[] mMeshData;
	}
}

Vertex::Vertex()
{

}

Vertex::~Vertex()
{

}
