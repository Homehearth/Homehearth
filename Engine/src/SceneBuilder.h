#pragma once
#include "HeadlessEngine.h"

template<typename T>
std::string GetTypeName()
{
	std::string name = typeid(T).name();
	name = name.substr(name.find_first_of(" ") + 1);
	return name;
}

template<typename T>
class SceneBuilder
{
protected:
	HeadlessEngine& m_engine;
	Scene& m_scene;
	std::string m_sceneName;
public:
	SceneBuilder(HeadlessEngine& m_engine);

	Scene& GetScene() const;
};

template<typename T>
inline SceneBuilder<T>::SceneBuilder(HeadlessEngine& engine)
	: m_engine(engine)
	, m_scene(engine.GetScene(GetTypeName<T>()))
	, m_sceneName(GetTypeName<T>())
{
	LOG_INFO("Created Scene: %s", m_sceneName.c_str());
}

template<typename T>
inline Scene& SceneBuilder<T>::GetScene() const
{
	return m_scene;
}

