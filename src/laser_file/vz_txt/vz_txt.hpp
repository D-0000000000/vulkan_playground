#ifndef _VZ_TXT_FILE_HPP_
#define _VZ_TXT_FILE_HPP_

#include <glm/glm.hpp>

#include <vector>

int read_vz_txt_file(const char *filename, std::vector<glm::vec3> &laspc);

#endif
