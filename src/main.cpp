#include "keyinput/keyinput.hpp"
#include "las_file/las_file.hpp"
#include "laser/laser.hpp"
#include "lvx_file/lvx_file.hpp"
#include "objreader/simple_obj_reader.hpp"
#include "vkgui/vkgui.hpp"
#include "vkmesh/vkmesh.hpp"
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

	auto context = std::shared_ptr<HVKContext>(new HVKContext);
	context->init();
	HVKApp Llidar;
	// HVKGUI gui;

	read_lvx_file(argv[1], point_vertex, point_idx);
	point_idx.resize(240000 * 9);
	// read_lvx_file("misc/L.lvx", point_vertex, point_idx);
	Llidar.setIndexedVertex(point_vertex, point_idx);

	std::thread fraps(fraps_main);

	// HVKApp Rlidar;
	// read_lvx_file("misc/R.lvx", point_vertex, point_idx);
	// for (auto &vv : point_vertex)
	// {
	// 	vv.pos.z += 0.3082;
	// 	vv.color.r = 1.0;
	// 	vv.pos.x -= 0.005;
	// }
	// Rlidar.setIndexedVertex(point_vertex, point_idx);

	HVKMesh viking;
	SimpleOBJReader sor("model/viking_room.obj");
	sor.getIndexedVertex(point_vertex, point_idx);
	std::cout << point_vertex.size() << " " << point_idx.size() << " viking\n";
	viking.setIndexedVertex(point_vertex, point_idx);

	try
	{
		Llidar.setContext(context);
		Llidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
		Llidar.init();

		// Rlidar.setContext(context);
		// Rlidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
		// Rlidar.init();

		viking.setContext(context);
		viking.init();

		while (!context->isClosed())
		{
			context->mainLoopBegin();
			context->drawStart();

			Llidar.drawFrame();
			// Rlidar.drawFrame();
			// viking.drawFrame();

			context->drawEnd();
			frame_count++;
		}
		context->mainLoopExit();

		Llidar.deinit();
		// Rlidar.deinit();
		viking.deinit();
		context->deinit();
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
