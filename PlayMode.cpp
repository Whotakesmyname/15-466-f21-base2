#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "EarthTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint ntfc_meshes_for_lit_color_texture_program = 0;
GLuint ntfc_meshes_for_earth_texture_program = 0;
Load< MeshBuffer > ntfc_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("NTFC.pnct"));
	ntfc_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	ntfc_meshes_for_earth_texture_program = ret->make_vao_for_program(earth_texture_program->program);
	return ret;
});

Load< Scene > ntfc_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("NTFC.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = ntfc_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		// earth rendering uses a different fragment shader with 3 textures
		if (mesh_name == "Earth") {
			drawable.pipeline = earth_texture_program_pipeline;
			drawable.pipeline.vao = ntfc_meshes_for_earth_texture_program;
		} else {
			drawable.pipeline = lit_color_texture_program_pipeline;
			drawable.pipeline.vao = ntfc_meshes_for_lit_color_texture_program;
		}
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*ntfc_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Earth") earth = &transform;
		else if (transform.name == "Plane") orbit_plane = &transform;
		else if (transform.name == "ISS") iss = &transform;
	}
	if (earth == nullptr) throw std::runtime_error("Earth not found.");
	if (orbit_plane == nullptr) throw std::runtime_error("Plane not found.");
	if (iss == nullptr) throw std::runtime_error("Iss not found.");

	earth_rotation = earth->rotation;
	orbit_plane_rotation = orbit_plane->rotation;
	iss_rotation = iss->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
	cam = camera->transform;
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		switch (evt.key.keysym.sym) {
			case SDLK_a: {
				left.downs += 1;
				left.pressed = true;
				return true;
			}
			case SDLK_d: {
				right.downs += 1;
				right.pressed = true;
				return true;
			}
			case SDLK_w: {
				forward.downs += 1;
				forward.pressed = true;
				return true;
			}
			case SDLK_s: {
				backward.downs += 1;
				backward.pressed = true;
				return true;
			}
			case SDLK_q: {
				rleft.downs += 1;
				rleft.pressed = true;
				return true;
			}
			case SDLK_e: {
				rright.downs += 1;
				rright.pressed = true;
				return true;
			}
			case SDLK_RCTRL:
			case SDLK_LCTRL: {
				down.downs += 1;
				down.pressed = true;
				return true;
			}
			case SDLK_SPACE: {
				up.downs += 1;
				up.pressed = true;
				return true;
			}
		}
	} else if (evt.type == SDL_KEYUP) {
		switch (evt.key.keysym.sym) {
			case SDLK_a: {
				left.pressed = false;
				return true;
			}
			case SDLK_d: {
				right.pressed = false;
				return true;
			}
			case SDLK_w: {
				forward.pressed = false;
				return true;
			}
			case SDLK_s: {
				backward.pressed = false;
				return true;
			}
			case SDLK_q: {
				rleft.pressed = false;
				return true;
			}
			case SDLK_e: {
				rright.pressed = false;
				return true;
			}
			case SDLK_RCTRL:
			case SDLK_LCTRL: {
				down.pressed = false;
				return true;
			}
			case SDLK_SPACE: {
				up.pressed = false;
				return true;
			}
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	//update space station rotation
	iss->rotation *= glm::angleAxis(iss_rotation_increment * elapsed, iss_forward_direction);
	//update space station orbit
	orbit_plane->rotation *= glm::angleAxis(orbit_rotation_increment * elapsed, orbit_rotation_axis);
	//update earth rotation
	earth->rotation *= glm::angleAxis(earth_rotation_increment * elapsed, earth_rotation_axis);

	//move camera:
	{

		//combine inputs into a move:
		if (left.pressed && !right.pressed) camera_move_speed_multiplier.x -= 1;
		if (!left.pressed && right.pressed) camera_move_speed_multiplier.x += 1;
		if (forward.pressed && !backward.pressed) camera_move_speed_multiplier.y += 1;
		if (!forward.pressed && backward.pressed) camera_move_speed_multiplier.y -= 1;
		if (down.pressed && !up.pressed) camera_move_speed_multiplier.z -= 1;
		if (!down.pressed && up.pressed) camera_move_speed_multiplier.z += 1;
		if (rleft.pressed && !rright.pressed) camera_rotation_speed_multiplier += 1;
		if (!rleft.pressed && rright.pressed) camera_rotation_speed_multiplier -= 1;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right = frame[0];
		glm::vec3 up = frame[1];
		glm::vec3 forward = -frame[2];

		camera->transform->position += glm::mat3x3(right, forward, up) * camera_move_speed_multiplier * camera_move_increment * elapsed;
		camera->transform->rotation *= glm::angleAxis(camera_rotation_speed_multiplier * camera_rotation_increment * elapsed, camera_forward_direction);

		//decide if docking successful
		//rotation match
		if (abs(camera_rotation_increment*camera_rotation_speed_multiplier + iss_rotation_increment) <= SUCCESS_ROTATION_DELTA) {
			//iss and camera belong to the same direct parent
			std::printf("distance: %f\n", glm::length(iss->position - cam->position));
			if (glm::length(iss->position - cam->position) <= SUCCESS_DISTANCE) {
				succeed = true;
			}
		}
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 3);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(light_dir));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.f, 1.f, 1.f)));
	glUseProgram(0);

	glUseProgram(earth_texture_program->program);
	glUniform1i(earth_texture_program->LIGHT_TYPE_int, 3);
	glUniform3fv(earth_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(light_dir));
	glUniform3fv(earth_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.f, 1.f, 1.f)));
	glUseProgram(0);

	glClearColor(0.f, 0.f, 0.f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		const std::string& text = succeed ? text_success : text_note;

		constexpr float H = 0.09f;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(text,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
