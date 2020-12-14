/*
 * RayCastingWindow.cc
 *
 *  Created on: 24 окт. 2020 г.
 *      Author: unyuu
 */
#include <iostream>
#include <cmath>
#include "RayCastingWindow.h"
#include <cassert>



RayCastingWindow::RayCastingWindow(int width, int height)
: Window(width, height)
{
	_map = std::make_shared<Map>("map01.txt");
	_player = std::make_shared<Player>(_map);
	assert(_renderer != nullptr);
	_wall_texture = std::make_shared<Texture>(_renderer, "wall.jpg");
}

void RayCastingWindow::render()
{
	SDL_Rect r_sky { 0, 0, width(), height() / 2 };
	SDL_Rect r_floor { 0, height() / 2, width(), height() / 2 };
	SDL_SetRenderDrawColor(_renderer.get(), 64, 128, 192, 255);
	SDL_RenderFillRect(_renderer.get(), &r_sky);
	SDL_SetRenderDrawColor(_renderer.get(), 0, 128, 0, 255);
	SDL_RenderFillRect(_renderer.get(), &r_floor);

	SDL_SetRenderDrawBlendMode(_renderer.get(), SDL_BLENDMODE_BLEND);

	// Рисование стен (с использованием алгоритма бросания лучей)

	constexpr double H = 0.5, e = 0.01, FOV = Pi/3;
	double gamma, beta, dx, dy, rx, ry, D, dH, dV, tx, txH, txV;
	int h;
	double sd = width()/(2 * tan (FOV/2));
	for (int col = 0; col < width(); ++col) {
		// Здесь будет алгоритм
		gamma = atan((col - width()/2) / sd);
			beta = _player->dir() + gamma;
				if (sin(beta) > e)
				{
					dy = 1;
					ry = floor(_player->y()) + e;
					dx = 1 / tan(beta);
					rx = _player->x() - (_player->y() - ry) * dx;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					dH = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else if (sin(beta) < -e)
				{
					dy = -1;
					ry = ceil(_player->y()) - e;
					dx = 1 / tan(-beta);
					rx = _player->x() - (ry - _player->y()) * dx;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					dH = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else
				{
					dH = INFINITY;
				}
				txH = rx - floor(rx);

				if(cos(beta) > e){
					dx = 1;
					rx = floor(_player->x()) + e;
					dy = tan(beta);
					ry = _player->y() - (_player->x() - rx) * dy;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					dV = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else if(cos(beta) < -e){
					dx = -1;
					rx = ceil(_player->x()) - e;
					dy = tan(-beta);
					ry = _player->y() - (rx - _player->x()) * dy;
					do{
						rx += dx;
						ry += dy;
					}while (not _map->wall(rx, ry));
					dV = sqrt((rx - _player->x()) * (rx - _player->x()) + (ry - _player->y()) * (ry - _player->y()));

				}
				else{
					dV = INFINITY;
				}
				txV = ry - floor(ry);

				if (dH < dV){
					D = dH;
					tx = txH;
				}

				else{
					D = dV;
					tx = txV;
				}
				h = int((sd * H)/D / cos(gamma)) ;

				if (_textured){
					draw_textured_col(col, h, tx);
				}
				else{
					//видимая высота стены на экране
					draw_col(col, h);
				}
				SDL_SetRenderDrawColor(_renderer.get(), 0, 0, 0, 255);
				SDL_RenderDrawLine(_renderer.get(), _player->x(), _player->y(), rx, ry);
	}

	// Рисование карты

	SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 255, 64);
	for (int y = 0; y < _map->height(); ++y)
		for (int x = 0; x < _map->width(); ++x) {
			SDL_Rect r { x * 50, y * 50, 50, 50 };
			if (_map->wall(x, y))
				SDL_RenderFillRect(_renderer.get(), &r);
		}

	SDL_Rect r_player {
		int(_player->x() * 50)-5,
		int(_player->y() * 50)-5,
		10, 10
	};
	SDL_Rect r_player_eye {
		int(_player->x() * 50 + 10*cos(_player->dir()))-2,
		int(_player->y() * 50 + 10*sin(_player->dir()))-2,
		4, 4
	};

	SDL_SetRenderDrawColor(_renderer.get(), 255, 64, 64, 255);
	SDL_RenderFillRect(_renderer.get(), &r_player);
	SDL_SetRenderDrawColor(_renderer.get(), 255, 255, 0, 255);
	SDL_RenderFillRect(_renderer.get(), &r_player_eye);
}

void RayCastingWindow::draw_col(int col, int h)
{
	SDL_SetRenderDrawColor(_renderer.get(), 64, 64, 64, 255);
	int y1 = height() / 2 - h / 2;
	int y2 = height() / 2 + h / 2;
	SDL_RenderDrawLine(_renderer.get(), col, y1, col, y2);

}

void RayCastingWindow::handle_keys(const Uint8 *keys)
{
	if (keys[SDL_SCANCODE_W]) _player->walk_forward();
	if (keys[SDL_SCANCODE_S]) _player->walk_backward();
	if (keys[SDL_SCANCODE_D]) _player->shift_right();
	if (keys[SDL_SCANCODE_A]) _player->shift_left();
	if (keys[SDL_SCANCODE_E]) _player->turn_right();
	if (keys[SDL_SCANCODE_Q]) _player->turn_left();
}

void RayCastingWindow::draw_textured_col(int col, int h, double tx)
{
	SDL_Rect what { int(floor(_wall_texture->getWidth() * tx)),
		0, 1, _wall_texture->getHeight()};
	SDL_Rect where { col, height()/2 - h/2, 1, h };
	_wall_texture->draw(&what, &where);
}
