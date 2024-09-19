#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint rhythm_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > rhythm_meshes(LoadTagDefault, []() -> MeshBuffer const * {
    MeshBuffer const *ret = new MeshBuffer(data_path("rhythm.pnct"));
    rhythm_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
    return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("rhythm.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = rhythm_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = rhythm_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

Load< Sound::Sample > rhythm_sample(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("rhythm.wav"));
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Sphere") player = &transform;
        else if (transform.name.find("Cube") != std::string::npos) {
            glm::vec3 platform;
            platform.x = transform.position.x;
            platform.y = transform.position.y;
            platform.z = transform.position.z;
            glm::vec3 size = glm::vec3(transform.scale.x, transform.scale.y, transform.scale.z);

            glm::vec3 min = platform - size;
            glm::vec3 max = platform + size;

            platforms.push_back(Platform{min, max});
        }
	}
	if (player == nullptr) throw std::runtime_error("Player not found.");
    
	
	player_rotation = player->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	//start music loop playing:
	// (note: position will be over-ridden in update())
	rhythm_loop = Sound::loop_3D(*rhythm_sample, 1.0f, get_player_position(), 10.0f);
}


PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
        if (evt.key.keysym.sym == SDLK_SPACE) {
            space.downs += 1;
            space.pressed = true;
            return true;
        }else if (evt.key.keysym.sym == SDLK_ESCAPE) {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            return true;
        } else if (evt.key.keysym.sym == SDLK_a) {
            A.downs += 1;
            A.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d) {
            D.downs += 1;
            D.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_w) {
            W.downs += 1;
            W.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_s) {
            S.downs += 1;
            S.pressed = true;
            return true;
        }else if (evt.key.keysym.sym == SDLK_r) {
            R.pressed = true;
            return true;
        }
	} else if (evt.type == SDL_KEYUP) {
        if (evt.key.keysym.sym == SDLK_SPACE) {
            space.pressed = false;
            return true;
        }else if (evt.key.keysym.sym == SDLK_a) {
            A.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_d) {
            D.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_w) {
            W.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_s) {
            S.pressed = false;
            return true;
        }else if (evt.key.keysym.sym == SDLK_r) {
            R.pressed = false;
            return true;
        }
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);
/*
	hip->rotation = hip_base_rotation * glm::angleAxis(
		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

*/
    //move sound to follow player position:
    rhythm_loop->set_position(get_player_position(), 1.0f / 60.0f);
	
	

//move camera:
    {
        
        //combine inputs into a move:
        //constexpr float CameraSpeed = 30.0f;
        glm::vec3 move = glm::vec3(0.0f);
        
        
        move.x = player->position.x - camera->transform->position.x;
        move.y = (player->position.y - 30.0f) - camera->transform->position.y;
        move.z = (player->position.z + 7.5f) - camera->transform->position.z;
        camera->transform->position += move;
        //std::cout << move.x << move.y << move.z << "\n";
        
        /*
        //make it so that moving diagonally doesn't go faster:
        if (move != glm::vec2(0.0f)) move = glm::normalize(move) * CameraSpeed * elapsed;

        glm::mat4x3 frame = camera->transform->make_local_to_parent();
        glm::vec3 frame_right = frame[0];
        //glm::vec3 up = frame[1];
        glm::vec3 frame_forward = -frame[2];
        
        */
    }

    { //update listener to camera position:
        glm::mat4x3 frame = camera->transform->make_local_to_parent();
        glm::vec3 frame_right = frame[0];
        glm::vec3 frame_at = frame[3];
        Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
    }
    
    //check if player fell
    if(player->position.z < -10.0f){
        player_died = true;
        rhythm_loop->stop( 1.0f / 60.0f);

    }
    
    if(player->position.y > 322.0f){
        game_win = true;
        rhythm_loop->stop( 1.0f / 60.0f);
    }
    
    //reset player position and sound`
    if(player_died && R.pressed){
        player_died = false;
        player->position.x = 0.0f;
        player->position.y = 1.0f;
        player->position.z = 1.0f;
        rhythm_loop = Sound::loop_3D(*rhythm_sample, 1.0f, get_player_position(), 10.0f);
    }
    //combine inputs into a move:
    constexpr float PlayerSpeed = 11.0f;
    constexpr float JumpSpeed = 20.0f;
    constexpr float Gravity = -10.0f;
    
    //update player position
    glm::vec3 newPosition = player->position;
    
    
    if (!on_ground and !space.pressed) newPosition.z += Gravity * elapsed;
    //std::cout<<is_jumping<<"\n";
    if (space.pressed){
        is_jumping = true;
        on_ground = false;
        newPosition.z += JumpSpeed * elapsed;
        //newPosition.y += 5.0f * elapsed;
    }
    newPosition.y += PlayerSpeed * elapsed;
    //check for collisions
    glm::vec3 player_min = newPosition - glm::vec3(player->scale.x, player->scale.y, player->scale.z);
    glm::vec3 player_max = newPosition + glm::vec3(player->scale.x, player->scale.y, player->scale.z);
    
    
    if(!game_win and !player_died){
        on_ground = false;
        for (auto& platform : platforms) {
            if (player_max.x >= platform.min.x && player_min.x <= platform.max.x &&
                player_max.y >= platform.min.y && player_min.y <= platform.max.y &&
                player_max.z >= platform.min.z && player_min.z <= platform.max.z) {
                
               
                float overlapX = std::min(player_max.x - platform.min.x, platform.max.x - player_min.x);
                float overlapY = std::min(player_max.y - platform.min.y, platform.max.y - player_min.y);
                float overlapZ = std::min(player_max.z - platform.min.z, platform.max.z - player_min.z);
                
                
                if (overlapX < overlapY && overlapX < overlapZ) {
                    //side
                    if (player_min.x < platform.min.x) {
                        newPosition.x = platform.min.x - player->scale.x;
                    } else {
                        newPosition.x = platform.max.x + player->scale.x;
                    }
                } else if (overlapZ < overlapX && overlapZ < overlapY) {
                    //horizontal
                    if (player_min.z <= platform.max.z) {
                        newPosition.z = platform.max.z + player->scale.z;
                        on_ground = true;
                        is_jumping = false;
                    }
                } else if (overlapY < overlapX && overlapY < overlapZ) {
                    //front
                    if (player_max.y > platform.min.y) {
                        newPosition.y = platform.min.y - player->scale.y;
                    } else if(player_min.y < platform.max.y){
                        newPosition.y = platform.max.y + player->scale.y;
                    }
                }
            }
            
        }
     
        player->position = newPosition;

    }
   
    //reset button press counters:
    A.downs = 0;
    D.downs = 0;
    W.downs = 0;
    S.downs = 0;
    R.downs = 0;
    space.downs = 0;
    
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.8f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

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
        constexpr float H = 0.3f;
        float ofs = 2.0f / drawable_size.y;
        if(!game_win and !player_died){
            lines.draw_text("Press Space to Jump",
                glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        }else if(game_win){
            lines.draw_text("You Won",
                glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        }else if(player_died){
            lines.draw_text("Press R to Restart",
                glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        }
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_player_position() {
	//the vertex position here was read from the model in blender:
	return player->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}
