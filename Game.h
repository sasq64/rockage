#ifndef GAME_H
#define GAME_H

#include <grappix/grappix.h>
#include <deque>
#include <flatland/shape.h>
#include <flatland/node.h>
#include <flatland/container.h>

template <typename T> class CircularArray {
public:
	CircularArray(int size) : v(size) {}

	T& operator[](const int &i) {
		return v[i % v.size()];
	}

	T operator[](const int &i) const { 
		return v[i % v.size()];
	}

	size_t size() { return v.size(); }

private:
	std::vector<T> v;

};

class Game {
public:
	Game(const grappix::RenderTarget &target, int size = 48) : 
		width { 16 },
		level_height { 1024 },
		visible_height { 24 },
		tile_size { target.width() / width },
		render_target { target },
		blocks { tile_size, tile_size, 512, 512 },
		gamelayer { width, level_height, (width-1) * tile_size, visible_height * tile_size, blocks, [&](uint32_t x, uint32_t y) -> uint32_t {
			auto o = x + y*width;
			uint32_t p = playfield[o];
			if(p > 1) {
				return 16+((y > 0 && playfield[o-width] == p ? 0 : 1) | 
				       (x < width-1 && playfield[o+1] == p ? 0 : 2) | 
				       (y < level_height-1 && playfield[o+width] == p ? 0 : 4) | 
				       (x > 0 && playfield[o-1] == p ? 0 : 8)); 
			}
			return 0;
		} },
		spritelayer { blocks },
		playfield (width * visible_height * 2)
	{
		x_offset = (render_target.width() - width*tile_size)/2;
		pwidth = width * tile_size;
		pheight = visible_height * tile_size;

		auto &res = grappix::Resources::getInstance();
		res.register_image("tiles", [&](grappix::bitmap &bm) {
			create_tiles();
			bm = blocks.texture.get_pixels();
		});

		res.on_load("tiles", [&](const std::string &name, grappix::Resources &res) {
			blocks.set_image(res.get_image(name));
		});

		sprite = spritelayer.addSprite(2, 320, render_target.height() - tile_size);


		context.setTarget(render_target.width(), render_target.height(), render_target.buffer());

		auto cardShape = flatland::Shape::createRectangle(40,70).roundCorners(4, 2.0);
		cardShape = cardShape.makeSolid().setColor(flatland::Colors::WHITE).concat(cardShape.setColor(flatland::Colors::BLACK));

		auto p = flatland::Shape::createText("Hello people").scale(2.0).translate(-400,0);
		root.add(new flatland::Container<flatland::Shape>(cardShape));
		root.add(new flatland::Container<flatland::Shape>(p));

		//grappix::tween::make_tween().to(root.position(), glm::vec2(100,0)).seconds(1.0);
		grappix::tween::make_tween().to(root.rotation(), 180.0).seconds(1.0).on_complete([=]() mutable {
			grappix::tween::make_tween().to(root.position(), glm::vec2(100,0)).seconds(1.0);
		});

	}

	void start();
	void update(uint32_t delta);
	void render();

private:

	void fire_block();	
	void create_tiles();
	void check_rectangle(unsigned int pos);
	bool generate_block(int w, int h, int pos);

	uint32_t width;
	uint32_t level_height;
	uint32_t visible_height;
	uint32_t pwidth;
	uint32_t pheight;
	uint32_t tile_size;

	grappix::RenderTarget render_target;


	grappix::TileSet blocks;
	grappix::TileLayer gamelayer;
	grappix::SpriteLayer spritelayer;
	
	int x_offset;
	int ship_pos;
	int target_pos;
	int gen_counter;
	int blockno;
	int remove_delay;
	int score;
	int rec_bonus;
	int difficulty;

	bool game_over;

	std::shared_ptr<grappix::Sprite> sprite;

	CircularArray<uint8_t> playfield;
	std::deque<int> remove_tiles;

	//flatland::Shape cardShape;
	flatland::RenderContext context;
	flatland::Node root;
};

#endif // GAME_H
