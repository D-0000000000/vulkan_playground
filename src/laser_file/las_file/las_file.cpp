#include "las_file.hpp"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
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

int read_las_file(char *filename, std::vector<glm::vec3> &laspc)
{
	laspc.clear();
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
			x /= 1000.0;
			y /= 1000.0;
			z /= 1000.0;
			auto color = glm_las_color((z - 1.2) * 255 / 0.3);
			// x = pcp.x;
			// y = pcp.y;
			// z = pcp.z;
			// pdrf2.X = (x - lph.X_offset) / lph.X_scale;
			// pdrf2.Y = (y - lph.X_offset) / lph.Y_scale;
			// pdrf2.Z = (z - lph.X_offset) / lph.Z_scale;
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
			// pdrf2.X = (x - lph.X_offset) / lph.X_scale;
			// pdrf2.Y = (y - lph.X_offset) / lph.Y_scale;
			// pdrf2.Z = (z - lph.X_offset) / lph.Z_scale;
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
			// pdrf2.X = (x - lph.X_offset) / lph.X_scale;
			// pdrf2.Y = (y - lph.X_offset) / lph.Y_scale;
			// pdrf2.Z = (z - lph.X_offset) / lph.Z_scale;
			pcc.push_back(std::make_pair(glm::vec3(x, y, z), color));
		}
	}
	for (auto pt : pcc)
	{
		laspc.push_back(pt.first);
	}
	std::cout << laspc.size() << " lassize\n";
	fin.close();

	return 0;
}

int save_las_file(const char *filename, std::vector<glm::vec3> &laspc)
{
	std::fstream fout;
	fout.open(filename, std::ios_base::out | std::ios_base::binary | std::ios_base::ate);
	fout.seekp(0);
	las_public_header lph;
	variable_length_record vlr;
	memset(&lph, 0, sizeof(lph));
	strcpy(lph.signature, "LASF");
	lph.version_major = 1;
	lph.version_minor = 2;
	lph.header_size = sizeof(las_public_header);
	lph.offset_to_point_data = sizeof(las_public_header) + sizeof(variable_length_record);
	lph.var_len_rec_num = 1;
	lph.point_data_format_id = 0;
	lph.point_data_rec_len = sizeof(point_data_record_format_0);
	lph.point_rec_num = laspc.size();
	lph.point_ret_num;
	lph.X_scale = 1e-6;
	lph.Y_scale = 1e-6;
	lph.Z_scale = 1e-6;
	lph.X_offset = 0;
	lph.Y_offset = 0;
	lph.Z_offset = 0;
	fout.write((char *)&lph, sizeof(las_public_header));

	memset(&vlr, 0, sizeof(vlr));
	strcpy(vlr.user_id, "");
	fout.write((char *)&vlr, sizeof(vlr));

	std::cout << "POINT DATA RECORD FORMAT " << (uint32_t)lph.point_data_format_id << "\n";
	if (lph.point_data_format_id == 0)
	{
		point_data_record_format_0 pdrf0;
		for (auto &pt : laspc)
		{
			memset(&pdrf0, 0, sizeof(pdrf0));
			pdrf0.X = pt.x * 1000000;
			pdrf0.Y = pt.y * 1000000;
			pdrf0.Z = pt.z * 1000000;
			fout.write((char *)&pdrf0, sizeof(pdrf0));
		}
	}
	else if (lph.point_data_format_id == 1)
	{
		point_data_record_format_1 pdrf1;
		for (auto &pt : laspc)
		{
			memset(&pdrf1, 0, sizeof(pdrf1));
			pdrf1.X = pt.x;
			pdrf1.Y = pt.y;
			pdrf1.Z = pt.z;
			fout.write((char *)&pdrf1, sizeof(pdrf1));
		}
	}
	else if (lph.point_data_format_id == 2)
	{
		point_data_record_format_1 pdrf2;
		for (auto &pt : laspc)
		{
			memset(&pdrf2, 0, sizeof(pdrf2));
			pdrf2.X = pt.x;
			pdrf2.Y = pt.y;
			pdrf2.Z = pt.z;
			fout.write((char *)&pdrf2, sizeof(pdrf2));
		}
	}
	fout.close();

	return 0;
}
