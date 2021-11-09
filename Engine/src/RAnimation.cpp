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

void RAnimation::LoadPositions(const std::string& bonename, aiNodeAnim* channel)
{
	KeyFrames keyframes;
	positionKey_t pos;

	//Keyframe exist - copy values
	auto key = m_keyFrames.find(bonename);
	if (key != m_keyFrames.end())
	{
		keyframes = key->second;
	}

	//Load in all the positions
	keyframes.position.reserve(size_t(channel->mNumPositionKeys));
	for (UINT i = 0; i < channel->mNumPositionKeys; i++)
	{
		pos.time = channel->mPositionKeys[i].mTime;
		const aiVector3D aiPos = channel->mPositionKeys[i].mValue;
		pos.val = { aiPos.x, aiPos.y, aiPos.z };
		keyframes.position.push_back(pos);
	}

	m_keyFrames[bonename] = keyframes;
}

void RAnimation::LoadScales(const std::string& bonename, aiNodeAnim* channel)
{
	KeyFrames keyframes;
	scaleKey_t scl;

	//Keyframe exist - copy values
	auto key = m_keyFrames.find(bonename);
	if (key != m_keyFrames.end())
	{
		keyframes = key->second;
	}

	//Load in all the scales
	keyframes.scale.reserve(size_t(channel->mNumScalingKeys));
	for (UINT i = 0; i < channel->mNumScalingKeys; i++)
	{
		scl.time = channel->mScalingKeys[i].mTime;
		const aiVector3D aiScl = channel->mScalingKeys[i].mValue;
		scl.val = { aiScl.x, aiScl.y, aiScl.z };
		keyframes.scale.push_back(scl);
	}

	m_keyFrames[bonename] = keyframes;
}

void RAnimation::LoadRotations(const std::string& bonename, aiNodeAnim* channel)
{
	KeyFrames keyframes;
	rotationKey_t rot;

	//Keyframe exist - copy values
	auto key = m_keyFrames.find(bonename);
	if (key != m_keyFrames.end())
	{
		keyframes = key->second;
	}

	//Load in all the rotation
	keyframes.rotation.reserve(size_t(channel->mNumRotationKeys));
	for (UINT i = 0; i < channel->mNumRotationKeys; i++)
	{
		rot.time = channel->mRotationKeys[i].mTime;
		const aiQuaternion aiRot = channel->mRotationKeys[i].mValue;
		rot.val = { aiRot.x, aiRot.y, aiRot.z, aiRot.w };
		rot.val.Normalize();
		keyframes.rotation.push_back(rot);
	}

	m_keyFrames[bonename] = keyframes;
}

void RAnimation::LoadKeyframes(const aiAnimation* animation)
{
	std::string assimpKeyword = "_$AssimpFbx$_";

	//Go through all the bones
	for (UINT i = 0; i < animation->mNumChannels; i++)
	{
		std::string assimpName = animation->mChannels[i]->mNodeName.C_Str();
		std::string boneName = assimpName.substr(0, assimpName.find(assimpKeyword));

		//Load in only one part if we get any of the following keywords
		if (assimpName.find("Translation") != std::string::npos)
		{
			LoadPositions(boneName, animation->mChannels[i]);
		}
		else if (assimpName.find("Scaling") != std::string::npos)
		{
			LoadScales(boneName, animation->mChannels[i]);
		}
		else if (assimpName.find("Rotation") != std::string::npos)
		{
			LoadRotations(boneName, animation->mChannels[i]);
		}
		else
		{
			LoadPositions(boneName, animation->mChannels[i]);
			LoadScales(boneName, animation->mChannels[i]);
			LoadRotations(boneName, animation->mChannels[i]);
		}
	}
}

const sm::Vector3 RAnimation::GetPosition(const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nrOfKeys = static_cast<UINT>(m_keyFrames.at(bonename).position.size());
	UINT closestLeft = lastKey;
	UINT closestRight = (closestLeft + 1) % nrOfKeys;

	/*
		Searches through all the keyframes to see which two is closest to current frame.
		Starts at the last key for optimization.
		Usually does the search for 1-3 times instead of search everything from start.
	*/
	bool foundKey = false;
	while (!foundKey)
	{
		if (m_keyFrames.at(bonename).position[closestLeft].time < currentFrame &&
			m_keyFrames.at(bonename).position[closestRight].time > currentFrame)
		{
			foundKey = true;
		}
		else
		{
			closestLeft = closestRight;
			closestRight = (closestRight + 1) % nrOfKeys;
		}
	}

	if (interpolate)
	{
		double firstTime = m_keyFrames.at(bonename).position[closestLeft].time;
		double secondTime = m_keyFrames.at(bonename).position[closestRight].time;
		sm::Vector3 firstVal = m_keyFrames.at(bonename).position[closestLeft].val;
		sm::Vector3 secondVal = m_keyFrames.at(bonename).position[closestRight].val;
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));
		finalVec = sm::Vector3::Lerp(firstVal, secondVal, lerpTime);
	}
	else
	{
		double distance1 = std::abs(currentFrame - m_keyFrames.at(bonename).position[closestLeft].time);
		double distance2 = std::abs(m_keyFrames.at(bonename).position[closestRight].time - currentFrame);

		if (distance1 < distance2)
			finalVec = m_keyFrames.at(bonename).position[closestLeft].val;
		else
			finalVec = m_keyFrames.at(bonename).position[closestRight].val;
	}

	lastKey = closestLeft;
	return finalVec;
}

const sm::Vector3 RAnimation::GetScale(const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nrOfKeys = static_cast<UINT>(m_keyFrames.at(bonename).scale.size());
	UINT closestLeft = lastKey;
	UINT closestRight = (closestLeft + 1) % nrOfKeys;

	/*
		Searches through all the keyframes to see which two is closest to current frame.
		Starts at the last key for optimization.
		Usually does the search for 1-3 times instead of search everything from start.
	*/
	bool foundKey = false;
	while (!foundKey)
	{
		if (m_keyFrames.at(bonename).scale[closestLeft].time < currentFrame &&
			m_keyFrames.at(bonename).scale[closestRight].time > currentFrame)
		{
			foundKey = true;
		}
		else
		{
			closestLeft = closestRight;
			closestRight = (closestRight + 1) % nrOfKeys;
		}
	}

	if (interpolate)
	{
		double firstTime = m_keyFrames.at(bonename).scale[closestLeft].time;
		double secondTime = m_keyFrames.at(bonename).scale[closestRight].time;
		sm::Vector3 firstVal = m_keyFrames.at(bonename).scale[closestLeft].val;
		sm::Vector3 secondVal = m_keyFrames.at(bonename).scale[closestRight].val;
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));
		finalVec = sm::Vector3::Lerp(firstVal, secondVal, lerpTime);
	}
	else
	{
		double distance1 = std::abs(currentFrame - m_keyFrames.at(bonename).scale[closestLeft].time);
		double distance2 = std::abs(m_keyFrames.at(bonename).scale[closestRight].time - currentFrame);

		if (distance1 < distance2)
			finalVec = m_keyFrames.at(bonename).scale[closestLeft].val;
		else
			finalVec = m_keyFrames.at(bonename).scale[closestRight].val;
	}

	lastKey = closestLeft;
	return finalVec;
}

const sm::Quaternion RAnimation::GetRotation(const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const
{
	sm::Quaternion finalQuat;
	UINT nrOfKeys = static_cast<UINT>(m_keyFrames.at(bonename).rotation.size());
	UINT closestLeft = lastKey;
	UINT closestRight = (closestLeft + 1) % nrOfKeys;

	/*
		Searches through all the keyframes to see which two is closest to current frame.
		Starts at the last key for optimization.
		Usually does the search for 1-3 times instead of search everything from start.
	*/
	bool foundKey = false;
	while (!foundKey)
	{
		if (m_keyFrames.at(bonename).rotation[closestLeft].time < currentFrame &&
			m_keyFrames.at(bonename).rotation[closestRight].time > currentFrame)
		{
			foundKey = true;
		}
		else
		{
			closestLeft = closestRight;
			closestRight = (closestRight + 1) % nrOfKeys;
		}
	}

	if (interpolate)
	{
		double firstTime = m_keyFrames.at(bonename).rotation[closestLeft].time;
		double secondTime = m_keyFrames.at(bonename).rotation[closestRight].time;
		sm::Quaternion firstVal = m_keyFrames.at(bonename).rotation[closestLeft].val;
		sm::Quaternion secondVal = m_keyFrames.at(bonename).rotation[closestRight].val;
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));
		finalQuat = sm::Quaternion::Slerp(firstVal, secondVal, lerpTime);
		finalQuat.Normalize();
	}
	else
	{
		double distance1 = std::abs(currentFrame - m_keyFrames.at(bonename).rotation[closestLeft].time);
		double distance2 = std::abs(m_keyFrames.at(bonename).rotation[closestRight].time - currentFrame);

		if (distance1 < distance2)
			finalQuat = m_keyFrames.at(bonename).rotation[closestLeft].val;
		else
			finalQuat = m_keyFrames.at(bonename).rotation[closestRight].val;
	}

	lastKey = closestLeft;
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

const double& RAnimation::GetTicksPerFrame() const
{
	return m_ticksPerFrame;
}

const double& RAnimation::GetDuraction() const
{
	return m_duration;
}

const sm::Matrix RAnimation::GetMatrix(const std::string& bonename, const double& currentFrame, std::array<UINT, 3>& lastKeys, bool interpolate)
{
	sm::Matrix finalMatrix = sm::Matrix::Identity;

	//Bone has to exist otherwise return identity matrix
	if (m_keyFrames.find(bonename) != m_keyFrames.end())
	{
		sm::Vector3 pos		= GetPosition(bonename, currentFrame, lastKeys[0], interpolate);
		sm::Vector3 scl		= GetScale(	  bonename, currentFrame, lastKeys[1], interpolate);
		sm::Quaternion rot	= GetRotation(bonename, currentFrame, lastKeys[2], interpolate);

		//Row major: Scale * Rotation * Translation
		finalMatrix = sm::Matrix::CreateScale(scl) * sm::Matrix::CreateFromQuaternion(rot) * sm::Matrix::CreateTranslation(pos);
	}

	return finalMatrix;
}

void RAnimation::Create(const aiAnimation* animation)
{
	m_duration = animation->mDuration;
	m_ticksPerFrame = animation->mTicksPerSecond;
	
	//Load in all the keyframes
	LoadKeyframes(animation);

}

bool RAnimation::Create(const std::string& filename)
{
	std::string filepath = ANIMATIONPATH + filename;
	Assimp::Importer importer;

	//Will remove extra text on bones like: "_$AssimpFbx$_"...
	importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

    const aiScene* scene = importer.ReadFile(filepath, 
											aiProcess_FlipWindingOrder |
											aiProcess_MakeLeftHanded);

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
	LoadKeyframes(animation);
	
//#ifdef _DEBUG
//	LOG_INFO("Loaded animation: %s\n", filename.c_str());
//#endif // _DEBUG

	importer.FreeScene();
	return true;
}
