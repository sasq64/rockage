#include "Game.h"

#include <coreutils/vec.h>
#include <lua/luainterpreter.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <flatland/glutil.h>
#include <flatland/node.h>
#include <flatland/container.h>

using namespace grappix;
using namespace utils;
using namespace flatland;


void Game::render() {

	static float r = 0;
	gamelayer.render(render_target, x_offset);

	//auto dx = ship_pos - (ship_pos%tile_size) + x_offset;
	//render_target.dashed_line(dx, 0, dx, render_target.height(), 0xff888888);
	//render_target.dashed_line(dx+tile_size, 0, dx+tile_size, render_target.height(), 0xff888888);

	spritelayer.render(render_target, x_offset);

	render_target.rectangle(0, pheight, pwidth, render_target.height()-pheight, 0xff222255);

	render_target.text("SCORE:", 5, pheight+5, 0xff8888ff, 1.2);
	render_target.text(format("%08d", score), 5, pheight+20, 0xffffffff, 2.5);

	if(game_over) {
		render_target.text("GAME OVER", 0, pheight/2, 0xffffffff, 4.0);
	}

	//auto p = Shape::createRectangle(200,200).roundCorners(4, 34.0);
	auto p = Shape::createText("Hello people").scale(2.0).translate(-400,0);
	//p = p.roundCorners(4, 2.0);
	//p = p.rotate(100.5);
	//p.render(context);
	//render_target.render(p);

	//Node node;
	//Container<Shape> container(p);
	//node.add(&container);
	//node.render(context);

	//Shape cardShape;
	//cardShape = flatland::Shape::createRectangle(40,70).roundCorners(4, 2.0);
	//cardShape = cardShape.makeSolid().setColor(flatland::Colors::WHITE).concat(cardShape.setColor(flatland::Colors::BLACK));

	//cardShape.render(context);
	//root.setRotation(r += 0.1);
	//root.rotation() += 0.5;

	root.render(context);

	//auto c = Shape::createCircle(50);
	//c.render(tempContext);

	//cardShape.render(tempContext);
}

void Game::start() {
	ship_pos = (width * tile_size)/2;
	target_pos = render_target.width()/2;
	sprite->y = visible_height*tile_size - tile_size;
	gamelayer.scrolly = (level_height - visible_height) * tile_size;

	memset(&playfield[0], 0, playfield.size());

	gen_counter = 0;
	blockno = 2;
	remove_delay = 0;
	difficulty = 0;
	game_over = false;
	score = 0;

}


void Game::check_rectangle(unsigned int pos) {

	auto p = playfield[pos];
	auto l = pos;
	auto r = pos;
	//auto h = width;
	auto yl = pos / width;

	// Find upper edge
	while(playfield[pos-width] == p) pos -= width;

	auto ul = pos;
	auto ur = pos;
	while(playfield[ul-1] == p) ul--;
	while(playfield[ur+1] == p) ur++;

	ul = ul % width;
	ur = ur % width;

	auto yu = pos / width;

	// Find extent left to right
	while(playfield[l-1] == p && playfield[l-1+width] != p) l--;
	while(playfield[r+1] == p && playfield[r+1+width] != p) r++;

	l = l % width;
	r = r % width;

	if(l == ul && r == ur) {
		LOGD("BOX!");
		rec_bonus = ((yl-yu-1) + (r-l-1)) * 100;
		for(unsigned int y=yu; y<=yl; y++)
			for(unsigned int x=l; x<=r; x++)
				remove_tiles.push_back(x+y*width);
				//playfield[x+y*width] = 0;
	}
};

void Game::fire_block() {

	auto tile_pos = ship_pos / tile_size;

	// offset to top and bottom tile position for shot
	auto offset = tile_pos + ((int)gamelayer.scrolly / tile_size) * width;
	auto doffset = offset + width * (visible_height-1);


	int target_index = -1;
	int block_hit = 1;
	float target_y = 0;

	int i = doffset;
	while(i >= offset && playfield[i] == 0) {
		i -= width;
	}

	target_index = i+width;
	playfield[target_index] = 1;
	auto ty = target_index / width;
	target_y = ty * tile_size - gamelayer.scrolly + 16;

	// Skip temporary tiles when finding the blockno we hit
	while(i >= offset && block_hit == 1) {
		block_hit = playfield[i];
		i -= width;
	}

	auto s = spritelayer.addSprite(31, tile_pos * tile_size, render_target.height() - tile_size*2);

	tween::make_tween().linear().to(s->y, target_y).seconds((s->y - target_y) * 0.001).on_complete([=]() mutable {
		s = 0;
		//LOGD("ARRIVED %d (%d)", target_index, block_hit);
		if(remove_tiles.size() > 0 && playfield[remove_tiles.front()] == block_hit) {
			playfield[target_index] = 0;
		} else {
			if(block_hit >= 1) {
				playfield[target_index] = block_hit;
				check_rectangle(target_index);
			}
		}
	});

}

void Game::update(uint32_t delta) {

	if(game_over) {
		return;
	}

	static int block_sizes[] = { 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7 };

	int freq = 100 - difficulty*2;

	auto offset = ((int)gamelayer.scrolly / tile_size) * width;
	//LOGD("%d", offset);
	auto doffset = offset + width * (visible_height-1);
	for(unsigned int i=doffset; i<doffset+width; i++) {
		if(playfield[i] != 0)
			game_over = true;
	}

	if(gen_counter == 0) {
		int s = 8;
		if(difficulty < (int)sizeof(block_sizes))
			s =  block_sizes[difficulty];
		int w = (rand() % s) + 2;
		int h = (rand() % (s-1)) + 1;
		generate_block(offset - 1 - ((rand() % 3)+h) * width + (rand() % (width-w)) + 1, w, h);
		gen_counter = (rand() % freq) + freq;
		if(blockno % 10 == 0)
			difficulty++;
	} else
		gen_counter--;

	auto quick = (screen.key_pressed(Window::DOWN));

	unsigned int mx, my;
#ifdef ANDROID
	mx = my = 0;
#else
	tie(mx,my) = screen.mouse_position();
	if(screen.mouse_pressed()) {
		if(my > visible_height *tile_size)
			quick = true;
	}
#endif

	auto key = screen.get_key();
	switch(key) {
	case 'Z':
		fire_block();
		break;
	case Window::LEFT:
		ship_pos -= tile_size;
		break;
	case Window::RIGHT:
		ship_pos += tile_size;
		break;
	default:
		break;
	}

	auto c = screen.get_click();
	if(c != Window::NO_CLICK && my < tile_size * visible_height) {
		int x = (c.x - x_offset)/tile_size;
		ship_pos = x*tile_size;
		fire_block();
	}

	float downspeed = (float)tile_size/128;

	int d = difficulty;
	while(d >= 10) {
		downspeed *= 2;
		d -= 10;
	}


	if(remove_tiles.size()) {
		if(remove_delay == 0) {
			auto o = remove_tiles.front();
			remove_tiles.pop_front();
			playfield[o] = 0;
			remove_delay = 2;
			score += rec_bonus;
		} else
			remove_delay--;
	} else {
		gamelayer.scrolly-= ( quick ? downspeed*8 : downspeed);
		if(quick)
			score += 10;
	}

	sprite->x = ship_pos;
}

bool Game::generate_block(int pos, int w, int h) {

	int o = ((rand() % 2) ? width : width+w-1);

	for(int x=0;x<w; x++)
		if(playfield[pos+x])
			return false;

	for(int y=0;y<h; y++)
		if(playfield[o+pos+y*width])
			return false;

	for(int x=0;x<w; x++)
		playfield[pos+x] = blockno;

	for(int y=0;y<h; y++)
		playfield[o+pos+y*width] = blockno;

	blockno++;

	return true;
};

void Game::create_tiles() {

	auto sz = tile_size;
	float radius = sz/2;
	Texture tile { sz, sz };

	vec2f center { tile.width() / 2.0f, tile.height() / 2.0f };

	blocks.add_solid(0xff000000);

	tile.clear();
	tile.circle(center, radius, 0x000020); // Outline
	tile.circle(center, radius*0.90, 0x0000C0); // Main ball
	tile.circle(center + vec2f{radius*0.15f, -radius*0.15f}, radius * 0.6, 0x0040ff); // Hilight
	blocks.add_tiles(tile.get_pixels());

	tile.clear();
	tile.line(sz/2,0, sz, sz, 0xff00ff00);
	tile.line(sz,sz, 0, sz, 0xff00ff00);
	tile.line(0, sz, sz/2,0, 0xff00ff00);
	blocks.add_tiles(tile.get_pixels());

	uint32_t LIGHT = 0xffcccccc;
	uint32_t GRAY = 0xffaaaaaa;
	uint32_t DARK = 0xff888888;
	uint32_t BLACK = 0;//0xffff0000;

	// UP, RIGHT, DOWN, LEFT
	blocks.set_tile(16);

	rectf clip;

	auto rec = [&](int x0, int y0, int w, int h, uint32_t col) {
		auto x1 = x0 + w;
		auto y1 = y0 + h;
		if(x0 < clip.x0) x0 = clip.x0;
		if(y0 < clip.y0) y0 = clip.y0;
		if(x1 > clip.x1) x1 = clip.x1;
		if(y1 > clip.y1) y1 = clip.y1;
		tile.rectangle(x0, y0, x1-x0, y1-y0, col);
	};

	for(int i=0; i<16; i++) {
		clip = {0, 0, (float)sz, (float)sz};

		tile.clear();
		rec(0,0,sz,sz,GRAY);
		rec(sz*0.3, sz*0.3, sz*0.4, sz*0.4 ,0xff444444);

		if(i & 1) {
			rec(0,0,sz,4,BLACK);
			clip.y0 = 2;
			rec(0,2,sz,2,LIGHT);
		}
		if(i & 2) {
			rec(sz-4, 0, 4, sz, BLACK);
			clip.x1 = sz-2;
			rec(sz-4, 0, 2, sz, DARK);
		}
		if(i & 4) {
			rec(0, sz-4, sz, 4, BLACK);
			clip.y1 = sz-2;
			rec(0, sz-4, sz, 2, DARK);
		}
		if(i & 8) {
			rec(0, 0, 4, sz, BLACK);
			clip.x0 = 2;
			rec(2, 0, 2, sz, LIGHT);
		}
		blocks.add_tiles(tile.get_pixels());
	}

	//auto bm = blocks.texture.get_pixels();
	//save_png(bm, "tiles.png");

}