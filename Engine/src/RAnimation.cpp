#include "EnginePCH.h"
#include "RAnimation.h"

RAnimation::RAnimation()
{
	m_ticksPerFrame = 30;
	m_duration		= 0;
	m_isLoopable	= true;
}

RAnimation::~RAnimation()
{
	for (auto& translation : m_keyFrames)
	{
		translation.second.position.clear();
		translation.second.scale.clear();
		translation.second.rotation.clear();
	}
	m_keyFrames.clear();
}

void RAnimation::LoadKeyframes(const aiAnimation* animation)
{
	//Go through all the bones
	for (UINT i = 0; i < animation->mNumChannels; i++)
	{
		const std::string boneName = animation->mChannels[i]->mNodeName.C_Str();
		KeyFrames		keyframes;
		positionKey_t	pos;
		scaleKey_t		scl;
		rotationKey_t	rot;

		//Load in all the positions
		keyframes.position.reserve(size_t(animation->mChannels[i]->mNumPositionKeys));
		for (UINT p = 0; p < animation->mChannels[i]->mNumPositionKeys; p++)
		{
			pos.time = animation->mChannels[i]->mPositionKeys[p].mTime;
			const aiVector3D aiPos = animation->mChannels[i]->mPositionKeys[p].mValue;
			pos.val = { aiPos.x, aiPos.y, aiPos.z };
			keyframes.position.push_back(pos);
		}

		//Load in all the scales
		keyframes.scale.reserve(size_t(animation->mChannels[i]->mNumScalingKeys));
		for (UINT s = 0; s < animation->mChannels[i]->mNumScalingKeys; s++)
		{
			scl.time = animation->mChannels[i]->mScalingKeys[s].mTime;
			const aiVector3D aiScl = animation->mChannels[i]->mScalingKeys[s].mValue;
			scl.val = { aiScl.x, aiScl.y, aiScl.z };
			keyframes.scale.push_back(scl);
		}

		//Load in all the rotations
		keyframes.rotation.reserve(size_t(animation->mChannels[i]->mNumRotationKeys));
		for (UINT r = 0; r < animation->mChannels[i]->mNumRotationKeys; r++)
		{
			rot.time = animation->mChannels[i]->mRotationKeys[r].mTime;
			const aiQuaternion aiRot = animation->mChannels[i]->mRotationKeys[r].mValue;
			rot.val = { aiRot.x, aiRot.y, aiRot.z, aiRot.w };
			keyframes.rotation.push_back(rot);
		}

		m_keyFrames[boneName] = keyframes;
	}
}

const sm::Vector3 RAnimation::GetPosition(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_keyFrames.at(bonename).position.size())
		nextKey = 0;

	const double secondTime = m_keyFrames.at(bonename).position[nextKey].time;

	if (interpolate)
	{
		const double firstTime = m_keyFrames.at(bonename).position[lastKey].time;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Vector3 firstPos = m_keyFrames.at(bonename).position[lastKey].val;
		const sm::Vector3 secondPos = m_keyFrames.at(bonename).position[nextKey].val;

		finalVec = sm::Vector3::Lerp(firstPos, secondPos, lerpTime);
	}
	else
	{
		finalVec = m_keyFrames.at(bonename).position[lastKey].val;
	}

	//Update the lastkey to new value
	/*if (nextFrame >= secondTime)
		lastKey = nextKey;*/

	return finalVec;
}

const sm::Vector3 RAnimation::GetScale(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_keyFrames.at(bonename).scale.size())
		nextKey = 0;

	const double secondTime = m_keyFrames.at(bonename).scale[nextKey].time;

	if (interpolate)
	{
		const double firstTime = m_keyFrames.at(bonename).scale[lastKey].time;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Vector3 firstScl = m_keyFrames.at(bonename).scale[lastKey].val;
		const sm::Vector3 secondScl = m_keyFrames.at(bonename).scale[nextKey].val;

		finalVec = sm::Vector3::Lerp(firstScl, secondScl, lerpTime);
	}
	else
	{
		finalVec = m_keyFrames.at(bonename).scale[lastKey].val;
	}

	//Update the lastkey to new value
	/*if (nextFrame >= secondTime)
		lastKey = nextKey;*/

	return finalVec;
}

const sm::Quaternion RAnimation::GetRotation(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Quaternion finalQuat;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_keyFrames.at(bonename).rotation.size())
		nextKey = 0;

	const double secondTime = m_keyFrames.at(bonename).rotation[nextKey].time;

	if (interpolate)
	{
		const double firstTime = m_keyFrames.at(bonename).rotation[lastKey].time;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Quaternion firstQuat = m_keyFrames.at(bonename).rotation[lastKey].val;
		const sm::Quaternion secondQuat = m_keyFrames.at(bonename).rotation[nextKey].val;

		finalQuat = sm::Quaternion::Slerp(firstQuat, secondQuat, lerpTime);
		finalQuat.Normalize();
	}
	else
	{
		finalQuat = m_keyFrames.at(bonename).rotation[lastKey].val;
	}

	//Update the lastkey to new value
	/*if (nextFrame >= secondTime)
		lastKey = nextKey;*/

	return finalQuat;
}

bool RAnimation::IsLoopable() const
{
	return m_isLoopable;
}

void RAnimation::SetLoopable(bool& enable)
{
	m_isLoopable = enable;
}

const double RAnimation::GetTicksPerFrame() const
{
	return m_ticksPerFrame;
}

const sm::Matrix RAnimation::GetMatrix(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT* lastKeys, bool interpolate)
{
	sm::Matrix finalMatrix = sm::Matrix::Identity;

	//Bone has to exist otherwise return identity matrix
	if (m_keyFrames.find(bonename) != m_keyFrames.end())
	{
		sm::Vector3 pos = GetPosition(bonename, currentFrame, nextFrame, lastKeys[0], interpolate);
		sm::Vector3 scl = GetScale(bonename, currentFrame, nextFrame, lastKeys[1], interpolate);
		sm::Quaternion rot = GetRotation(bonename, currentFrame, nextFrame, lastKeys[2], interpolate);
		finalMatrix = sm::Matrix::CreateScale(scl) * sm::Matrix::CreateFromQuaternion(rot) * sm::Matrix::CreateTranslation(pos);
	}

	return finalMatrix.Transpose();
}

void RAnimation::Create(const aiAnimation* animation)
{
	m_duration = animation->mDuration;
	m_ticksPerFrame = animation->mTicksPerSecond;
	
	//Load in all the keyframes
	LoadKeyframes(animation);

#ifdef _DEBUG
		LOG_INFO("Loaded animation: %s\n", animation->mName.C_Str());
#endif // _DEBUG
}

bool RAnimation::Create(const std::string& filename)
{
	std::string filepath = ANIMATIONPATH + filename;
	Assimp::Importer importer;

	//Will remove extra text on bones like: "_$AssimpFbx$_"...
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(filepath, 0);

    //Check if readfile was successful
    if (!scene || !scene->mRootNode)
	{
#ifdef _DEBUG
        LOG_WARNING("Assimp error: %s", importer.GetErrorString());
#endif 
        importer.FreeScene();
        return false;
    }

	if (!scene->HasAnimations())
	{
#ifdef _DEBUG
		std::cout << "The file " << filename << " does not have any animations..." << std::endl;
#endif
		importer.FreeScene();
		return false;
	}

	//Only supports to load in the first animation
	const aiAnimation* animation = scene->mAnimations[0];

	m_duration = animation->mDuration;
	m_ticksPerFrame = animation->mTicksPerSecond;
	
	//Load in all the keyframes - works with only one animation at time
	//Takes the first
	LoadKeyframes(animation);
	
#ifdef _DEBUG
	LOG_INFO("Loaded animation: %s\n", filename.c_str());
#endif // _DEBUG

	importer.FreeScene();
	return true;
}
