#include "lvx_file.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

glm::vec3 glm_lvx_color(uint8_t ref)
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

int read_lvx_file(const char *filename, std::vector<glm::vec3> &lvxpc)
{
	lvxpc.clear();
	std::fstream fin;
	fin.open(filename, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	size_t file_size = fin.tellg();
	fin.seekg(0);
	uint8_t rxbuf[sizeof(LvxBasePackDetail)];
	fin.read((char *)rxbuf, sizeof(LvxFilePublicHeader));
	fin.read((char *)rxbuf, sizeof(LvxFilePrivateHeader));
	uint8_t device_count = ((LvxFilePrivateHeader *)rxbuf)->device_count;
	for (int i = 0; i < device_count; i++)
	{
		fin.read((char *)rxbuf, sizeof(LvxDeviceInfo));
	}
	size_t cur_offset = sizeof(LvxFilePublicHeader) + sizeof(LvxFilePrivateHeader) + device_count * sizeof(LvxDeviceInfo);
	while (cur_offset < file_size)
	{
		memset(rxbuf, 0, sizeof(FrameHeader));
		fin.read((char *)rxbuf, sizeof(FrameHeader));
		FrameHeader *fh = (FrameHeader *)rxbuf;
		cur_offset += sizeof(FrameHeader);
		size_t block_size = fh->next_offset - fh->current_offset;
		size_t next_offset = fh->next_offset;
		while (cur_offset < next_offset)
		{
			// int hsize = sizeof(LvxBasePackDetail) - sizeof(LvxBasePackDetail::raw_point) - sizeof(LvxBasePackDetail::pack_size);
			int hsize = sizeof(LvxBasePackDetail) - sizeof(LvxBasePackDetail::raw_point);
			uint8_t lbpt[sizeof(LvxBasePackDetail)];
			fin.read((char *)lbpt, hsize);
			cur_offset += hsize;
			LvxBasePackDetail *bpd = (LvxBasePackDetail *)lbpt;
			if (bpd->version != 5 || bpd->port_id >= 4)
			{
				std::cout << "invalid\n";
				break;
			}

			size_t psize = 0;
			if (bpd->data_type == kCartesian)
			{
				psize = RAW_POINT_NUM * sizeof(LivoxRawPoint);
			}
			else if (bpd->data_type == kSpherical)
			{
				psize = RAW_POINT_NUM * sizeof(LivoxSpherPoint);
			}
			else if (bpd->data_type == kExtendCartesian)
			{
				psize = SINGLE_POINT_NUM * sizeof(LivoxExtendRawPoint);
			}
			else if (bpd->data_type == kExtendSpherical)
			{
				psize = SINGLE_POINT_NUM * sizeof(LivoxExtendSpherPoint);
			}
			else if (bpd->data_type == kDualExtendCartesian)
			{
				psize = DUAL_POINT_NUM * sizeof(LivoxDualExtendRawPoint);
			}
			else if (bpd->data_type == kDualExtendSpherical)
			{
				psize = DUAL_POINT_NUM * sizeof(LivoxDualExtendSpherPoint);
			}
			else if (bpd->data_type == kImu)
			{
				psize = IMU_POINT_NUM * sizeof(LivoxImuPoint);
			}
			else if (bpd->data_type == kTripleExtendCartesian)
			{
				psize = TRIPLE_POINT_NUM * sizeof(LivoxTripleExtendRawPoint);
			}
			else if (bpd->data_type == kTripleExtendSpherical)
			{
				psize = TRIPLE_POINT_NUM * sizeof(LivoxTripleExtendSpherPoint);
			}
			else
			{
				std::cout << "Unknown data " << (uint32_t)bpd->data_type << " at " << fin.tellg() << "\n";
				break;
			}

			cur_offset += psize;

			if (bpd->data_type == kCartesian)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxRawPoint *lerp = (LivoxRawPoint *)bpd->raw_point;
				for (int i = 0; i < RAW_POINT_NUM; i++)
				{
					glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
					lvxpc.push_back(pos);
				}
			}
			else if (bpd->data_type == kSpherical)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxSpherPoint *lerp = (LivoxSpherPoint *)bpd->raw_point;
			}
			else if (bpd->data_type == kExtendCartesian)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxExtendRawPoint *lerp = (LivoxExtendRawPoint *)bpd->raw_point;
				for (int i = 0; i < SINGLE_POINT_NUM; i++)
				{
					glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
					lvxpc.push_back(pos);
				}
			}
			else if (bpd->data_type == kExtendSpherical)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxExtendSpherPoint *lerp = (LivoxExtendSpherPoint *)bpd->raw_point;
			}
			else if (bpd->data_type == kDualExtendCartesian)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxDualExtendRawPoint *lerp = (LivoxDualExtendRawPoint *)bpd->raw_point;
				for (int i = 0; i < DUAL_POINT_NUM; i++)
				{
					glm::vec3 pos(lerp[i].x1 / 1000.0, lerp[i].y1 / 1000.0, lerp[i].z1 / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity1);
					lvxpc.push_back(pos);
					pos = glm::vec3(lerp[i].x2 / 1000.0, lerp[i].y2 / 1000.0, lerp[i].z2 / 1000.0);
					color = glm_lvx_color(lerp[i].reflectivity2);
					lvxpc.push_back(pos);
				}
			}
			else if (bpd->data_type == kDualExtendSpherical)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxDualExtendSpherPoint *lerp = (LivoxDualExtendSpherPoint *)bpd->raw_point;
			}
			else if (bpd->data_type == kImu)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxImuPoint *lerp = (LivoxImuPoint *)bpd->raw_point;
			}
			else if (bpd->data_type == kTripleExtendCartesian)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxTripleExtendRawPoint *ltep = (LivoxTripleExtendRawPoint *)bpd->raw_point;
				for (int i = 0; i < TRIPLE_POINT_NUM; i++)
				{
					LivoxExtendRawPoint *lerp = (LivoxExtendRawPoint *)&ltep[i];
					bool echo_flag = false;
					for (int j = 0; j < 3; j++)
					{
						if (((lerp[j].tag & 0b00001111) != 0b00000000))
						{
							echo_flag = true;
							// continue;
							break;
						}
					}
					if (((lerp[0].tag & 0b00110000) == 0b00000000))
					{
						echo_flag = true;
					}
					if (!echo_flag)
					{
						for (int j = 0; j < 3; j++)
						{
							glm::vec3 pos(lerp[j].x / 1000.0, lerp[j].y / 1000.0, lerp[j].z / 1000.0);
							glm::vec3 color = glm_lvx_color(lerp[j].reflectivity);
							if ((lerp[j].tag & 0b00110000) == 0b00100000)
							{
								color = glm::vec3(1.0f, 0.0f, 0.0f);
							}
							else if ((lerp[j].tag & 0b00110000) == 0b00110000)
							{
								color = glm::vec3(1.0f, 0.0f, 0.0f);
							}
							if ((lerp[j].tag & 0b00001100) != 0b00000000)
							{
								std::cout << "noise!!!\n";
								color = glm::vec3(1.0f, 0.0f, 1.0f);
							}
							if ((lerp[j].tag & 0b00000011) != 0b00000000)
							{
								std::cout << "noise!!!\n";
								color = glm::vec3(1.0f, 0.0f, 1.0f);
							}
							lvxpc.push_back(pos);
						}
					}
				}
			}
			else if (bpd->data_type == kTripleExtendSpherical)
			{
				fin.read((char *)bpd->raw_point, psize);
				LivoxTripleExtendSpherPoint *lerp = (LivoxTripleExtendSpherPoint *)bpd->raw_point;
			}
		}
		fin.seekg(next_offset);
		cur_offset = next_offset;
	}
	fin.close();
	return 0;
}
