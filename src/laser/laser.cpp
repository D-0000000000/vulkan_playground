#include "laser.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

struct pointcloud
{
	uint64_t timestamp;
	double X;
	double Y;
	double Z;
	uint8_t Reflectivity;
	uint64_t Ori_x;
	uint64_t Ori_y;
	uint64_t Ori_z;
};

// std::vector<Vertex> vts = {
//     {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}},
//     {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
//     {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}},
//     {{1.0f, 1.0f, -1.0f}, {1.0f, 1.0f, 1.0f}},
//     {{-1.0f, -1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}},
//     {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 1.0f}},
//     {{1.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
//     {{1.0f, 1.0f, 1.0f}, {0.5f, 0.5f, 0.5f}}};

// std::vector<uint16_t> inds = {
//     0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 4, 2, 4, 2, 6, 1, 3, 5, 3, 5, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7};

std::vector<Vertex> vts = {
	{{-0.5f, -0.5f, -1.0f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f, 4.0f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 1.5f, 10.0f}, {1.0f, 1.0f, 1.0f}}};

std::vector<uint32_t> inds = {
	0, 1, 2, 2, 3, 0};

// Timestamp, X, Y, Z, Reflectivity, Ori_x, Ori_y, Ori_z

int read_laser_csv(char *filename, std::vector<Vertex> &spcvk, std::vector<uint32_t> &ind)
{
	std::fstream fin(filename, std::ios_base::in);
	std::string str;
	fin >> str;
	while (!fin.eof())
	{
		fin >> str;
		pointcloud onepc;
		sscanf(str.c_str(), "%llu,%lf,%lf,%lf,%hhu,%llu,%llu,%llu", &onepc.timestamp, &onepc.X, &onepc.Y, &onepc.Z, &onepc.Reflectivity, &onepc.Ori_x, &onepc.Ori_y, &onepc.Ori_z);
		Vertex pv1;
		pv1.pos = glm::vec3(onepc.X, onepc.Y, onepc.Z);
		int ref = onepc.Reflectivity;
		int r = std::max(ref * 2 - 255, 0);
		int g = std::min(ref * 2, 255 * 2 - ref * 2);
		int b = std::max(255 - ref * 2, 0);
		pv1.color = glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
		spcvk.push_back(pv1);
	}
	fin.close();
	for (int i = 0; i < spcvk.size(); i++)
	{
		ind.push_back(i);
	}
	return 0;
}
