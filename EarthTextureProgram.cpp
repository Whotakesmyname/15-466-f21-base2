#include "EarthTextureProgram.hpp"

#include <fstream>

#include "gl_compile_program.hpp"
#include "gl_errors.hpp"
#include "load_save_png.hpp"
#include "data_path.hpp"

Scene::Drawable::Pipeline earth_texture_program_pipeline;

Load< EarthTextureProgram > earth_texture_program(LoadTagEarly, []() -> EarthTextureProgram const * {
	EarthTextureProgram *ret = new EarthTextureProgram();

	//----- build the pipeline template -----
	earth_texture_program_pipeline.program = ret->program;

	earth_texture_program_pipeline.OBJECT_TO_CLIP_mat4 = ret->OBJECT_TO_CLIP_mat4;
	earth_texture_program_pipeline.OBJECT_TO_LIGHT_mat4x3 = ret->OBJECT_TO_LIGHT_mat4x3;
	earth_texture_program_pipeline.NORMAL_TO_LIGHT_mat3 = ret->NORMAL_TO_LIGHT_mat3;

	/* This will be used later if/when we build a light loop into the Scene:
	earth_texture_program_pipeline.LIGHT_TYPE_int = ret->LIGHT_TYPE_int;
	earth_texture_program_pipeline.LIGHT_LOCATION_vec3 = ret->LIGHT_LOCATION_vec3;
	earth_texture_program_pipeline.LIGHT_DIRECTION_vec3 = ret->LIGHT_DIRECTION_vec3;
	earth_texture_program_pipeline.LIGHT_ENERGY_vec3 = ret->LIGHT_ENERGY_vec3;
	earth_texture_program_pipeline.LIGHT_CUTOFF_float = ret->LIGHT_CUTOFF_float;
	*/

	{
		//load daytime, night, cloud textures from improvised binary format:
		GLuint tex[EarthTextureProgram::TEXTURE_COUNT];
		glGenTextures(EarthTextureProgram::TEXTURE_COUNT, tex);
		std::vector< char > tex_data;
		uint32_t size[2];
		std::ifstream earth_tex_file(data_path("earth.tex"), std::ios::binary);

		earth_tex_file.read(reinterpret_cast<char*>(size), 2*sizeof(uint32_t));
		tex_data.resize(3*size[0]*size[1]);
		earth_tex_file.read(&tex_data[0], tex_data.size());
		std::printf("earth daymap size: %u, %u\n", size[0], size[1]);
		glBindTexture(GL_TEXTURE_2D, tex[EarthTextureProgram::DAYTIME_TEX]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size[0], size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		tex_data.clear();

		earth_tex_file.read(reinterpret_cast<char*>(size), 2*sizeof(uint32_t));
		tex_data.resize(3*size[0]*size[1]);
		earth_tex_file.read(&tex_data[0], tex_data.size());
		std::printf("earth nightmap size: %u, %u\n", size[0], size[1]);
		glBindTexture(GL_TEXTURE_2D, tex[EarthTextureProgram::NIGHT_TEX]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size[0], size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		tex_data.clear();

		earth_tex_file.read(reinterpret_cast<char*>(size), 2*sizeof(uint32_t));
		tex_data.resize(size[0]*size[1]);  // 1-channel grayscale data
		earth_tex_file.read(&tex_data[0], tex_data.size());
		std::printf("earth cloudmap size: %u, %u\n", size[0], size[1]);
		glBindTexture(GL_TEXTURE_2D, tex[EarthTextureProgram::CLOUD_TEX]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size[0], size[1], 0, GL_RED, GL_UNSIGNED_BYTE, tex_data.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		
		glBindTexture(GL_TEXTURE_2D, 0);

		for (size_t i = 0; i < EarthTextureProgram::TEXTURE_COUNT; ++i) {
			earth_texture_program_pipeline.textures[i].texture = tex[i];
			earth_texture_program_pipeline.textures[i].target = GL_TEXTURE_2D;
		}
	}

	return ret;
});

EarthTextureProgram::EarthTextureProgram() {
	//Compile vertex and fragment shaders using the convenient 'gl_compile_program' helper function:
	program = gl_compile_program(
		//vertex shader:
		"#version 330\n"
		"uniform mat4 OBJECT_TO_CLIP;\n"
		"uniform mat4x3 OBJECT_TO_LIGHT;\n"
		"uniform mat3 NORMAL_TO_LIGHT;\n"
		"in vec4 Position;\n"
		"in vec3 Normal;\n"
		"in vec4 Color;\n"
		"in vec2 TexCoord;\n"
		"out vec3 position;\n"
		"out vec3 normal;\n"
		"out vec4 color;\n"
		"out vec2 texCoord;\n"
		"void main() {\n"
		"	gl_Position = OBJECT_TO_CLIP * Position;\n"
		"	position = OBJECT_TO_LIGHT * Position;\n"
		"	normal = NORMAL_TO_LIGHT * Normal;\n"
		"	color = Color;\n"
		"	texCoord = TexCoord;\n"
		"}\n"
	,
		//fragment shader:
		"#version 330\n"
		"uniform sampler2D DAYTIME_TEX;\n"
		"uniform sampler2D NIGHT_TEX;\n"
		"uniform sampler2D CLOUD_TEX;\n"
		"uniform int LIGHT_TYPE;\n"
		"uniform vec3 LIGHT_LOCATION;\n"
		"uniform vec3 LIGHT_DIRECTION;\n"
		"uniform vec3 LIGHT_ENERGY;\n"
		"uniform float LIGHT_CUTOFF;\n"
		"in vec3 position;\n"
		"in vec3 normal;\n"
		"in vec4 color;\n"
		"in vec2 texCoord;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	vec3 n = normalize(normal);\n"
		"	vec3 e;\n"
		"	if (LIGHT_TYPE == 0) { //point light \n"
		"		vec3 l = (LIGHT_LOCATION - position);\n"
		"		float dis2 = dot(l,l);\n"
		"		l = normalize(l);\n"
		"		float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"		e = nl * LIGHT_ENERGY;\n"
		"	} else if (LIGHT_TYPE == 1) { //hemi light \n"
		"		e = (dot(n,-LIGHT_DIRECTION) * 0.5 + 0.5) * LIGHT_ENERGY;\n"
		"	} else if (LIGHT_TYPE == 2) { //spot light \n"
		"		vec3 l = (LIGHT_LOCATION - position);\n"
		"		float dis2 = dot(l,l);\n"
		"		l = normalize(l);\n"
		"		float nl = max(0.0, dot(n, l)) / max(1.0, dis2);\n"
		"		float c = dot(l,-LIGHT_DIRECTION);\n"
		"		nl *= smoothstep(LIGHT_CUTOFF,mix(LIGHT_CUTOFF,1.0,0.1), c);\n"
		"		e = nl * LIGHT_ENERGY;\n"
		"	} else { //(LIGHT_TYPE == 3) //directional light \n"
		"		e = max(0.0, dot(n,-LIGHT_DIRECTION)) * LIGHT_ENERGY;\n"
		"	}\n"
		"	vec3 day = texture(DAYTIME_TEX, texCoord).rgb;\n"
		"	vec3 night = texture(NIGHT_TEX, texCoord).rgb;\n"
		"	float cloud = texture(CLOUD_TEX, texCoord).r;\n"
		"	vec3 albedo = mix(day, night, e.r);\n"
		"	fragColor = vec4(mix(albedo, vec3(1.0f), cloud), 1.0f);\n"
		"}\n"
	);
	//As you can see above, adjacent strings in C/C++ are concatenated.
	// this is very useful for writing long shader programs inline.

	//look up the locations of vertex attributes:
	Position_vec4 = glGetAttribLocation(program, "Position");
	Normal_vec3 = glGetAttribLocation(program, "Normal");
	Color_vec4 = glGetAttribLocation(program, "Color");
	TexCoord_vec2 = glGetAttribLocation(program, "TexCoord");

	//look up the locations of uniforms:
	OBJECT_TO_CLIP_mat4 = glGetUniformLocation(program, "OBJECT_TO_CLIP");
	OBJECT_TO_LIGHT_mat4x3 = glGetUniformLocation(program, "OBJECT_TO_LIGHT");
	NORMAL_TO_LIGHT_mat3 = glGetUniformLocation(program, "NORMAL_TO_LIGHT");

	LIGHT_TYPE_int = glGetUniformLocation(program, "LIGHT_TYPE");
	LIGHT_LOCATION_vec3 = glGetUniformLocation(program, "LIGHT_LOCATION");
	LIGHT_DIRECTION_vec3 = glGetUniformLocation(program, "LIGHT_DIRECTION");
	LIGHT_ENERGY_vec3 = glGetUniformLocation(program, "LIGHT_ENERGY");
	LIGHT_CUTOFF_float = glGetUniformLocation(program, "LIGHT_CUTOFF");


	GLuint DAYTIME_TEX_sampler2D = glGetUniformLocation(program, "DAYTIME_TEX");
	GLuint NIGHT_TEX_sampler2D = glGetUniformLocation(program, "NIGHT_TEX");
	GLuint CLOUD_TEX_sampler2D = glGetUniformLocation(program, "CLOUD_TEX");

	//set TEX to always refer to texture binding zero:
	glUseProgram(program); //bind program -- glUniform* calls refer to this program now

	glUniform1i(DAYTIME_TEX_sampler2D, 0);
	glUniform1i(NIGHT_TEX_sampler2D, 1);
	glUniform1i(CLOUD_TEX_sampler2D, 2);

	glUseProgram(0); //unbind program -- glUniform* calls refer to ??? now
}

EarthTextureProgram::~EarthTextureProgram() {
	glDeleteProgram(program);
	program = 0;
}

