#pragma once

class Stats
{
private:
	Stats();
	~Stats() = default;

	static auto& Get()
	{
		static Stats instance;
		return instance;
	}

	//Time it takes between each frame
	float m_deltaTime;
	float m_maxFPS;
	
	const float&	IGetDeltaTime() const;
	void			ISetDeltaTime(const float& dt);
	const float		IGetCurrentFPS() const;
	const float&	IGetMaxFPS() const;

public:
	//Delete copy constructors
	Stats(const Stats& rm) = delete;
	Stats(const Stats&& rm) = delete;
	Stats& operator=(const Stats& rm) = delete;
	Stats& operator=(const Stats&& rm) = delete;

	static const float& GetDeltaTime()					{ return Get().IGetDeltaTime(); }
	static void			SetDeltaTime(const float& dt)	{ Get().ISetDeltaTime(dt); }
	static const float  GetCurrentFPS()					{ return Get().IGetCurrentFPS(); }
	static const float&	GetMaxFPS()						{ return Get().IGetMaxFPS(); }

};