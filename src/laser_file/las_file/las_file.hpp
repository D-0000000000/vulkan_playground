#ifndef _LAS_FILE_HPP_
#define _LAS_FILE_HPP_

#include <glm/glm.hpp>

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

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

struct point_data_record_format_1
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
	double gps_time;
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

struct point_data_record_format_3
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
	double gps_time;
	uint16_t red;
	uint16_t green;
	uint16_t blue;
};

#pragma pack()

int read_las_file(const char *filename, std::vector<glm::vec3> &laspc);

int save_las_file(const char *filename, std::vector<glm::vec3> &laspc);

#endif
