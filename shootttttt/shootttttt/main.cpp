#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "SDL2-2.0.8\include\SDL.h"
#include "SDL2-2.0.8\include\SDL_image.h"

//load libraries
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2main.lib")
#pragma comment(lib,"SDL2-2.0.8\\lib\\x86\\SDL2_image.lib")

#pragma comment(linker,"/subsystem:console")




SDL_Renderer *renderer = NULL;
int screen_w = 1280;
int screen_h = 800;
float dt = 0.05;
#define R() (1.0-2.0*rand()/RAND_MAX)


struct ObjectList
{
	float* x;
	float* y;
	float* dir_x;
	float* dir_y;
	int* active;
	int num_of_objects;
};

void initialize(ObjectList* bullet_list)
{
	bullet_list->num_of_objects = 500;
	bullet_list->x = (float*)malloc(sizeof(float) * 500);
	bullet_list->y = (float*)malloc(sizeof(float) * 500);
	bullet_list->dir_x = (float*)malloc(sizeof(float) * 500);
	bullet_list->dir_y = (float*)malloc(sizeof(float) * 500);
	bullet_list->active = (int*)calloc(500, sizeof(int));
}


int next_Available(ObjectList* object_list)
{
	for (int i = 0; i < object_list->num_of_objects; i++)
	{
		if (object_list->active[i] == 0) return i;
	}
	return -1;
}


void update_Movement(float* actor_x, float* actor_y, int* go)
{
	float v = 1.0;
	if ((go[left] || go[right]) && (go[up] || go[down])) v=0.7;
	if (go[left]) (*actor_x)-=v*dt;
	if (go[up]) (*actor_y)-=v*dt;
	if (go[right]) (*actor_x)+=v*dt;
	if (go[down]) (*actor_y) +=v*dt;
	if (*actor_x> screen_w) *actor_x -= screen_w;
	if (*actor_y > screen_h) *actor_y -= screen_h;
	if (*actor_x < 0) *actor_x += screen_w;
	if (*actor_y < 0) *actor_y += screen_h;
}

void spawn_Bullet(float actor_x, float actor_y, float dir_x, float dir_y, ObjectList* bullet_list)
{
	int t = next_Available(bullet_list);
	bullet_list->x[t] = actor_x + 30*dir_x;
	bullet_list->y[t] = actor_y + 30*dir_y;
	bullet_list->dir_x[t] = dir_x;
	bullet_list->dir_y[t] = dir_y;
	bullet_list->active[t] = 1;
}

void update_Bullet_Movement(ObjectList* bullet_list)
{
	for (int i = 0; i < bullet_list->num_of_objects; i++)
	{
		bullet_list->x[i] += (bullet_list->dir_x[i])*1.5*dt;
		bullet_list->y[i] += (bullet_list->dir_y[i])*1.5*dt;
		if (bullet_list->x[i] > screen_w) bullet_list->x[i] -= screen_w;
		if (bullet_list->y[i] > screen_h) bullet_list->y[i] -= screen_h;
		if (bullet_list->x[i] < 0) bullet_list->x[i] += screen_w;
		if (bullet_list->y[i] < 0) bullet_list->y[i] += screen_h;
	}
}

void get_Normalized_Direc(float start_x, float start_y, float target_x, float target_y, float* dir_x, float* dir_y)
{
	//step 1 - find the distance vector between the source and the target
	float dist_x = target_x - start_x;
	float dist_y = target_y - start_y;
	//step 2 - calculate the unit vector - unit vector points in the direction of the target and its magnitude is exactly 1
	float magnitude = sqrt(dist_x * dist_x + dist_y * dist_y);
	(*dir_x) = dist_x / magnitude;
	(*dir_y) = dist_y / magnitude;
}

void draw_Objects(ObjectList* bullet_list, float size, int red, int green, int blue)
{
	SDL_SetRenderDrawColor(renderer, red, green, blue, 0);
	SDL_Rect tmp = { 0, 0, size, size };
	for (int i = 0; i < bullet_list->num_of_objects; i++)
	{
		if (bullet_list->active[i] == 0) continue;
		tmp.x = bullet_list->x[i];
		tmp.y = bullet_list->y[i];
		SDL_RenderFillRect(renderer, &tmp);
	}
}

void update_Enemy_Direc(ObjectList* enemy_list, float actor_x, float actor_y)
{
	for (int i = 0; i < enemy_list->num_of_objects; i++)
	{
		float dir_x = 0, dir_y = 0;
		get_Normalized_Direc(enemy_list->x[i], enemy_list->y[i], actor_x, actor_y, &dir_x, &dir_y);
		enemy_list->dir_x[i] = dir_x / 2;
		enemy_list->dir_y[i] = dir_y / 2;
	}
}

void collision_check(ObjectList* bullet_list, ObjectList* enemy_list)
{
	for (int i = 0; i < bullet_list->num_of_objects; i++)
	{
		if (bullet_list->active[i] == 0) continue;
		for (int j = 0; j < enemy_list->num_of_objects; j++)
		{
			if (enemy_list->active[j] == 0) continue;
			if (bullet_list->x[i] > enemy_list->x[j] + 20) continue;
			if (bullet_list->y[i] > enemy_list->y[j] + 20) continue;
			if (bullet_list->x[i] + 8 < enemy_list->x[j]) continue;
			if (bullet_list->y[i] + 8 < enemy_list->y[j]) continue;
			enemy_list->active[j] = 0;
			bullet_list->active[i] = 0;
			break;
		}
	}
}

int alive_check(ObjectList* object_list, float actor_x, float actor_y, float size)
{
	for (int i = 0; i < object_list->num_of_objects; i++)
	{
		if (object_list->active[i] == 0) continue;
		if (actor_x + 15 < object_list->x[i]) continue;
		if (actor_x > object_list->x[i] + size) continue;
		if (actor_y + 15 < object_list->y[i]) continue;
		if (actor_y > object_list->y[i] + size) continue;
		return 0;
	}
	return 1;
}


int main(int argc, char **argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *window = SDL_CreateWindow("Mark is being chased - but he has a gun...", 50, 50, screen_w, screen_h, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	srand(time(0));

	ObjectList bullet_list;
	initialize(&bullet_list);


	ObjectList enemy_list;
	initialize(&enemy_list);

	

	float actor_x = 0.0;
	float actor_y = 0.0;
	float actor_speed = 0.1;
	int go[4] = { left, up, right, down };

	int last_enemy_spawn = SDL_GetTicks();
	int last_bullet_spawn = SDL_GetTicks();

	for (;;)
	{
		memset(go, 0, sizeof(int) * 4);
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) return 0;
		}
		const Uint8* state = SDL_GetKeyboardState(NULL);
		
		// Move player
		if (state[SDL_SCANCODE_W]) go[up] = 1;
		if (state[SDL_SCANCODE_A]) go[left] = 1;
		if (state[SDL_SCANCODE_S]) go[down] = 1;
		if (state[SDL_SCANCODE_D]) go[right] = 1;

		update_Movement(&actor_x, &actor_y, go);
		unsigned int current_time = SDL_GetTicks();
		int mx, my;


		// Update
		update_Enemy_Direc(&enemy_list, actor_x, actor_y);
		collision_check(&bullet_list, &enemy_list);
		if (alive_check(&bullet_list, actor_x, actor_y, 8) == 0) break;
		if (alive_check(&enemy_list, actor_x, actor_y, 20) == 0) break;


		// Move Bullets
		update_Bullet_Movement(&bullet_list);
		update_Bullet_Movement(&enemy_list);

		// SPAWN
		current_time = SDL_GetTicks();

		// Player shoots -> spawn bullets
		if ((SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) && (current_time - last_bullet_spawn > 300))
		{
			float dir_x = 0.0, dir_y = 0.0;
			get_Normalized_Direc(actor_x, actor_y, mx, my, &dir_x, &dir_y);
			spawn_Bullet(actor_x, actor_y, dir_x, dir_y, &bullet_list);
			last_bullet_spawn = current_time;
		}

		// Spawn enemies
		if ((current_time - last_enemy_spawn > 500))
		{
			for (;;)
			{
				float x = rand() % screen_w;
				float y = rand() % screen_h;
				if (x + 20 < actor_x || x > actor_x + 15 || y + 20 < actor_y || y > actor_y + 15) int t = 1;
				else continue;
				float dir_x = 0.0, dir_y = 0.0;
				spawn_Bullet(x, y, 0, 0, &enemy_list);
				last_enemy_spawn = current_time;
				break;
			}
		}
		
		// Draw
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 0);
		SDL_Rect rect = { actor_x, actor_y, 15,15 };
		SDL_RenderFillRect(renderer, &rect);

		draw_Objects(&bullet_list, 8, 255, 255, 255);
		draw_Objects(&enemy_list , 20, 255, 0, 0);

		SDL_RenderPresent(renderer);
	}

	getchar();

	return 0;
}