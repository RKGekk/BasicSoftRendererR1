#pragma once

#include <string>

class Scene {
public:
	Scene(const std::string& name) : name(name) {}
	virtual ~Scene() = default;

	virtual void Update(float dt) = 0;
	virtual void Draw() = 0;
	
	const std::string& GetName() const;

private:
	std::string name;
};