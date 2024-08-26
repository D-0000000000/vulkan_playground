#ifndef _LVX_FILE_HPP_
#define _LVX_FILE_HPP_

#include <cstdint>

#define kMaxPointSize 1500
#define kDefaultFrameDurationTime 50

#define WRITE_BUFFER_LEN 1024 * 1024
#define MAGIC_CODE (0xac0ea767)
#define RAW_POINT_NUM 100
#define SINGLE_POINT_NUM 96
#define DUAL_POINT_NUM 48
#define TRIPLE_POINT_NUM 30
#define IMU_POINT_NUM 1
#define M_PI 3.14159265358979323846

#pragma pack(1)

/** Point data type. */
typedef enum
{
    kCartesian,             /**< Cartesian coordinate point cloud. */
    kSpherical,             /**< Spherical coordinate point cloud. */
    kExtendCartesian,       /**< Extend cartesian coordinate point cloud. */
    kExtendSpherical,       /**< Extend spherical coordinate point cloud. */
    kDualExtendCartesian,   /**< Dual extend cartesian coordinate  point cloud. */
    kDualExtendSpherical,   /**< Dual extend spherical coordinate point cloud. */
    kImu,                   /**< IMU data. */
    kTripleExtendCartesian, /**< Triple extend cartesian coordinate  point cloud. */
    kTripleExtendSpherical, /**< Triple extend spherical coordinate  point cloud. */
    kMaxPointDataType       /**< Max Point Data Type. */
} PointDataType;

/** Cartesian coordinate format. */
typedef struct
{
    int32_t x;            /**< X axis, Unit:mm */
    int32_t y;            /**< Y axis, Unit:mm */
    int32_t z;            /**< Z axis, Unit:mm */
    uint8_t reflectivity; /**< Reflectivity */
} LivoxRawPoint;

/** Spherical coordinate format. */
typedef struct
{
    uint32_t depth;       /**< Depth, Unit: mm */
    uint16_t theta;       /**< Zenith angle[0, 18000], Unit: 0.01 degree */
    uint16_t phi;         /**< Azimuth[0, 36000], Unit: 0.01 degree */
    uint8_t reflectivity; /**< Reflectivity */
} LivoxSpherPoint;

/** Standard point cloud format */
typedef struct
{
    float x;              /**< X axis, Unit:m */
    float y;              /**< Y axis, Unit:m */
    float z;              /**< Z axis, Unit:m */
    uint8_t reflectivity; /**< Reflectivity */
} LivoxPoint;

/** Extend cartesian coordinate format. */
typedef struct
{
    int32_t x;            /**< X axis, Unit:mm */
    int32_t y;            /**< Y axis, Unit:mm */
    int32_t z;            /**< Z axis, Unit:mm */
    uint8_t reflectivity; /**< Reflectivity */
    uint8_t tag;          /**< Tag */
} LivoxExtendRawPoint;

/** Extend spherical coordinate format. */
typedef struct
{
    uint32_t depth;       /**< Depth, Unit: mm */
    uint16_t theta;       /**< Zenith angle[0, 18000], Unit: 0.01 degree */
    uint16_t phi;         /**< Azimuth[0, 36000], Unit: 0.01 degree */
    uint8_t reflectivity; /**< Reflectivity */
    uint8_t tag;          /**< Tag */
} LivoxExtendSpherPoint;

/** Dual extend cartesian coordinate format. */
typedef struct
{
    int32_t x1;            /**< X axis, Unit:mm */
    int32_t y1;            /**< Y axis, Unit:mm */
    int32_t z1;            /**< Z axis, Unit:mm */
    uint8_t reflectivity1; /**< Reflectivity */
    uint8_t tag1;          /**< Tag */
    int32_t x2;            /**< X axis, Unit:mm */
    int32_t y2;            /**< Y axis, Unit:mm */
    int32_t z2;            /**< Z axis, Unit:mm */
    uint8_t reflectivity2; /**< Reflectivity */
    uint8_t tag2;          /**< Tag */
} LivoxDualExtendRawPoint;

/** Dual extend spherical coordinate format. */
typedef struct
{
    uint16_t theta;        /**< Zenith angle[0, 18000], Unit: 0.01 degree */
    uint16_t phi;          /**< Azimuth[0, 36000], Unit: 0.01 degree */
    uint32_t depth1;       /**< Depth, Unit: mm */
    uint8_t reflectivity1; /**< Reflectivity */
    uint8_t tag1;          /**< Tag */
    uint32_t depth2;       /**< Depth, Unit: mm */
    uint8_t reflectivity2; /**< Reflectivity */
    uint8_t tag2;          /**< Tag */
} LivoxDualExtendSpherPoint;

/** Triple extend cartesian coordinate format. */
typedef struct
{
    int32_t x1;            /**< X axis, Unit:mm */
    int32_t y1;            /**< Y axis, Unit:mm */
    int32_t z1;            /**< Z axis, Unit:mm */
    uint8_t reflectivity1; /**< Reflectivity */
    uint8_t tag1;          /**< Tag */
    int32_t x2;            /**< X axis, Unit:mm */
    int32_t y2;            /**< Y axis, Unit:mm */
    int32_t z2;            /**< Z axis, Unit:mm */
    uint8_t reflectivity2; /**< Reflectivity */
    uint8_t tag2;          /**< Tag */
    int32_t x3;            /**< X axis, Unit:mm */
    int32_t y3;            /**< Y axis, Unit:mm */
    int32_t z3;            /**< Z axis, Unit:mm */
    uint8_t reflectivity3; /**< Reflectivity */
    uint8_t tag3;          /**< Tag */
} LivoxTripleExtendRawPoint;

/** Triple extend spherical coordinate format. */
typedef struct
{
    uint16_t theta;        /**< Zenith angle[0, 18000], Unit: 0.01 degree */
    uint16_t phi;          /**< Azimuth[0, 36000], Unit: 0.01 degree */
    uint32_t depth1;       /**< Depth, Unit: mm */
    uint8_t reflectivity1; /**< Reflectivity */
    uint8_t tag1;          /**< Tag */
    uint32_t depth2;       /**< Depth, Unit: mm */
    uint8_t reflectivity2; /**< Reflectivity */
    uint8_t tag2;          /**< Tag */
    uint32_t depth3;       /**< Depth, Unit: mm */
    uint8_t reflectivity3; /**< Reflectivity */
    uint8_t tag3;          /**< Tag */
} LivoxTripleExtendSpherPoint;

/** IMU data format. */
typedef struct
{
    float gyro_x; /**< Gyroscope X axis, Unit:rad/s */
    float gyro_y; /**< Gyroscope Y axis, Unit:rad/s */
    float gyro_z; /**< Gyroscope Z axis, Unit:rad/s */
    float acc_x;  /**< Accelerometer X axis, Unit:g */
    float acc_y;  /**< Accelerometer Y axis, Unit:g */
    float acc_z;  /**< Accelerometer Z axis, Unit:g */
} LivoxImuPoint;

typedef struct
{
    uint8_t signature[16];
    uint8_t version[4];
    uint32_t magic_code;
} LvxFilePublicHeader;

typedef struct
{
    uint32_t frame_duration;
    uint8_t device_count;
} LvxFilePrivateHeader;

typedef struct
{
    uint8_t lidar_broadcast_code[16];
    uint8_t hub_broadcast_code[16];
    uint8_t device_index;
    uint8_t device_type;
    uint8_t extrinsic_enable;
    float roll;
    float pitch;
    float yaw;
    float x;
    float y;
    float z;
} LvxDeviceInfo;

typedef struct
{
    uint8_t device_index;
    uint8_t version;
    uint8_t port_id;
    uint8_t lidar_index;
    uint8_t rsvd;
    uint32_t error_code;
    uint8_t timestamp_type;
    uint8_t data_type;
    uint8_t timestamp[8];
    uint8_t raw_point[kMaxPointSize];
    // uint8_t raw_point[RAW_POINT_NUM * sizeof(LivoxRawPoint)];
    uint32_t pack_size;
} LvxBasePackDetail;

typedef struct
{
    uint64_t current_offset;
    uint64_t next_offset;
    uint64_t frame_index;
} FrameHeader;

#pragma pack()

int read_lvx_file(char *filename);

#endif
