#pragma once
#include "AnimStructures.h"
struct aiAnimation;

/*
	Load in one animation from file.
	Tested formats: fbx

	Create(aiAnimation): Load an animation from assimpformat
	Create(filename):	 Load an animation from file
	GetMatrix():		 Get the matrix for specific time
*/

class RAnimation : public resource::GResource
{
private:
	double	m_ticksPerFrame;
	double	m_duration;
	bool	m_isLoopable;

	struct KeyFrames
	{
		std::vector<positionKey_t>	position;
		std::vector<scaleKey_t>		scale;
		std::vector<rotationKey_t>	rotation;
	};

	//Each bone has a list of translations
	std::unordered_map<std::string, KeyFrames> m_keyFrames;

private:
	void LoadKeyframes(const aiAnimation* animation);

	//Make public? Needed for blending two animations?
	const sm::Vector3 GetPosition(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const;
	const sm::Vector3 GetScale(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const;
	const sm::Quaternion GetRotation(const std::string& bonename, const double& currentFrame, const double& nextFrame, UINT& lastKey, bool interpolate) const;

public:
	RAnimation();
	~RAnimation();

	//Set and get
	bool IsLoopable() const;
	void SetLoopable(bool& enable);
	const double GetTicksPerFrame() const;

	/*
		Get translations depending on the bone.
		Current frame is the frame we shall get information about.
		Next frame is the next predicted frame.
		Last keys is a way to optimize the search for next pos,scl,rot.
		Can use interpolation for smoother animations. By default it's is on
	*/
	const sm::Matrix GetMatrix(const std::string& bonename, 
								const double& currentFrame, 
								const double& nextFrame, 
								std::array<UINT,3>& lastKeys,
								bool interpolate = true);

	//Create from a assimp-animation if needed
	void Create(const aiAnimation* animation);

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

};