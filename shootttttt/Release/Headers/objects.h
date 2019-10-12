#pragma once

enum Direction { LEFT, UP, RIGHT, DOWN};

class Object
{
protected:
	float x, y, vx, vy;
	float size;
	float angle;

public:

	Object()
	{

	}

	Object(float _x, float _y, float _vx, float _vy, float _size)
	{
		x = _x;
		y = _y;
		vx = _vx;
		vy = _vy;
		size = _size;
		angle = 0;
	}

	float get_x()
	{
		return x;
	}

	float get_y()
	{
		return y;
	}

	float get_size()
	{
		return size;
	}

	void update_angle()
	{
		if (vx == 0 && vy < 0) angle = 0;
		else if (vx > 0 && vy < 0) angle = 90 - atan(-vy / vx) * 180 / 3.14;
		else if (vx > 0 && vy == 0) angle = 90;
		else if (vx > 0 && vy > 0) angle = 90 + atan(vy / vx) * 180 / 3.14;
		else if (vx == 0 && vy > 0) angle = 180;
		else if (vx < 0 && vy >0) angle = 270 - atan(vy / -vx) * 180 / 3.14;
		else if (vx < 0 && vy == 0) angle = 270;
		else if (vx < 0 && vy < 0) angle = 270 + atan(vy / vx) * 180 / 3.14;
	}

	void move(float dt)
	{
		x += vx * dt;
		y += vy * dt;
		if (x > screen_w) x -= screen_w;
		else if (x < 0) x += screen_w;
		if (y > screen_h) y -= screen_h;
		else if (y < 0) y += screen_h;
	}

	bool collide(Object* other)
	{
		float _x = other->get_x();
		float _y = other->get_y();
		float _size = other->get_size();
		if (x - size/4 > _x + _size/2) return 0;
		if (x + size/4 < _x - _size/2) return 0;
		if (y - size/4 > _y + _size/2) return 0;
		if (y + size/5 < _y - _size/2) return 0;
		return 1;
	}

	void draw(SDL_Renderer* renderer, SDL_Texture *texture)
	{
		SDL_Rect rect = { x - size/2, y - size/2,size,size };
		SDL_RenderCopyEx(renderer, texture, NULL, &rect, angle, NULL, SDL_FLIP_NONE);
	}


	~Object()
	{

	}
};

class Player : public Object
{
public:
	Player(float _x, float _y, float _vx, float _vy, float _size)
	{
		x = _x;
		y = _y;
		vx = _vx;
		vy = _vy;
		size = _size;
	}

	void update_Direction()
	{
		// Get key state
		const Uint8* state = SDL_GetKeyboardState(NULL);
		int go[4] = { 0 };
		if (state[SDL_SCANCODE_W]) go[UP] = 1;
		if (state[SDL_SCANCODE_A]) go[LEFT] = 1;
		if (state[SDL_SCANCODE_S]) go[DOWN] = 1;
		if (state[SDL_SCANCODE_D]) go[RIGHT] = 1;
		
		// Update velocity
		float speed = 1.0;
		if ((go[LEFT] || go[RIGHT]) && (go[UP] || go[DOWN])) speed = 0.7; // For moving diagonally 

		vx = 0; vy = 0;
		if (go[LEFT]) vx -= speed;
		if (go[UP]) vy -= speed;
		if (go[RIGHT]) vx += speed;
		if (go[DOWN]) vy += speed;
		
		update_angle();
	}
};

class Enemy : public Object
{
	int type;
public:
	Enemy(float _x, float _y, float _vx, float _vy, float _size)
	{
		x = _x;
		y = _y;
		vx = _vx;
		vy = _vy;
		size = _size;
		type = rand() % 3;
	}

	int get_type()
	{
		return type;
	}


	void update_Direction(Player* player)
	{
		static int dir[9][2] = { {0,0}, {-screen_w,-screen_h}, {0,-screen_h}, {screen_w,-screen_h}, {-screen_w,0}, {screen_w,0}, {-screen_w,screen_h}
			, {0,screen_h}, {screen_w,screen_h} };

		float min = 99999999999;
		for (int i = 0; i < 9; i++)
		{
			float player_x = player->get_x() + dir[i][0];
			float player_y = player->get_y() + dir[i][1];

			float dx = player_x - x;
			float dy = player_y - y;

			float magnitude = sqrt(dx * dx + dy * dy);

			if (magnitude < min)
			{
				if (magnitude < 1)
				{
					vx = 0;
					vy = 0;
					break;
				}
				vx = dx / magnitude;
				vy = dy / magnitude;
				min = magnitude;
			}
		}
		vx *= 0.2;
		vy *= 0.2;

		update_angle();
	}
};

class Bullet : public Object
{
public:
	Bullet(float _x, float _y, float _vx, float _vy, float _size)
	{
		x = _x;
		y = _y;
		vx = _vx;
		vy = _vy;
		size = _size;
	}
};