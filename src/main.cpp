#include "las_file/las_file.hpp"
#include "laser/laser.hpp"
#include "lvx_file/lvx_file.hpp"
#include "objreader/simple_obj_reader.hpp"
#include "pcd_file/pcd_file.hpp"
#include "vkgui/vkgui.hpp"
#include "vkmesh/vkmesh.hpp"
#include "vkrun/vkrun.hpp"
#include <thread>

int main(int argc, char *argv[])
{

	std::vector<Vertex> point_vertex;
	std::vector<uint32_t> point_idx;

	auto context = std::shared_ptr<HVKContext>(new HVKContext);
	context->init();

	auto camera = std::shared_ptr<HVKCamera>(new HVKCamera);
	camera->setContext(context);
	camera->init();

	auto gui = std::shared_ptr<HVKGUI>(new HVKGUI);
	gui->setContext(context, camera);
	gui->init();

	HVKApp Llidar;
	// read_lvx_file("misc/L.lvx", point_vertex, point_idx);
	// read_lvx_file(argv[1], point_vertex, point_idx);
	// read_pcd_file(argv[1], point_vertex, point_idx);
	read_las_file(argv[1], point_vertex, point_idx);
	Llidar.setIndexedVertex(point_vertex, point_idx);
	Llidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
	// save_pcd_file("misc/savepcd.pcd", point_vertex, point_idx);

	try
	{
		Llidar.setContext(context, camera, gui);
		Llidar.init();

		while (!context->isClosed())
		{
			context->mainLoopBegin();
			context->drawStart();
			camera->updateCamObjectBuffers(context->getCurrentFrame());

			Llidar.drawFrame();

			gui->drawFrame();

			context->drawEnd();
		}
		context->mainLoopExit();

		Llidar.deinit();

		gui->deinit();
		camera->deinit();
		context->deinit();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
