/**
* Author: Justin Yee
* Assignment: Rise of the AI
* Date due: 2024-11-09, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 1
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 17

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"


// ————— GAME STATE ————— //
struct GameState
{
    Entity *player;

    Entity *basilisk;
    Entity* chad;
	Entity* fish;


    Entity* text;

    Map *map;
    
    Mix_Music *bgm;
    Mix_Chunk *jump_sfx;
	Mix_Chunk* yell_sfx;
};

enum AppStatus { RUNNING, TERMINATED };

// ————— CONSTANTS ————— //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.0f,
            BG_BLUE    = 0.0f,
            BG_GREEN   = 0.0f,
            BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "RISE!";

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;


constexpr char SPRITESHEET_FILEPATH[] = "assets/mc.png", //From https://opengameart.org/content/anime-collection
MAP_TILESET_FILEPATH[] = "assets/map.png", //From https://opengameart.org/content/tile-map-dark-2d-platformer
TEXT_FILEPATH[] = "assets/font1.png",

BASILISK_FILEPATH[] = "assets/basilisk.png", //From https://opengameart.org/content/rpg-enemy-basilisk-ff6-ish-style
CHAD_FILEPATH[] = "assets/chad.png", //From https://opengameart.org/content/3-rpg-enemy-remixes
FISH_FILEPATH[] = "assets/fish.png", //From https://opengameart.org/content/3-rpg-enemy-remixes

BGM_FILEPATH[] = "assets/SCP-x3x.mp3", //From Incompetech
JUMP_SFX_FILEPATH[] = "assets/jump.wav", //From https://opengameart.org/content/jump-landing-sound
YELL_SFX_FILEPATH[] = "assets/yell.wav"; //From https://opengameart.org/content/battlecry


constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;

unsigned int LEVEL_1_DATA[] =
{
    36, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 49,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 10, 10, 10, 10, 10, 10, 10, 10, 12, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,

    55, 0, 5, 10, 12, 0, 0, 0, 5, 10, 10, 10, 10, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 5, 10, 10, 10, 10, 12, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 5, 10, 12, 0, 0, 0, 0, 0, 0, 0, 55,

    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 5, 10, 10, 10, 10, 10, 10, 10, 10, 12, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 55,
    55, 8, 8, 8, 8, 8, 8, 0, 0, 8, 0, 8, 0, 55,
    55, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 55,

    18, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 87,

}; 

bool win = false;
bool lose = false;

int player_walking_animation[6][4] =
{
    { 0, 1, 2, 3 },  // for mc to move to the left,
    { 4, 5, 6, 7 }, // for mc to move to the right,
    { 8, 9, 10, 11 }, // for mc to run left,
    { 12, 13, 14, 15 },   // for mc to run right
    { 16, 17, 18, 19 },   // for mc to attack left
    { 20, 21, 22, 23 }   // for mc to attack right
};

// ————— VARIABLES ————— //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f,
      g_accumulator    = 0.0f;


void initialise();
void process_input();
void update();
void render();
void shutdown();

// ————— GENERAL FUNCTIONS ————— //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return texture_id;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 6, 14);

    // ————— GEORGE SET-UP ————— //

    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    GLuint basilisk_texture_id = load_texture(BASILISK_FILEPATH);
    GLuint chad_texture_id = load_texture(CHAD_FILEPATH);
    GLuint fish_texture_id = load_texture(FISH_FILEPATH);
	GLuint text_texture_id = load_texture(TEXT_FILEPATH);


    glm::vec3 acceleration = glm::vec3(0.0f, -4.905f, 0.0f);

    g_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        1.15f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        4,                         // animation frame amount
        0,                         // current animation index
        4,                         // animation column amount
        6,                         // animation row amount
        0.4f,                      // width
        0.8f,                       // height
        PLAYER
    );

    g_game_state.player->set_position(glm::vec3(7.0f, -2.0f, 0.0f));


    g_game_state.basilisk = new Entity(basilisk_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, ZOOM, IDLE);
    g_game_state.basilisk->set_position(glm::vec3(1.0f, -15.0f, 0.0f));
	g_game_state.basilisk->set_acceleration(acceleration);
    g_game_state.basilisk->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));
    g_game_state.basilisk->update(0, NULL, NULL, 0, g_game_state.map);

    g_game_state.chad = new Entity(chad_texture_id, 1.0f, .8f, 1.0f, ENEMY, MONKEY, IDLE);
    
    g_game_state.chad->set_position(glm::vec3(6.0f, -3.0f, 0.0f));

    //No acceleration because I want it to "stick" to the bottom of the tile
    g_game_state.chad->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.chad->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));
    g_game_state.chad->update(0, NULL, NULL, 0, g_game_state.map);
	

	g_game_state.fish = new Entity(fish_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, IDLE);
    g_game_state.fish->set_position(glm::vec3(1.0f, -1.0f, 0.0f));
    g_game_state.fish->set_acceleration(acceleration);
    g_game_state.fish->set_scale(glm::vec3(2.0f, 2.0f, 1.0f));

    g_game_state.fish->update(0, NULL, NULL, 0, g_game_state.map);

    g_game_state.text = new Entity();
	g_game_state.text->set_texture_id(text_texture_id);

    // Jumping
    g_game_state.player->set_jumping_power(5.0f);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_game_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 8.0f);
    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);
	g_game_state.yell_sfx = Mix_LoadWAV(YELL_SFX_FILEPATH);

    Mix_VolumeChunk(g_game_state.yell_sfx, MIX_MAX_VOLUME / 16.0f);
    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_game_state.player->get_collided_bottom())
                        {
                            g_game_state.player->jump();
                            Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                        }
                        
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    //move left and right, can only dash while on the ground
    if (key_state[SDL_SCANCODE_LEFT])
    {

        //will yell whenever dashing, feel free to remove it if it gets annoying
        if (key_state[SDL_SCANCODE_LSHIFT] && g_game_state.player->get_collided_bottom())
        {
            Mix_PlayChannel(-1, g_game_state.yell_sfx, 0); 
            g_game_state.player->run_left();
        }
        else
        {
            g_game_state.player->move_left();
        }
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        if (key_state[SDL_SCANCODE_LSHIFT] && g_game_state.player->get_collided_bottom())
        {
            Mix_PlayChannel(-1, g_game_state.yell_sfx, 0);
            g_game_state.player->run_right();
        } 
        
        else
        {
            g_game_state.player->move_right();
        }
    }


    if (glm::length(g_game_state.player->get_movement()) > 1.5f)
        g_game_state.player->normalise_movement();
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    

    if (!g_game_state.player->get_is_active())
    {
        lose = true;
    }

    if (!g_game_state.fish->get_is_active() && !g_game_state.chad->get_is_active() && !g_game_state.basilisk->get_is_active())
    {
        win = true;
    }

    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0, 
                                    g_game_state.map);
        
		g_game_state.player->check_collision_x(g_game_state.fish, 1);
		g_game_state.player->check_collision_x(g_game_state.chad, 1);
		g_game_state.player->check_collision_x(g_game_state.basilisk, 1);

		g_game_state.player->check_collision_y(g_game_state.fish, 1);
		g_game_state.player->check_collision_y(g_game_state.chad, 1);
		g_game_state.player->check_collision_y(g_game_state.basilisk, 1);

        g_game_state.basilisk->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0,
            g_game_state.map);
		g_game_state.chad->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0,
			g_game_state.map);
        g_game_state.fish->update(FIXED_TIMESTEP, g_game_state.player, NULL, 0,
            g_game_state.map);
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    g_view_matrix = glm::mat4(1.0f);
    
    // Camera Follows the player
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, -g_game_state.player->get_position().y, 0.0f));
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glClear(GL_COLOR_BUFFER_BIT);



    //draws collidable entities if they are active
    if (g_game_state.player->get_is_active())
    {
        g_game_state.player->render(&g_shader_program);
    }

    if (g_game_state.basilisk->get_is_active())
    {
        g_game_state.basilisk->render(&g_shader_program);
    }

    if (g_game_state.fish->get_is_active())
    {
        g_game_state.fish->render(&g_shader_program);
    }

    if (g_game_state.chad->get_is_active())
    {

        g_game_state.chad->render(&g_shader_program);
    }

    
	
	//only draws if lose or win
    g_game_state.map->render(&g_shader_program);
    if (win)
    {
        g_game_state.text->draw_text(&g_shader_program, "You Win!",
            .75, .001, glm::vec3(g_game_state.player->get_position().x - 3, g_game_state.player->get_position().y + 1, 0.0f));
    }
    if (lose)
    {
        g_game_state.text->draw_text(&g_shader_program, "You Lose :(",
            .75, .001, glm::vec3(g_game_state.player->get_position().x - 3, g_game_state.player->get_position().y + 1, 0.0f));
    }


    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{    
    SDL_Quit();
    
    delete    g_game_state.player;
	delete g_game_state.basilisk;
	delete g_game_state.chad;
	delete g_game_state.fish;
    delete    g_game_state.map;
	delete    g_game_state.text;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
