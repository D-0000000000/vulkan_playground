#include "las_file.hpp"
#include "ransac/random_sample_consensus.hpp"
#include "vkcommon/vkcommon.hpp"

#include <cmath>
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
	las_public_header *lph = (las_public_header *)rxbuf;
	uint32_t offset_to_point_data = lph->offset_to_point_data;
	uint32_t point_num = lph->point_rec_num;
	std::cout << offset_to_point_data << " data offset\n";
	fin.seekg(offset_to_point_data);
	std::vector<std::pair<glm::vec3, glm::vec3>> pcc;
	std::cout << point_num << " point_num\n";

	std::cout << "POINT DATA RECORD FORMAT " << (uint32_t)lph->point_data_format_id << "\n";
	if (lph->point_data_format_id == 0)
	{
		point_data_record_format_0 pdrf0;
		for (int i = 0; i < point_num; i++)
		{
			memset(&pdrf0, 0, sizeof(pdrf0));
			fin.read((char *)&pdrf0, sizeof(pdrf0));
			float x = (pdrf0.X * lph->X_scale) + lph->X_offset;
			float y = (pdrf0.Y * lph->Y_scale) + lph->Y_offset;
			float z = (pdrf0.Z * lph->Z_scale) + lph->Z_offset;
			auto color = glm_las_color((z - 1.2) * 255 / 0.3);
			// x = pcp.x;
			// y = pcp.y;
			// z = pcp.z;
			// pdrf2.X = (x - lph->X_offset) / lph->X_scale;
			// pdrf2.Y = (y - lph->X_offset) / lph->Y_scale;
			// pdrf2.Z = (z - lph->X_offset) / lph->Z_scale;
			pcc.push_back(std::make_pair(glm::vec3(x, y, z), color));
		}
	}
	else if (lph->point_data_format_id == 1)
	{
		point_data_record_format_1 pdrf1;
		for (int i = 0; i < point_num; i++)
		{
			memset(&pdrf1, 0, sizeof(pdrf1));
			fin.read((char *)&pdrf1, sizeof(pdrf1));
			float x = (pdrf1.X * lph->X_scale) + lph->X_offset;
			float y = (pdrf1.Y * lph->Y_scale) + lph->Y_offset;
			float z = (pdrf1.Z * lph->Z_scale) + lph->Z_offset;
			auto color = glm_las_color(z * 255 / 2.0);
			// x = pcp.x;
			// y = pcp.y;
			// z = pcp.z;
			// pdrf2.X = (x - lph->X_offset) / lph->X_scale;
			// pdrf2.Y = (y - lph->X_offset) / lph->Y_scale;
			// pdrf2.Z = (z - lph->X_offset) / lph->Z_scale;
			pcc.push_back(std::make_pair(glm::vec3(x, y, z), color));
		}
	}
	else if (lph->point_data_format_id == 2)
	{
		point_data_record_format_2 pdrf2;
		for (int i = 0; i < point_num; i++)
		{
			memset(&pdrf2, 0, sizeof(pdrf2));
			fin.read((char *)&pdrf2, sizeof(pdrf2));
			float x = (pdrf2.X * lph->X_scale) + lph->X_offset;
			float y = (pdrf2.Y * lph->Y_scale) + lph->Y_offset;
			float z = (pdrf2.Z * lph->Z_scale) + lph->Z_offset;
			auto color = glm_las_color(z * 255 / 2.0);
			// x = pcp.x;
			// y = pcp.y;
			// z = pcp.z;
			// pdrf2.X = (x - lph->X_offset) / lph->X_scale;
			// pdrf2.Y = (y - lph->X_offset) / lph->Y_scale;
			// pdrf2.Z = (z - lph->X_offset) / lph->Z_scale;
			pcc.push_back(std::make_pair(glm::vec3(x, y, z), color));
		}
	}
	for (auto pt : pcc)
	{
		laspc.push_back({pt.first, pt.second});
	}
	std::cout << laspc.size() << " lassize\n";
	for (int i = 0; i < laspc.size(); i++)
	{
		lasind.push_back(i);
	}
	fin.close();
	// fout.close();

	return 0;
}
