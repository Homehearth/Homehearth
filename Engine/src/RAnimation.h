#pragma once
struct aiAnimation;
struct aiNodeAnim;

/*
	Load in one animation from file.
	Tested formats: fbx

	Create(aiAnimation): Load an animation from assimpformat
	Create(filename):	 Load an animation from file
*/

class RAnimation : public resource::GResource
{
private:
	double	m_ticksPerFrame;
	double	m_duration;
	bool	m_isLoopable;

	/*
		Keyframes structures
	*/
	struct positionKey_t
	{
		double		time;
		sm::Vector3 val;
	};
	struct scaleKey_t
	{
		double		time;
		sm::Vector3	val;
	};
	struct rotationKey_t
	{
		double			time;
		sm::Quaternion	val;
	};
	struct keyFrames_t
	{
		std::vector<positionKey_t>	position;
		std::vector<scaleKey_t>		scale;
		std::vector<rotationKey_t>	rotation;
	};

	//Each bone has a list of translations
	std::unordered_map<std::string, keyFrames_t> m_keyFrames;

private:
	void LoadPositions(const std::string& bonename, aiNodeAnim* channel);
	void LoadScales(const std::string& bonename, aiNodeAnim* channel);
	void LoadRotations(const std::string& bonename, aiNodeAnim* channel);
	void LoadKeyframes(const aiAnimation* animation);

public:
	RAnimation();
	~RAnimation();

	//Create from a assimp-animation if needed
	void Create(const aiAnimation* animation);

	// Inherited via GResource
	virtual bool Create(const std::string& filename) override;

	//Set and get
	void SetLoopable(bool& enable);
	bool IsLoopable() const;
	void SetTicksPerFrame(const double& speed);
	const double& GetTicksPerFrame() const;
	const double& GetDuration() const;
	const double  GetDurationInSeconds() const;

	/*
		Data for doing animation
	*/
	const sm::Vector3	 GetPosition(const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const;
	const sm::Vector3	 GetScale(	 const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const;
	const sm::Quaternion GetRotation(const std::string& bonename, const double& currentFrame, UINT& lastKey, bool interpolate) const;

	/*
		Get translations depending on the bone.
		Current frame is the frame we shall get information about.
		Last keys is a way to optimize the search for next pos,scl,rot.
		Can use interpolation for smoother animations. By default it's is on
	*/
	const sm::Matrix GetMatrix(const std::string& bonename, 
								const double& currentFrame, 
								UINT (&lastKeys)[3], 
								bool interpolate = true) const;

};