#include "lvx_file.hpp"
#include "vkrun/vkrun.hpp"
#include <fstream>
#include <vector>
#include <cstdint>

std::vector<Vertex> lvxpc;
std::vector<uint32_t> lvxind;

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

int read_lvx_file(char *filename)
{
	lvxpc.clear();
	std::fstream fin;
	fin.open(filename, std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
	size_t file_size = fin.tellg();
	fin.seekg(0);
	uint8_t rxbuf[sizeof(LvxBasePackDetail)];
	fin.read((char *)rxbuf, sizeof(LvxFilePublicHeader));
	fin.read((char *)rxbuf, sizeof(LvxFilePrivateHeader));
	fin.read((char *)rxbuf, sizeof(LvxDeviceInfo));
	size_t cur_offset = sizeof(LvxFilePublicHeader) + sizeof(LvxFilePrivateHeader) + sizeof(LvxDeviceInfo);
	while (cur_offset < file_size)
	{
		memset(rxbuf, 0, sizeof(FrameHeader));
		fin.read((char *)rxbuf, sizeof(FrameHeader));
		FrameHeader *fh = (FrameHeader *)rxbuf;
		cur_offset += sizeof(FrameHeader);
		size_t block_size = fh->next_offset - fh->current_offset;
		size_t next_offset = fh->next_offset;
		// std::cout << fh->current_offset << " " << (block_size - sizeof(FrameHeader)) << " " << (block_size - sizeof(FrameHeader)) % sizeof(LvxBasePackDetail) << "\n";

		// for (int i = 0; i < pack_num; i++)
		while (cur_offset < next_offset)
		{
			int hsize = sizeof(LvxBasePackDetail) - sizeof(LvxBasePackDetail::raw_point) - sizeof(LvxBasePackDetail::pack_size);
			fin.read((char *)rxbuf, hsize);
			cur_offset += hsize;
			LvxBasePackDetail *bpd = (LvxBasePackDetail *)rxbuf;
			size_t psize = 0;
#if 1
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
				std::cout << "Unknown data " << (uint32_t)bpd->data_type << "\n";
			}
#endif
			cur_offset += psize;

			if (bpd->data_type == kCartesian)
			{
				fin.read((char *)rxbuf, psize);
				LivoxRawPoint *lerp = (LivoxRawPoint *)rxbuf;
				for (int i = 0; i < RAW_POINT_NUM; i++)
				{
					Vertex vt;
					glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
					vt.pos = pos;
					vt.color = color;
					lvxpc.push_back(vt);
				}
			}
			else if (bpd->data_type == kSpherical)
			{
				fin.read((char *)rxbuf, psize);
				LivoxSpherPoint *lerp = (LivoxSpherPoint *)bpd->raw_point;
			}
			else if (bpd->data_type == kExtendCartesian)
			{
				std::cout << "kExtendCartesian\n";
				fin.read((char *)rxbuf, psize);
				LivoxExtendRawPoint *lerp = (LivoxExtendRawPoint *)rxbuf;
				for (int i = 0; i < SINGLE_POINT_NUM; i++)
				{
					Vertex vt;
					glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
					vt.pos = pos;
					vt.color = color;
					lvxpc.push_back(vt);
				}
			}
			else if (bpd->data_type == kExtendSpherical)
			{
				fin.read((char *)rxbuf, psize);
				LivoxExtendSpherPoint *lerp = (LivoxExtendSpherPoint *)rxbuf;
			}
			else if (bpd->data_type == kDualExtendCartesian)
			{
				std::cout << "kDualExtendCartesian\n";
				fin.read((char *)rxbuf, psize);
				LivoxDualExtendRawPoint *lerp = (LivoxDualExtendRawPoint *)rxbuf;
				for (int i = 0; i < DUAL_POINT_NUM; i++)
				{
					Vertex vt;
					glm::vec3 pos(lerp[i].x1 / 1000.0, lerp[i].y1 / 1000.0, lerp[i].z1 / 1000.0);
					glm::vec3 color = glm_lvx_color(lerp[i].reflectivity1);
					vt.pos = pos;
					vt.color = color;
					lvxpc.push_back(vt);
					pos = glm::vec3(lerp[i].x2 / 1000.0, lerp[i].y2 / 1000.0, lerp[i].z2 / 1000.0);
					color = glm_lvx_color(lerp[i].reflectivity2);
					vt.pos = pos;
					vt.color = color;
					lvxpc.push_back(vt);
				}
			}
			else if (bpd->data_type == kDualExtendSpherical)
			{
				fin.read((char *)rxbuf, psize);
				LivoxDualExtendSpherPoint *lerp = (LivoxDualExtendSpherPoint *)rxbuf;
			}
			else if (bpd->data_type == kImu)
			{
				fin.read((char *)rxbuf, psize);
				LivoxImuPoint *lerp = (LivoxImuPoint *)rxbuf;
			}
			else if (bpd->data_type == kTripleExtendCartesian)
			{
				fin.read((char *)rxbuf, psize);
				LivoxTripleExtendRawPoint *ltep = (LivoxTripleExtendRawPoint *)rxbuf;
				for (int i = 0; i < TRIPLE_POINT_NUM; i++)
				{
					LivoxExtendRawPoint *lerp = (LivoxExtendRawPoint *)&ltep[i];
					bool echo_flag = false;
					for (int j = 0; j < 3; j++)
					{
						Vertex vt;
						glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
						glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
						vt.pos = pos;
						vt.color = color;
						if ((lerp[i].tag & 0b00110000) == 0b00000000)
						{
							// lvxpc.push_back(vt);
							echo_flag = true;
						}
						else if ((lerp[i].tag & 0b00110000) == 0b00010000)
						{
							// lvxpc.push_back(vt);
						}
						else if ((lerp[i].tag & 0b00110000) == 0b00100000)
						{
							// lvxpc.push_back(vt);
							echo_flag = true;
						}
						else if ((lerp[i].tag & 0b00110000) == 0b00110000)
						{
							// lvxpc.push_back(vt);
							echo_flag = true;
						}
						else
						{
							echo_flag = true;
							std::cout << "tag!!!\n";
						}
					}
					if (!echo_flag)
					{
						for (int j = 0; j < 3; j++)
						{
							Vertex vt;
							glm::vec3 pos(lerp[i].x / 1000.0, lerp[i].y / 1000.0, lerp[i].z / 1000.0);
							glm::vec3 color = glm_lvx_color(lerp[i].reflectivity);
							vt.pos = pos;
							vt.color = color;
							if ((lerp[i].tag & 0b00110000) == 0b00010000)
							{
								lvxpc.push_back(vt);
							}
							else
							{
								std::cout << "tag!!!\n";
							}
						}
					}
				}
			}
			else if (bpd->data_type == kTripleExtendSpherical)
			{
				fin.read((char *)rxbuf, psize);
				LivoxTripleExtendSpherPoint *lerp = (LivoxTripleExtendSpherPoint *)rxbuf;
			}
			else
			{
				std::cout << "Unknown data " << (uint32_t)bpd->data_type << "\n";
			}
		}
		fin.seekg(next_offset);
		cur_offset = next_offset;
	}
	fin.close();
	for (size_t i = 0; i < lvxpc.size(); i++)
	{
		lvxind.push_back(i);
	}
	setIndexedVertex(lvxpc, lvxind);
	return 0;
}
