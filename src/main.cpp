#include "las_file/las_file.hpp"
#include "laser/laser.hpp"
#include "lvx_file/lvx_file.hpp"
#include "objreader/simple_obj_reader.hpp"
#include "pcd_file/pcd_file.hpp"
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

	std::vector<Vertex> point_vertex;
	std::vector<uint32_t> point_idx;

	std::thread fraps(fraps_main);

	auto context = std::shared_ptr<HVKContext>(new HVKContext);
	context->init();

	auto camera = std::shared_ptr<HVKCamera>(new HVKCamera);
	camera->setContext(context);
	camera->init();

	auto gui = std::shared_ptr<HVKGUI>(new HVKGUI);
	gui->setContext(context);
	gui->setCamera(camera);
	gui->init();

	HVKApp Llidar;
	Llidar.setBase(camera, gui);
	// read_lvx_file("misc/L.lvx", point_vertex, point_idx);
	// read_lvx_file(argv[1], point_vertex, point_idx);
	read_pcd_file(argv[1], point_vertex, point_idx);
	// std::vector<Vertex> ptv;
	// point_idx.clear();
	// for (auto &pt : point_vertex)
	// {
	// 	if (glm::length(pt.pos) > 3.0f)
	// 	{
	// 		point_idx.push_back(ptv.size());
	// 		ptv.push_back(pt);
	// 	}
	// }
	// point_vertex = ptv;
	Llidar.setIndexedVertex(point_vertex, point_idx);

	// HVKApp Rlidar;
	// Rlidar.setCamera(camera);
	// read_lvx_file("misc/R.lvx", point_vertex, point_idx);
	// for (auto &vv : point_vertex)
	// {
	// 	vv.pos.z += 0.3082;
	// 	// vv.color.r = 1.0;
	// 	vv.pos.x -= 0.005;
	// }
	// Rlidar.setIndexedVertex(point_vertex, point_idx);

	// HVKMesh viking;
	// // SimpleOBJReader sor("model/viking_room.obj");
	// SimpleOBJReader sor("model/crane.obj");
	// sor.getIndexedVertex(point_vertex, point_idx);
	// std::cout << point_vertex.size() << " " << point_idx.size() << " viking\n";
	// viking.setIndexedVertex(point_vertex, point_idx);
	// viking.setMeshTexturePath("model.crane.obj", "model/viking_room.png");

	// HVKMesh iroha;
	// SimpleOBJReader sor("model/CH0156/Model/CH0156_Tank.obj");
	// sor.getIndexedVertex(point_vertex, point_idx);
	// iroha.setIndexedVertex(point_vertex, point_idx);
	// iroha.setMeshTexturePath("model/CH0156/Model/CH0156_Tank.obj", "model/CH0156/Model/Texture/CH0156_Tank_01.png");

	try
	{
		Llidar.setContext(context);
		Llidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
		Llidar.init();

		// iroha.setContext(context);
		// iroha.setCamera(camera);
		// iroha.init();

		// Rlidar.setContext(context);
		// Rlidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
		// Rlidar.init();

		// viking.setContext(context);
		// viking.init();

		while (!context->isClosed())
		{
			context->mainLoopBegin();
			context->drawStart();
			camera->updateCamObjectBuffers(context->getCurrentFrame());

			Llidar.drawFrame();
			// iroha.drawFrame();
			// Rlidar.drawFrame();
			// viking.drawFrame();

			gui->drawFrame();

			context->drawEnd();
			frame_count++;
		}
		context->mainLoopExit();

		Llidar.deinit();
		// iroha.deinit();
		// Rlidar.deinit();
		// viking.deinit();
		gui->deinit();
		camera->deinit();
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
