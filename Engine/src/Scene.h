#pragma once

class Scene {
private:
	// ecs stuff
public:
	Scene();
	virtual ~Scene();

	// update all systems
	void update(float dt);
	
	// draw all renderSystems
	void render();

};