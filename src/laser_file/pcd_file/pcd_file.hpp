#ifndef _PCD_FILE_HPP_
#define _PCD_FILE_HPP_

#include <glm/glm.hpp>

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>

#include <cstdint>
#include <vector>

int read_pcd_file(const char *filename, std::vector<glm::vec3> &laspc);

int read_pcd_file(const char *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr laspc);

int save_pcd_file(const char *filename, std::vector<glm::vec3> &laspc);

int save_pcd_file(const char *filename, pcl::PointCloud<pcl::PointXYZ>::Ptr laspc);

#endif
