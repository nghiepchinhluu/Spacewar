#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include <windows.h>
#pragma warning (disable:4996)

#pragma comment(linker,"/subsystem:console")

// Include SDL lib
#include "Headers/SDL2-2.0.8/include/SDL.h"
#include "Headers/SDL2-2.0.8/include/SDL_image.h"
#include "Headers/SDL2-2.0.8/include/SDL_mixer.h"
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2_image.lib")
#pragma comment(lib,"Headers\\SDL2-2.0.8\\lib\\x86\\SDL2_mixer.lib")


SDL_Renderer *renderer = NULL;
int screen_w = 1280;
int screen_h = 800;
float dt = 0.05;
int score = 0;
int level = 1;
int alive = 1;
#define R() (1.0-2.0*rand()/RAND_MAX)
	
#include "Headers/objects.h"
#include "Headers/Collision.h"


void load_Image(SDL_Texture** texture, const char* filename)
{
	SDL_Surface* surface = IMG_Load(filename);
	*texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
}

void showScore(int Score, SDL_Texture* ScoreSheet_Text, SDL_Renderer* renderer, int start)
{
	int tmp = Score;
	int Ndigit = 0;
	while (tmp > 0)
	{
		tmp /= 10;
		Ndigit++;
	}
	
	SDL_Rect DestRect = { start,50,40,50 };
	while (Score > 0)
	{
		int tmp = Score % 10;
		SDL_Rect SrcRect = { tmp * 50,0,50,49 };
		SDL_RenderCopyEx(renderer, ScoreSheet_Text, &SrcRect, &DestRect, 0, 0, SDL_FLIP_NONE);
		Score /= 10;
		DestRect.x -= 40;
	}
}



void handleEvent1(int* enemies_active, int n_enemies, int* bullets_active, int n_bullets)
{
	for (int i = 0; i < n_enemies; i++)
	{
		if (enemies_active[i] == 1) score++;
		enemies_active[i] = 0;
	}
	for (int i = 0; i < n_bullets; i++)
	{
		bullets_active[i] = 0;
	}
}



int main(int argc, char **argv)
{
	srand(time(0));
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Rogue like game", 50, 50, screen_w, screen_h, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	// Load textures
	SDL_Texture* background_text = NULL;
	SDL_Texture* player_text = NULL;
	SDL_Texture** enemy_text = (SDL_Texture**)malloc(sizeof(SDL_Texture*) * 3);
	SDL_Texture* bullet_text = NULL;
	SDL_Texture* event1_text = NULL;
	SDL_Texture* event2_text = NULL;
	SDL_Texture* event3_text = NULL;
	SDL_Texture* scoresheet_text = NULL;
	SDL_Texture* level_text = NULL;
	SDL_Texture* dead_text = NULL;
	SDL_Texture* R_text = NULL;
	load_Image(&background_text, "Images/background.jpg");
	load_Image(&player_text, "Images/player.png");
	load_Image(&enemy_text[0], "Images/enemy1.png");
	load_Image(&enemy_text[1], "Images/enemy2.png");
	load_Image(&enemy_text[2], "Images/enemy3.png");
	load_Image(&bullet_text, "Images/bullet.png");
	load_Image(&event1_text, "Images/special_event1.png");
	load_Image(&event2_text, "Images/special_event2.png");
	load_Image(&event3_text, "Images/special_event3.png");
	load_Image(&scoresheet_text, "Images/score_sheet.png");
	load_Image(&level_text, "Images/level.png");
	load_Image(&dead_text, "Images/dead.png");



	// Initialize objects
	Player* player = new Player((float)screen_w / 2, (float)screen_h / 2, (float)0, (float)0, (float)50);
	Enemy** enemies_list = new Enemy*[200];
	int n_enemies = 200;
	int enemies_active[200] = { 0 };
	Bullet** bullets_list = new Bullet*[1000];
	int n_bullets = 1000;
	int bullets_active[1000] = { 0 };
	Object* event1 = NULL;
	Object* event2 = NULL;
	Object* event3 = NULL;
	int vulnerable = 1;
	// Initialize clocks
	unsigned int last_enemy_spawn = SDL_GetTicks();
	unsigned int last_bullet_spawn = SDL_GetTicks();
	unsigned int last_event1_taken = SDL_GetTicks();
	unsigned int last_event2_taken = SDL_GetTicks();
	unsigned int last_event3_taken = SDL_GetTicks();
	unsigned int last_level_up = 10;

	unsigned int enemy_spawn_freq = 1000;
	unsigned int bullet_spawn_freq = 200;
	unsigned int event1_freq = 10000;
	unsigned int event2_freq = 5000;
	unsigned int event3_freq = 5000;

	for (;;)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) return 0;
		}
		const Uint8* state = SDL_GetKeyboardState(NULL);
		if (alive==0 && state[SDL_SCANCODE_R])
		{
			delete player;
			player = new Player((float)screen_w / 2, (float)screen_h / 2, (float)0, (float)0, (float)50);
			alive = 1;
			score = 0;
			level = 1;
			last_level_up = 10;
			last_enemy_spawn = SDL_GetTicks();
			last_bullet_spawn = SDL_GetTicks();
			last_event1_taken = SDL_GetTicks();
			last_event2_taken = SDL_GetTicks();
			last_event3_taken = SDL_GetTicks();
			memset(enemies_active, 0, 200 * sizeof(int));
			memset(bullets_active, 0, 1000 * sizeof(int));
			enemy_spawn_freq = 1000;
			bullet_spawn_freq = 200;
			event1_freq = 10000;
			event2_freq = 5000;
			event3_freq = 5000;
			delete event1;
			delete event2;
			delete event3;
			event1 = NULL;
			event2 = NULL;
			event3 = NULL;
		}
		if (alive == 0)
		{
			SDL_Rect dead_rect = { screen_w/2-300, screen_h/2-200, 600, 400 };
			SDL_RenderCopyEx(renderer, dead_text, NULL, &dead_rect, 0, 0, SDL_FLIP_NONE);
			SDL_RenderPresent(renderer);
			continue;
		}
		// Move objects
		player->move(0.5);
		for (int i = 0; i < n_enemies; i++)
		{
			if (enemies_active[i] == 0) continue;
			enemies_list[i]->move(1);
		}
		for (int i = 0; i < n_bullets; i++)
		{
			if (bullets_active[i] == 0) continue;
			bullets_list[i]->move(1);
		}

		// Update objects' directions
		player->update_Direction();
		for (int i = 0; i < n_enemies; i++)
		{
			if (enemies_active[i] == 0) continue;
			enemies_list[i]->update_Direction(player);
		}
		for (int i = 0; i < n_bullets; i++)
		{
			if (bullets_active[i] == 0) continue;
			bullets_list[i]->update_angle();
		}

		// SPAWN ENEMIES AND BULLETS
		// Enemies
		unsigned int current_time = SDL_GetTicks();
		if (current_time - last_enemy_spawn > enemy_spawn_freq)
		{
			for (int i=0; i<n_enemies;i++)
			{
				if (enemies_active[i] == 0)
				{
					enemies_active[i] = 1;
					float spawn_x;
					float spawn_y;
					for (;;)
					{
						spawn_x = 30 + rand() % (screen_w - 60);
						spawn_y = 30 + rand() % (screen_h - 60);
						float d = (player->get_x() - spawn_x)*(player->get_x() - spawn_x) + (player->get_y() - spawn_y)*(player->get_y() - spawn_y);
						if (d > 40000) break;
					}
					enemies_list[i] = new Enemy(spawn_x, spawn_y , 0, 0, 40);
					last_enemy_spawn = current_time;
					break;
				}
			}
		}
		// Bullets
		int mx, my;
		if (SDL_GetMouseState(&mx, &my) && SDL_BUTTON(SDL_BUTTON_LEFT) && current_time - last_bullet_spawn > bullet_spawn_freq)
		{
			for (int i = 0; i < n_bullets; i++)
			{
				if (bullets_active[i] == 0)
				{
					bullets_active[i] = 1;
					float player_x = player->get_x();
					float player_y = player->get_y();

					float dx = mx - player_x;
					float dy = my - player_y;

					float magnitude = sqrt(dx * dx + dy * dy);

					float vx = dx / magnitude;
					float vy = dy / magnitude;
			
					float spawn_x = player_x + vx * player->get_size() *1.2;
					float spawn_y = player_y + vy * player->get_size() *1.2;
					if (spawn_x < 0) spawn_x += screen_w;
					else if (spawn_x > screen_w) spawn_x -= screen_w;
					if (spawn_y < 0) spawn_y += screen_h;
					else if (spawn_y > screen_h) spawn_y += screen_h;
					bullets_list[i] = new Bullet(spawn_x, spawn_y, vx*0.6, vy*0.6, 20);
					last_bullet_spawn = current_time;
					break;
				}
			}
		}
		// Special event 1
		if (event1 == NULL && current_time - last_event1_taken > event1_freq)
		{
			float percent = (float)rand() / RAND_MAX;
			if (percent < 0.001)
			{
				float spawn_x = rand() % screen_w;
				float spawn_y = rand() % screen_h;
				event1 = new Object(spawn_x, spawn_y, 0, 0, 50);
			}
		}
		// Special event 2
		if (event2 == NULL && current_time - last_event2_taken > event2_freq)
		{
			float percent = (float)rand() / RAND_MAX;
			if (percent < 0.01)
			{
				float spawn_x = rand() % screen_w;
				float spawn_y = rand() % screen_h;
				event2 = new Object(spawn_x, spawn_y, 0, 0, 70);
			}
		}
		// Special event 3
		if (event3 == NULL && current_time - last_event3_taken > event3_freq)
		{
			float percent = (float)rand() / RAND_MAX;
			if (percent < 0.01)
			{
				float spawn_x = rand() % screen_w;
				float spawn_y = rand() % screen_h;
				event3 = new Object(spawn_x, spawn_y, 0, 0, 50);
			}
		}

		// Draw
		SDL_RenderCopyEx(renderer, background_text, NULL, NULL, NULL, NULL, SDL_FLIP_NONE);

		int start = screen_w - 100;
		showScore(score, scoresheet_text, renderer,start);
		SDL_Rect level_rect = { 10, 30, 150, 100 };
		SDL_RenderCopyEx(renderer, level_text, NULL, &level_rect, 0, 0, SDL_FLIP_NONE);
		start = 150;
		showScore(level, scoresheet_text, renderer, start);

		player->draw(renderer, player_text);
		for (int i = 0; i < n_enemies; i++)
		{
			if (enemies_active[i]) enemies_list[i]->draw(renderer,enemy_text[enemies_list[i]->get_type()]);
		}
		for (int i = 0; i < n_bullets; i++)
		{
			if (bullets_active[i]) bullets_list[i]->draw(renderer, bullet_text);
		}
		if (event1 != NULL) event1->draw(renderer, event1_text);
		if (event2 != NULL) event2->draw(renderer, event2_text);
		if (event3 != NULL) event3->draw(renderer, event3_text);
		SDL_RenderPresent(renderer);


		// COLLISION CHECK
		if (vulnerable)
		{
			if (Collision::player_Bullets_Collision_Check(bullets_list, bullets_active, n_bullets, player)) alive = 0;
			if (Collision::player_Enemies_Collision_Check(enemies_list, enemies_active, n_enemies, player)) alive = 0;
		}
		Collision::bullets_Enemies_Collision_Check(bullets_list, bullets_active, n_bullets, enemies_list, enemies_active, n_enemies);
		if (event1 != NULL && player->collide(event1))
		{
			handleEvent1(enemies_active, n_enemies, bullets_active, n_bullets);
			delete event1;
			event1 = NULL;
			last_event1_taken = current_time;
		}
		if (event2 != NULL && player->collide(event2))
		{
			bullet_spawn_freq = 20;
			delete event2;
			event2 = NULL;
			last_event2_taken = current_time;
		}
		if (event3 != NULL && player->collide(event3))
		{
			vulnerable = 0;
			delete event3;
			event3 = NULL;
			last_event3_taken = current_time;
		}

		// Handle leftover - reverse the stats & game state to original after special events end
		if (current_time - last_event2_taken > 5000) bullet_spawn_freq = 200;
		if (current_time - last_event3_taken > 3000) vulnerable = 1;

		// Level up
		if (score >= last_level_up*2)
		{		
			level++;
			enemy_spawn_freq /= 1.5;
			last_level_up = score;
		}
	}
	getchar();

	return 0;
}