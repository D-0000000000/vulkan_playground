#ifndef _PCD_FILE_HPP_
#define _PCD_FILE_HPP_

#include "vkcommon/vkcommon.hpp"
#include <cstdint>
#include <vector>

int read_pcd_file(char *filename, std::vector<Vertex> &laspc, std::vector<uint32_t> &lasind);

#endif
