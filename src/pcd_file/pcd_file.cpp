#include "pcd_file.hpp"

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include <cmath>

int read_pcd_file(char *filename, std::vector<Vertex> &laspc, std::vector<uint32_t> &lasind)
{
	laspc.clear();
	lasind.clear();

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	if (pcl::io::loadPCDFile<pcl::PointXYZ>(filename, *cloud) == -1)
	{
		PCL_ERROR("Couldn't read file test_pcd.pcd \n");
		return (-1);
	}

	std::cout << "Loaded " << cloud->width * cloud->height << " data points from test_pcd.pcd with the following fields: " << std::endl;

	for (const auto &point : *cloud)
	{
		if (__builtin_isnan(point.x) || __builtin_isnan(point.y) || __builtin_isnan(point.z))
		{
			continue;
		}
		laspc.push_back({{point.x / 1000.0f, point.y / 1000.0f, point.z / 1000.0f}, {0.5f, 0.5f, 0.5f}});
	}
	for (int i = 0; i < laspc.size(); i++)
	{
		lasind.push_back(i);
	}

	return 0;
}
