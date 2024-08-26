#ifndef _RANDOM_SAMPLE_CONSENSUS_HPP_
#define _RANDOM_SAMPLE_CONSENSUS_HPP_

#include <cstddef>
#include <vector>
#include <glm/glm.hpp>

size_t get_filtered_size();

size_t get_filtered_res(float outx[], float outy[], float outz[]);

bool first_filter(std::vector<glm::vec3> &inpc, std::vector<glm::vec3> &outpc);

#endif
