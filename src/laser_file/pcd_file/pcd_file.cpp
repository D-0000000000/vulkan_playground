#include "pcd_file.hpp"

#include <glm/glm.hpp>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include <cmath>

int read_pcd_file(const char *filename, std::vector<glm::vec3> &laspc)
{
	laspc.clear();

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	if (pcl::io::loadPCDFile<pcl::PointXYZ>(filename, *cloud) == -1)
	{
		PCL_ERROR("Couldn't read file test_pcd.pcd \n");
		return (-1);
	}

	std::cout << "Loaded " << cloud->width * cloud->height << " data points from test_pcd.pcd with the following fields: " << std::endl;

	for (const auto &point : *cloud)
	{
		if (std::isnan(point.x) || std::isnan(point.y) || std::isnan(point.z))
		{
			continue;
		}
		laspc.push_back(glm::vec3(point.x, point.y, point.z));
	}
	return 0;
}

int read_pcd_file(const char *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr laspc)
{
	laspc->clear();
	if (pcl::io::loadPCDFile<pcl::PointXYZ>(filename, *laspc) == -1)
	{
		PCL_ERROR("Couldn't read file test_pcd.pcd \n");
		return (-1);
	}
	std::cout << "Loaded " << laspc->width * laspc->height << " data points from test_pcd.pcd with the following fields: " << std::endl;

	return 0;
}

int save_pcd_file(const char *filename, std::vector<glm::vec3> &laspc)
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	for (auto &pt : laspc)
	{
		cloud->push_back(pcl::PointXYZ(pt.x, pt.y, pt.z));
	}
	cloud->height = 1;
	cloud->width = laspc.size();
	cloud->is_dense = true;
	if (pcl::io::savePCDFile<pcl::PointXYZ>(filename, *cloud, true) == -1)
	{
		PCL_ERROR("Couldn't save file test_pcd.pcd \n");
		return (-1);
	}

	return 0;
}

int save_pcd_file(const char *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr cloud)
{
	if (pcl::io::savePCDFile<pcl::PointXYZ>(filename, *cloud, true) == -1)
	{
		PCL_ERROR("Couldn't save file test_pcd.pcd \n");
		return (-1);
	}

	return 0;
}