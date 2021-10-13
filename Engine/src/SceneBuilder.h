#pragma once
#include "HeadlessEngine.h"

template<typename T>
std::string GetTypeName()
{
	std::string name = typeid(T).name();
	name = name.substr(name.find_first_of(" ") + 1);
	return name;
}

template<typename T, typename SceneType>
class SceneBuilder
{
protected:
	HeadlessEngine<SceneType>& m_headlessEngine;
	SceneType& m_scene;
	std::string m_sceneName;
public:
	SceneBuilder(HeadlessEngine<SceneType>& m_engine);

	SceneType& GetScene() const;

};

template<typename T, typename SceneType>
inline SceneBuilder<T, SceneType>::SceneBuilder(HeadlessEngine<SceneType>& engine)
	: m_headlessEngine(engine)
	, m_scene(engine.GetScene(GetTypeName<T>()))
	, m_sceneName(GetTypeName<T>())
{
	LOG_INFO("Created Scene: %s", m_sceneName.c_str());

}

template<typename T, typename SceneType>
inline SceneType& SceneBuilder<T, SceneType>::GetScene() const
{
	return m_scene;
}

