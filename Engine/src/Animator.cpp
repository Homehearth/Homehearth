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
	m_animations.clear();
}

void Animator::SetBones(const std::vector<bone_t>& bones)
{
	m_bones = bones;
	m_bones.shrink_to_fit();
	m_finalMatrix.reserve(m_bones.size());
}

void Animator::Update()
{
	if (!m_bones.empty())
	{
		//double tickDT = m_animations[m_currentAnim]->GetTicksPerFrame() * 
		//m_frameTime +=  m_animations[m_currentAnim]->GetTicksPerFrame();
		//double nextFrameTime = 

		std::vector<sm::Matrix> modelMatrices;

		for (size_t i = 0; i < m_bones.size(); i++)
		{
			sm::Matrix localMatrix; //= m_animations[m_currentAnim]->GetMatrix(m_bones[i].name, );

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

		//Update the buffer for the GPU
	}
	
}
