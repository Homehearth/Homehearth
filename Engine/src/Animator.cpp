#include "EnginePCH.h"
#include "Animator.h"

Animator::Animator()
{
	m_frameTime		= 0;
	m_currentAnim	= "";
	m_nextAnim		= "";
}

Animator::~Animator()
{
	m_bones.clear();
	m_finalMatrix.clear();
	m_animations.clear();
}

bool Animator::LoadModel(const std::string& filename)
{
	bool loaded = false;
	m_model = ResourceManager::Get().GetResource<RModel>(filename);

	if (m_model)
	{
		std::vector<bone_t> allBones = m_model->GetSkeleton();
		for (size_t i = 0; i < allBones.size(); i++)
		{
			bone_keyFrames_t bone;
			bone.name = allBones[i].name;
			bone.inverseBind = allBones[i].inverseBind;
			bone.parentIndex = allBones[i].parentIndex;
			m_bones.push_back(bone);
		}
		allBones.clear();

		m_finalMatrix.resize(m_bones.size(), sm::Matrix::Identity);
		loaded = true;
	}
	return loaded;
}

bool Animator::Create(const std::string& filename)
{
	/*
		Testing with hardcoded values for now...
	*/

	m_currentAnim = "Player_Idle.fbx";
	if (!LoadModel("Player_Skeleton.fbx"))
		return false;

	m_animations[m_currentAnim] = ResourceManager::Get().GetResource<RAnimation>(m_currentAnim);


	return true;
}

void Animator::Render()
{
	//Unnecessary to the render if the model does not exist
	if (m_model)
	{
		/*
			Update the bones for the model
		*/

		if (!m_bones.empty())
		{
			//double tickDT = m_animations[m_currentAnim]->GetTicksPerFrame() * 
			//m_frameTime +=  m_animations[m_currentAnim]->GetTicksPerFrame();
			//double nextFrameTime = 

			std::vector<sm::Matrix> modelMatrices;
			modelMatrices.resize(m_bones.size(), sm::Matrix::Identity);

			for (size_t i = 0; i < m_bones.size(); i++)
			{
				sm::Matrix localMatrix = m_animations[m_currentAnim]->GetMatrix(m_bones[i].name, 0.1, 0.2, m_bones[i].lastKeys);

				if (m_bones[i].parentIndex == -1)
				{
					modelMatrices[i] = localMatrix;
				}
				else
				{
					modelMatrices[i] = modelMatrices[m_bones[i].parentIndex] * localMatrix;
				}

				m_finalMatrix[i] = modelMatrices[i] * m_bones[i].inverseBind;
			}

			modelMatrices.clear();

			//Update the buffer for the GPU
		}

		/*
			Finally render the model at the current pose
		*/
		m_model->Render();
	}
}
