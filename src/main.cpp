#include "laser_file/las_file/las_file.hpp"
#include "laser_file/lvx_csv/lvx_csv.hpp"
#include "laser_file/lvx_file/lvx_file.hpp"
#include "laser_file/pcd_file/pcd_file.hpp"
#include "laser_file/vz_txt/vz_txt.hpp"
#include "objreader/simple_obj_reader.hpp"
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

	auto i2color = [](uint8_t ref)
	{
		uint8_t r, g, b;
		if (ref < 30)
		{
			r = 0;
			g = int(ref * 255 / 30) & 0xff;
			b = 0xff;
		}
		else if (ref < 90)
		{
			r = 0;
			g = 0xff;
			b = int((90 - ref) * 255 / 60) & 0xff;
		}
		else if (ref < 150)
		{
			r = ((ref - 90) * 255 / 60) & 0xff;
			g = 0xff;
			b = 0;
		}
		else
		{
			r = 0xff;
			g = int((255 - ref) * 255 / (256 - 150)) & 0xff;
			b = 0;
		}
		return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
	};

	std::vector<glm::vec3> inpc;
	read_pcd_file(argv[1], inpc);
	for (int i = 0; i < inpc.size(); i++)
	{
		auto color = i2color((inpc[i].z - 1.2) * 255 / 0.5);
		point_vertex.push_back(Vertex(inpc[i], color));
		point_idx.push_back(i);
	}

	HVKApp Llidar;
	// read_lvx_file(argv[1], point_vertex, point_idx);
	// read_pcd_file(argv[1], point_vertex, point_idx);
	// read_las_file(argv[1], point_vertex, point_idx);

	Llidar.setIndexedVertex(point_vertex, point_idx);
	Llidar.setPrimitiveTopology(vk::PrimitiveTopology::ePointList);
	// save_pcd_file("misc/savepcd.pcd", point_vertex, point_idx);

	// HVKMesh block;
	// block.setContext(context, camera, gui);
	// SimpleOBJReader sor;
	// sor.readOBJ("model/block.obj");
	// sor.getIndexedVertex(point_vertex, point_idx);
	// block.setIndexedVertex(point_vertex, point_idx);
	// block.setMeshTexturePath("model/block.obj", "textures/shuini.png");
	// block.init();

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
			// block.drawFrame();

			gui->drawFrame();

			context->drawEnd();
		}
		context->mainLoopExit();

		Llidar.deinit();
		// block.deinit();

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
