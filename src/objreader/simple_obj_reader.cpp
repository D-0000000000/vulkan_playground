#include "simple_obj_reader.hpp"

SimpleOBJReader::SimpleOBJReader(char *file_name)
{
	readOBJ(file_name);
}

SimpleOBJReader::~SimpleOBJReader()
{
	if (fin.is_open())
	{
		fin.close();
	}
}

glm::ivec3 read_face(std::string str)
{
	std::vector<uint32_t> spilt;
	spilt.clear();
	for (uint32_t i = 0; i < str.length(); i++)
	{
		if (str[i] == '/')
		{
			spilt.push_back(i);
		}
	}
	glm::ivec3 face;
	std::string fx = str.substr(0, spilt[0]);
	std::string fy = str.substr(spilt[0] + 1, spilt[1] - spilt[0] - 1);
	std::string fz = spilt[1] != str.size() ? str.substr(spilt[1] + 1, str.size() - spilt[1] - 1) : "";
	auto str2int = [](std::string s)
	{
		if (s.length() == 0)
		{
			return 0;
		}
		int ret = 0;
		sscanf(s.c_str(), "%d", &ret);
		return ret;
	};
	face = glm::ivec3(str2int(fx), str2int(fy), str2int(fz));
	return face;
}

void SimpleOBJReader::readOBJ(char *file_name)
{
	std::cout << file_name << " file_name\n";
	fin.open(file_name, std::ios_base::in);
	std::string vline;
	std::stringstream ss;
	while (std::getline(fin, vline))
	{
		if (vline.length() < 2)
		{
			continue;
		}
		ss.flush();
		ss.clear();
		ss << vline;
		ss << "\n";
		std::string op;
		ss >> op;
		if (op == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			vertex.push_back(glm::vec3(x, y, z));
		}
		else if (op == "vt")
		{
			float x, y;
			ss >> x >> y;
			vertex_tex_coord.push_back(glm::vec2(x, y));
		}
		else if (op == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			vertex_normal.push_back(glm::vec3(x, y, z));
		}
		else if (op == "f")
		{
			std::string vv[3];
			ss >> vv[0] >> vv[1] >> vv[2];
			glm::ivec3 vve[3];
			for (int i = 0; i < 3; i++)
			{
				glm::ivec3 xyz = read_face(vv[i]);
				// std::cout << xyz.x << " xyzx\n";
				// uint32_t x, y, z;
				// sscanf(vv[i].c_str(), "%d/%d/%d", &x, &y, &z);
				// vve[i] = glm::ivec3(x, y, z);P
				vve[i] = xyz;
				if (vertices.size() < xyz.x)
				{
					vertices.resize(xyz.x);
				}
				vertices[xyz.x - 1] = {vertex[xyz.x - 1], glm::vec3(0.0f, 0.0f, 0.0f), vertex_tex_coord[xyz.y - 1]};
			}
			face.push_back(glm::ivec3(vve[0].x, vve[1].x, vve[2].x));
		}
	}
	fin.close();
	for (auto f : face)
	{
		indices.push_back(f.x - 1);
		indices.push_back(f.y - 1);
		indices.push_back(f.z - 1);
	}
	return;
}

void SimpleOBJReader::getIndexedVertex(std::vector<Vertex> &vx, std::vector<uint32_t> &ind)
{
	vx = vertices;
	ind = indices;
}