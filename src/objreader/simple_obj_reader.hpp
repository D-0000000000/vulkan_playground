#ifndef _SIMPLE_OBJ_READER_HPP_
#define _SIMPLE_OBJ_READER_HPP_

#include "vkcommon/vkcommon.hpp"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

class SimpleOBJReader
{
public:
	SimpleOBJReader()
	{
	}

	SimpleOBJReader(char *file_name);

	~SimpleOBJReader();

	void readOBJ(char *file_name);

	void getIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind);

private:
	std::fstream fin;
	std::vector<std::string> file_line;
	std::vector<glm::vec3> vertex;
	std::vector<glm::vec3> vertex_normal;
	std::vector<glm::vec2> vertex_tex_coord;
	std::vector<glm::ivec3> face;
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

#endif
