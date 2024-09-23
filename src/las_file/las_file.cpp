#include "las_file.hpp"
#include "ransac/random_sample_consensus.hpp"
#include "vkcommon/vkcommon.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>

glm::vec3 glm_las_color(uint8_t ref)
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
}

int read_las_file(char *filename, std::vector<Vertex> &laspc, std::vector<uint32_t> &lasind)
{
	laspc.clear();
	lasind.clear();
	std::fstream fin;
	fin.open(filename, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	// std::fstream fout("rot.las", std::ios_base::out | std::ios_base::binary);
	size_t file_size = fin.tellg();
	std::cout << file_size << " filesize\n";
	fin.seekg(0);
	uint8_t rxbuf[sizeof(las_public_header)];
	memset(rxbuf, 0, sizeof(rxbuf));
	fin.read((char *)rxbuf, sizeof(las_public_header));
	// fout.write((char *)rxbuf, sizeof(las_public_header));
	las_public_header *lph = (las_public_header *)rxbuf;
	uint32_t offset_to_point_data = lph->offset_to_point_data;
	uint32_t point_num = lph->point_rec_num;
	std::cout << offset_to_point_data << " data offset\n";
	fin.seekg(offset_to_point_data);
	point_data_record_format_2 pdrf2;
	glm::vec3 pl(0.44f, 0.06f, 0.89f);
	pl = glm::normalize(pl);
	glm::vec3 zaxis(0.0f, 0.0f, 1.0f);
	glm::vec3 rotaxis = glm::cross(pl, zaxis);
	float angle = glm::angle(pl, zaxis);
	glm::mat4 rotmat = glm::rotate(glm::mat4(1.0f), glm::degrees(angle), glm::normalize(rotaxis));
	glm::vec3 lidar_centre(1e9, 0, 0);
	std::vector<std::pair<glm::vec3, glm::vec3>> pcc;
	std::cout << point_num << " point_num\n";
	int min_return_number = 0;
	for (int i = 0; i < point_num; i++)
	{
		memset(&pdrf2, 0, sizeof(pdrf2));
		fin.read((char *)&pdrf2, sizeof(pdrf2));
		float x = (pdrf2.X * lph->X_scale) + lph->X_offset;
		float y = (pdrf2.Y * lph->Y_scale) + lph->Y_offset;
		float z = (pdrf2.Z * lph->Z_scale) + lph->Z_offset;
		// std::cout << x << " " << y << " " << z << "\n";
		auto color = glm_las_color(pdrf2.intensity);
		// auto color = glm::vec3(0.8f, 0.0f, 0.0f);
		// glm::vec4 pcp(x, y, z, 1.0f);
		// pcp = rotmat * pcp;
		// x = pcp.x;
		// y = pcp.y;
		// z = pcp.z;
		// pdrf2.X = (x - lph->X_offset) / lph->X_scale;
		// pdrf2.Y = (y - lph->X_offset) / lph->Y_scale;
		// pdrf2.Z = (z - lph->X_offset) / lph->Z_scale;

		float dist = sqrt(x * x + y * y + z * z);
		if (dist < 2.0 || dist > 16.0)
		{
			continue;
		}
		min_return_number = std::max(min_return_number, (int)pdrf2.ret_num);
		// std::cout << (int)pdrf2.ret_num << " ret\n";
		pcc.push_back(std::make_pair(glm::vec3(x, y, z), color));
		// fout.write((char *)&pdrf2, sizeof(pdrf2));
		// uint32_t ref = dist * 25.5;
		// if (ref > 255)
		// {
		// 	ref = 255;
		// }
		// color = glm_las_color(ref);
		if (fabs(y) < 0.05 && fabs(z) < 0.05)
		{
			lidar_centre.x = std::min(lidar_centre.x, x);
		}
	}
	std::cout << min_return_number << " min return number\n";
	std::cout << lidar_centre.x << " " << lidar_centre.y << " " << lidar_centre.z << " centre\n";
	float centre_dist = glm::length(lidar_centre);
	for (auto pt : pcc)
	{
		laspc.push_back({pt.first, pt.second});
	}
	// for (auto pt : pcc)
	// {
	// 	float dist = glm::length(pt.first - lidar_centre);
	// 	// if (dist > centre_dist)
	// 	// {
	// 	// 	continue;
	// 	// }
	// 	laspc.push_back({pt.first, pt.second});
	// }
	std::cout << laspc.size() << " lassize\n";
	for (int i = 0; i < laspc.size(); i++)
	{
		lasind.push_back(i);
	}
	fin.close();
	// fout.close();
	laspc.push_back({{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}});
	lasind.push_back(laspc.size());
	laspc.push_back({lidar_centre, {1.0f, 1.0f, 1.0f}});
	lasind.push_back(laspc.size());

	return 0;
}
