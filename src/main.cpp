#include "keyinput/keyinput.hpp"
#include "las_file/las_file.hpp"
#include "laser/laser.hpp"
#include "lvx_file/lvx_file.hpp"
#include "vkrun/vkrun.hpp"
#include <thread>

static bool onExit = false;
bool run = false;
uint32_t frame_count = 0;

void keyinput_run()
{
	run = true;
	return;
}

void fraps_main()
{
	auto tt = std::chrono::steady_clock::now();
	uint32_t prev_count = 0, cur_count = 0;

	frame_count = 0;
	while (!onExit)
	{
		cur_count = frame_count;
		std::cout << cur_count - prev_count << "\n";
		prev_count = cur_count;
		tt += std::chrono::seconds(1);
		std::this_thread::sleep_until(tt);
	}
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "File name\n";
		return 0;
	}

	std::vector<Vertex> point_vertex;
	std::vector<uint32_t> point_idx;
	read_lvx_file(argv[1], point_vertex, point_idx);

	auto phyDev = std::shared_ptr<HVKPhyDev>(new HVKPhyDev);
	phyDev->init();
	HVKApp app;
	std::thread fraps(fraps_main);

	try
	{
		app.setPhyDev(phyDev);
		app.initVulkan();

		while (!phyDev->isClosed())
		{
			phyDev->mainLoopBegin();
			app.drawFrame();
			frame_count++;
		}
		phyDev->mainLoopExit();

		app.cleanup();
		phyDev->deinit();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	onExit = true;
	fraps.join();
	return EXIT_SUCCESS;
}
