#pragma once

class Stats
{
private:
	Stats();
	~Stats() = default;

	//How long since last update
	float m_frameTime;
	float m_updateTime;
	float m_networkTime;

	//How often to render a frame
	float m_framerate;

	//How often to update the information
	float m_updaterate;
	
	//How often to get and send to server
	float m_tickrate;

public:
	//Delete copy constructors
	Stats(const Stats& rm) = delete;
	Stats(const Stats&& rm) = delete;
	Stats& operator=(const Stats& rm) = delete;
	Stats& operator=(const Stats&& rm) = delete;

	static auto& Get()
	{
		static Stats instance;
		return instance;
	}

	//Frametime
	void			SetFrameTime(const float& ft);
	const float&	GetFrameTime() const;

	//Updatetime
	void			SetUpdateTime(const float& ut);
	void			SetUpdateRate(const float& rate);
	const float&	GetUpdateTime() const;

	//Networktime
	void			SetNetworkTime(const float& nt);
	const float&	GetNetworkTime() const;

	//Which rate to update in
	const float&	GetFramerate() const;
	const float&	GetUpdaterate() const;
	const float&	GetTickrate() const;

};