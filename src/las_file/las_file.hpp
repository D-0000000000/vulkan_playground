#ifndef _LAS_FILE_HPP_
#define _LAS_FILE_HPP_

#include "vkcommon/vkcommon.hpp"

#include <cstdint>
#include <cstring>
#include <fstream>

#pragma pack(1)

struct las_public_header
{
	char signature[4];
	uint16_t source_id;
	uint16_t global_encoding;
	uint32_t guid1;
	uint16_t guid2;
	uint16_t guid3;
	uint8_t guid4[8];
	uint8_t version_major;
	uint8_t version_minor;
	char system_id[32];
	char gen_sw[32];
	uint16_t file_day;
	uint16_t file_year;
	uint16_t header_size;
	uint32_t offset_to_point_data;
	uint32_t var_len_rec_num;
	uint8_t point_data_format_id;
	uint16_t point_data_rec_len;
	uint32_t point_rec_num;
	uint32_t point_ret_num[5];
	double X_scale;
	double Y_scale;
	double Z_scale;
	double X_offset;
	double Y_offset;
	double Z_offset;
	double Max_X;
	double Min_X;
	double Max_Y;
	double Min_Y;
	double Max_Z;
	double Min_Z;
};

struct variable_length_record
{
	uint16_t reserved;
	char user_id[16];
	uint16_t rec_id;
	uint16_t rec_len;
	char description[32];
};

struct point_data_record_format_0
{
	int32_t X;
	int32_t Y;
	int32_t Z;
	uint16_t intensity;
	uint8_t ret_num : 3;
	uint8_t num_ret : 3;
	uint8_t scan_dir_flag : 1;
	uint8_t edge_of_flight_line : 1;
	uint8_t classification;
	int8_t scan_angle_rank;
	uint8_t user_data;
	uint16_t source_id;
};

struct point_data_record_format_2
{
	int32_t X;
	int32_t Y;
	int32_t Z;
	uint16_t intensity;
	uint8_t ret_num : 3;
	uint8_t num_ret : 3;
	uint8_t scan_dir_flag : 1;
	uint8_t edge_of_flight_line : 1;
	uint8_t classification;
	int8_t scan_angle_rank;
	uint8_t user_data;
	uint16_t source_id;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
};

#pragma pack()

struct las_file
{
	std::fstream flas;
	las_public_header lph;
	variable_length_record vlr;

	void wr_init(char *filename)
	{
		flas.open(filename, std::ios::out | std::ios::binary);
		memset(&lph, 0, sizeof(lph));
		strcpy(lph.signature, "LASF");
		lph.version_major = 1;
		lph.version_minor = 2;
		strcpy(lph.system_id, "AVIA");
		strcpy(lph.gen_sw, "YINYC@XDD");
		lph.header_size = sizeof(las_public_header);
		lph.offset_to_point_data = sizeof(las_public_header) + sizeof(variable_length_record);
		lph.var_len_rec_num = 1;
		lph.point_data_format_id = 2;
		lph.point_data_rec_len = sizeof(point_data_record_format_2);
		lph.point_rec_num;
		lph.point_ret_num;
		lph.X_scale = 1;
		lph.Y_scale = 1;
		lph.Z_scale = 1;
		lph.X_offset = 0;
		lph.Y_offset = 0;
		lph.Z_offset = 0;

		memset(&vlr, 0, sizeof(vlr));
		strcpy(vlr.user_id, "");

		flas.write((char *)&lph, sizeof(lph));
		flas.write((char *)&vlr, sizeof(vlr));

		return;
	}

	void write_point(point_data_record_format_2 pdrf2)
	{
		flas.write((char *)&pdrf2, sizeof(pdrf2));
		return;
	}

	void set_point_num(size_t point_num)
	{
		size_t wrpos = flas.tellp();
		flas.seekg(0);
		lph.point_rec_num = point_num;
		lph.point_ret_num[0] = point_num;
		flas.write((char *)&lph, sizeof(lph));
		return;
	}

	void rd_init(char *filename)
	{
		flas.open(filename, std::ios::in | std::ios::binary);
		return;
	}

	~las_file()
	{
		flas.close();
	}
};

int read_las_file(char *filename, std::vector<Vertex> &laspc, std::vector<uint32_t> &lasind);

#endif
