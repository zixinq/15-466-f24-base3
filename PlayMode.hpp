#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

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
    } space, W, S, A, D, R;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//player to wobble:
	Scene::Transform *player = nullptr;
	glm::quat player_rotation;

	float wobble = 0.0f;
        
    struct Platform {
        glm::vec3 min; //bottom-left corner
        glm::vec3 max; //top-right corner
    };

    std::vector<Platform> platforms;
    
    bool on_ground = true;
    bool is_jumping = false;
    
    bool game_win = false;
    bool player_died = false;
    
    static const glm::vec3 gravity;
    glm::vec3 ballVelocity;
    

	glm::vec3 get_player_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > rhythm_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
