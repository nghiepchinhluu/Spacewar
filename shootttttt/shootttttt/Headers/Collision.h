#pragma once

namespace Collision
{
	void bullets_Enemies_Collision_Check(Bullet** bullets_list, int* bullets_active, int n_bullets, Enemy** enemies_list, int* enemies_active, int n_enemies)
	{
		for (int i = 0; i < n_bullets; i++)
		{
			if (bullets_active[i] == 0) continue;
			for (int j = 0; j < n_enemies; j++)
			{
				if (enemies_active[j] == 0) continue;
				if (bullets_list[i]->collide(enemies_list[j]))
				{
					bullets_active[i] = 0;
					enemies_active[j] = 0;
					score++;
					break;
				}
			}
		}
	}

	int player_Bullets_Collision_Check(Bullet** bullets_list, int* bullets_active, int n_bullets, Player* player)
	{
		for (int i = 0; i < n_bullets; i++)
		{
			if (bullets_active[i] == 0) continue;
			if (player->collide(bullets_list[i])) return 1;
		}
		return 0;
	}

	int player_Enemies_Collision_Check(Enemy** enemies_list, int* enemies_active, int n_enemies, Player* player)
	{
		for (int i = 0; i < n_enemies; i++)
		{
			if (enemies_active[i] == 0) continue;
			if (player->collide(enemies_list[i])) return 1;
		}
		return 0;
	}
}