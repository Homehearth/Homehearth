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
	for (auto& translation : m_translations)
	{
		translation.second.position.clear();
		translation.second.scale.clear();
		translation.second.rotation.clear();
	}
	m_translations.clear();
}

const sm::Vector3 RAnimation::GetPosition(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_translations.at(bonename).position.size())
		nextKey = 0;

	const double secondTime = m_translations.at(bonename).position[nextKey].first;

	if (interpolate)
	{
		const double firstTime = m_translations.at(bonename).position[lastKey].first;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Vector3 firstPos = m_translations.at(bonename).position[lastKey].second;
		const sm::Vector3 secondPos = m_translations.at(bonename).position[nextKey].second;

		finalVec = sm::Vector3::Lerp(firstPos, secondPos, lerpTime);
	}
	else
	{
		finalVec = m_translations.at(bonename).position[lastKey].second;
	}

	//Update the lastkey to new value
	if (nextFrame >= secondTime)
		lastKey = nextKey;

	return finalVec;
}

const sm::Vector3 RAnimation::GetScale(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Vector3 finalVec;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_translations.at(bonename).scale.size())
		nextKey = 0;

	const double secondTime = m_translations.at(bonename).scale[nextKey].first;

	if (interpolate)
	{
		const double firstTime = m_translations.at(bonename).scale[lastKey].first;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Vector3 firstScl = m_translations.at(bonename).scale[lastKey].second;
		const sm::Vector3 secondScl = m_translations.at(bonename).scale[nextKey].second;

		finalVec = sm::Vector3::Lerp(firstScl, secondScl, lerpTime);
	}
	else
	{
		finalVec = m_translations.at(bonename).scale[lastKey].second;
	}

	//Update the lastkey to new value
	if (nextFrame >= secondTime)
		lastKey = nextKey;

	return finalVec;
}

const sm::Quaternion RAnimation::GetRotation(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const
{
	sm::Quaternion finalQuat;
	UINT nextKey = lastKey + 1;
	//Next key should not go out of range
	if (nextKey >= m_translations.at(bonename).rotation.size())
		nextKey = 0;

	const double secondTime = m_translations.at(bonename).rotation[nextKey].first;

	if (interpolate)
	{
		const double firstTime = m_translations.at(bonename).rotation[lastKey].first;

		//Gets a value between 0.0f to 1.0f of how much to interpolate
		float lerpTime = float((currentFrame - firstTime) / (secondTime - firstTime));

		const sm::Quaternion firstQuat = m_translations.at(bonename).rotation[lastKey].second;
		const sm::Quaternion secondQuat = m_translations.at(bonename).rotation[nextKey].second;

		finalQuat = sm::Quaternion::Slerp(firstQuat, secondQuat, lerpTime);
		finalQuat.Normalize();
	}
	else
	{
		finalQuat = m_translations.at(bonename).rotation[lastKey].second;
	}

	//Update the lastkey to new value
	if (nextFrame >= secondTime)
		lastKey = nextKey;

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

const sm::Matrix RAnimation::GetMatrix(const std::string& bonename, const double& currentFrame, const double& nextFrame, std::array<UINT, 3>& lastKeys, bool interpolate)
{
	sm::Matrix finalMatrix = sm::Matrix::Identity;

	//Bone has to exist otherwise return identity matrix
	if (m_translations.find(bonename) != m_translations.end())
	{
		sm::Vector3 pos = GetPosition(bonename, currentFrame, nextFrame, lastKeys[0], interpolate);
		sm::Vector3 scl = GetScale(bonename, currentFrame, nextFrame, lastKeys[1], interpolate);
		sm::Quaternion rot = GetRotation(bonename, currentFrame, nextFrame, lastKeys[2], interpolate);
		finalMatrix = sm::Matrix::CreateScale(scl) * sm::Matrix::CreateFromQuaternion(rot) * sm::Matrix::CreateTranslation(pos);	
	}

	return finalMatrix;
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
	
	//Go through all the bones
	for (UINT i = 0; i < animation->mNumChannels; i++)
	{
		const std::string boneName = animation->mChannels[i]->mNodeName.C_Str();
		Translations						translations;
		std::pair<double, sm::Vector3>		pair;
		std::pair<double, sm::Quaternion>	pairQ;

		//Load in all the positions
		translations.position.reserve(size_t(animation->mChannels[i]->mNumPositionKeys));
		for (UINT p = 0; p < animation->mChannels[i]->mNumPositionKeys; p++)
		{
			pair.first = animation->mChannels[i]->mPositionKeys[p].mTime;
			const aiVector3D aiPos = animation->mChannels[i]->mPositionKeys[p].mValue;
			pair.second = { aiPos.x, aiPos.y, aiPos.z };
			translations.position.push_back(pair);
		}

		//Load in all the scales
		translations.scale.reserve(size_t(animation->mChannels[i]->mNumScalingKeys));
		for (UINT s = 0; s < animation->mChannels[i]->mNumScalingKeys; s++)
		{
			pair.first = animation->mChannels[i]->mScalingKeys[s].mTime;
			const aiVector3D aiScl = animation->mChannels[i]->mScalingKeys[s].mValue;
			pair.second = { aiScl.x, aiScl.y, aiScl.z };
			translations.scale.push_back(pair);
		}

		//Load in all the rotations
		translations.rotation.reserve(size_t(animation->mChannels[i]->mNumRotationKeys));
		for (UINT r = 0; r < animation->mChannels[i]->mNumRotationKeys; r++)
		{
			pairQ.first = animation->mChannels[i]->mRotationKeys[r].mTime;
			const aiQuaternion aiRot = animation->mChannels[i]->mRotationKeys[r].mValue;
			pairQ.second = { aiRot.x, aiRot.y, aiRot.z, aiRot.w };
			translations.rotation.push_back(pairQ);
		}

		m_translations[boneName] = translations;
	}
	
#ifdef _DEBUG
	LOG_INFO("Loaded animation: %s\n", filename.c_str());
#endif // _DEBUG

	importer.FreeScene();
	return true;
}
