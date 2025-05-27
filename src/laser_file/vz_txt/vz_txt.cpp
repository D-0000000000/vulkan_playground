#include "vz_txt.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

int read_vz_txt_file(const char *filename, std::vector<glm::vec3> &laspc)
{
	laspc.clear();
	std::fstream fin(filename, std::ios::in);
	while (1)
	{
		if (fin.fail())
		{
			break;
		}
		std::string line;
		std::getline(fin, line);
		if (line[0] != '{')
		{
			continue;
		}
		int firstbracket = 0;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == '}')
			{
				firstbracket = i;
				break;
			}
		}
		line.resize(firstbracket + 1);
		int comma[2] = {-1, -1};
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == ',')
			{
				if (comma[0] == -1)
				{
					comma[0] = i;
				}
				else
				{
					comma[1] = i;
					break;
				}
			}
		}
		std::string xx = line.substr(2, comma[0] - 2);
		std::string yy = line.substr(comma[0] + 2, comma[1] - comma[0] - 2);
		std::string zz = line.substr(comma[1] + 2, line.size() - 4 - comma[1]);
		std::stringstream ss;
		double x, y, z;
		ss << xx;
		ss >> x;
		ss.clear();
		ss << yy;
		ss >> y;
		ss.clear();
		ss << zz;
		ss >> z;
		ss.clear();
		glm::vec3 pt(x, y, z);
		laspc.push_back(pt);
	}
	return 0;
}
