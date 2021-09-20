#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, rleft, rright, forward, backward;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//success flag
	bool succeed = false;

	//text
	const std::string text_success = "Docking successfully!";
	const std::string text_note = "QE rotates; WASD moves around; Space & Ctrl moves up and down";


	//transform constants:
	glm::vec3 light_dir = glm::normalize(glm::vec3(1.f, 1.f, 0.f));
	
	constexpr static float SUCCESS_DISTANCE = 0.3f;
	constexpr static float SUCCESS_ROTATION_DELTA = 0.05f;

	constexpr static float iss_rotation_increment = 4.f;
	constexpr static glm::vec3 iss_forward_direction = glm::vec3(0.f, 1.f, 0.f);
	constexpr static float orbit_rotation_increment = 0.04f;
	constexpr static glm::vec3 orbit_rotation_axis = glm::vec3(0.f, 0.f, 1.f);
	constexpr static float earth_rotation_increment = 0.02f;
	constexpr static glm::vec3 earth_rotation_axis = glm::vec3(0.f, 0.f, 1.f);
	
	constexpr static float camera_rotation_increment = 0.02f;
	constexpr static float camera_move_increment = 0.01f;
	constexpr static glm::vec3 camera_forward_direction = glm::vec3(0.f, 0.f, 1.f);

	//transform states:
	Scene::Transform *earth = nullptr;
	Scene::Transform *orbit_plane = nullptr;
	Scene::Transform *iss = nullptr;
	Scene::Transform *cam = nullptr;
	glm::quat earth_rotation;
	glm::quat orbit_plane_rotation;
	glm::quat iss_rotation;
	glm::quat camera_rotation;
	int camera_rotation_speed_multiplier = 0;
	glm::ivec3 camera_move_speed_multiplier = glm::ivec3(0);
	
	//camera:
	Scene::Camera *camera = nullptr;

};
