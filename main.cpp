#include "Game.h"

#include <grappix/grappix.h>
#include <coreutils/vec.h>
#include <cstdlib>
#include <memory>

using namespace grappix;
using namespace utils;
using namespace std;

int main(int argc, char **argv) {
	screen.open(450, 800, false);
	auto game = make_shared<Game>(screen);
	game->start();
	screen.render_loop([=](uint32_t delta) mutable {
		game->update(delta);
		screen.clear(0xff444488);
		game->render();
		screen.flip();
	});

	return 0;
}