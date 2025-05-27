#ifndef ANALYZEMFT_CONSTANTS_H
#define ANALYZEMFT_CONSTANTS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef __AVX2__
#define SIMD_OPTIMIZED 1
#endif

#ifdef __SSE4_2__
#define CRC32_HARDWARE 1
#endif

constexpr uint16_t FILE_RECORD_IN_USE = 0x0001;
constexpr uint16_t FILE_RECORD_IS_DIRECTORY = 0x0002;
constexpr uint16_t FILE_RECORD_IS_EXTENSION = 0x0004;
constexpr uint16_t FILE_RECORD_HAS_SPECIAL_INDEX = 0x0008;

constexpr uint32_t STANDARD_INFORMATION_ATTRIBUTE = 0x10;
constexpr uint32_t ATTRIBUTE_LIST_ATTRIBUTE = 0x20;
constexpr uint32_t FILE_NAME_ATTRIBUTE = 0x30;
constexpr uint32_t OBJECT_ID_ATTRIBUTE = 0x40;
constexpr uint32_t SECURITY_DESCRIPTOR_ATTRIBUTE = 0x50;
constexpr uint32_t VOLUME_NAME_ATTRIBUTE = 0x60;
constexpr uint32_t VOLUME_INFORMATION_ATTRIBUTE = 0x70;
constexpr uint32_t DATA_ATTRIBUTE = 0x80;
constexpr uint32_t INDEX_ROOT_ATTRIBUTE = 0x90;
constexpr uint32_t INDEX_ALLOCATION_ATTRIBUTE = 0xA0;
constexpr uint32_t BITMAP_ATTRIBUTE = 0xB0;
constexpr uint32_t REPARSE_POINT_ATTRIBUTE = 0xC0;
constexpr uint32_t EA_INFORMATION_ATTRIBUTE = 0xD0;
constexpr uint32_t EA_ATTRIBUTE = 0xE0;
constexpr uint32_t LOGGED_UTILITY_STREAM_ATTRIBUTE = 0x100;

const std::unordered_map<uint32_t, std::string> ATTRIBUTE_NAMES = {
    {STANDARD_INFORMATION_ATTRIBUTE, "$STANDARD_INFORMATION"},
    {ATTRIBUTE_LIST_ATTRIBUTE, "$ATTRIBUTE_LIST"},
    {FILE_NAME_ATTRIBUTE, "$FILE_NAME"},
    {OBJECT_ID_ATTRIBUTE, "$OBJECT_ID"},
    {SECURITY_DESCRIPTOR_ATTRIBUTE, "$SECURITY_DESCRIPTOR"},
    {VOLUME_NAME_ATTRIBUTE, "$VOLUME_NAME"},
    {VOLUME_INFORMATION_ATTRIBUTE, "$VOLUME_INFORMATION"},
    {DATA_ATTRIBUTE, "$DATA"},
    {INDEX_ROOT_ATTRIBUTE, "$INDEX_ROOT"},
    {INDEX_ALLOCATION_ATTRIBUTE, "$INDEX_ALLOCATION"},
    {BITMAP_ATTRIBUTE, "$BITMAP"},
    {REPARSE_POINT_ATTRIBUTE, "$REPARSE_POINT"},
    {EA_INFORMATION_ATTRIBUTE, "$EA_INFORMATION"},
    {EA_ATTRIBUTE, "$EA"},
    {LOGGED_UTILITY_STREAM_ATTRIBUTE, "$LOGGED_UTILITY_STREAM"}
};

constexpr size_t MFT_RECORD_SIZE = 1024;
constexpr uint32_t MFT_RECORD_MAGIC = 0x454C4946; // 'FILE'

constexpr size_t MFT_RECORD_MAGIC_NUMBER_OFFSET = 0;
constexpr size_t MFT_RECORD_UPDATE_SEQUENCE_OFFSET = 4;
constexpr size_t MFT_RECORD_UPDATE_SEQUENCE_SIZE_OFFSET = 6;
constexpr size_t MFT_RECORD_LOGFILE_SEQUENCE_NUMBER_OFFSET = 8;
constexpr size_t MFT_RECORD_SEQUENCE_NUMBER_OFFSET = 16;
constexpr size_t MFT_RECORD_HARD_LINK_COUNT_OFFSET = 18;
constexpr size_t MFT_RECORD_FIRST_ATTRIBUTE_OFFSET = 20;
constexpr size_t MFT_RECORD_FLAGS_OFFSET = 22;
constexpr size_t MFT_RECORD_USED_SIZE_OFFSET = 24;
constexpr size_t MFT_RECORD_ALLOCATED_SIZE_OFFSET = 28;
constexpr size_t MFT_RECORD_FILE_REFERENCE_OFFSET = 32;
constexpr size_t MFT_RECORD_NEXT_ATTRIBUTE_ID_OFFSET = 40;
constexpr size_t MFT_RECORD_RECORD_NUMBER_OFFSET = 44;

const std::vector<std::string> CSV_HEADER = {
    "Record Number", "Record Status", "Record Type", "File Type", "Sequence Number",
    "Parent Record Number", "Parent Record Sequence Number", "Filename", "Filepath",
    "SI Creation Time", "SI Modification Time", "SI Access Time", "SI Entry Time",
    "FN Creation Time", "FN Modification Time", "FN Access Time", "FN Entry Time",
    "Object ID", "Birth Volume ID", "Birth Object ID", "Birth Domain ID",
    "Has Standard Information", "Has Attribute List", "Has File Name", "Has Volume Name",
    "Has Volume Information", "Has Data", "Has Index Root", "Has Index Allocation",
    "Has Bitmap", "Has Reparse Point", "Has EA Information", "Has EA",
    "Has Logged Utility Stream", "Attribute List Details", "Security Descriptor",
    "Volume Name", "Volume Information", "Data Attribute", "Index Root",
    "Index Allocation", "Bitmap", "Reparse Point", "EA Information", "EA",
    "Logged Utility Stream", "MD5", "SHA256", "SHA512", "CRC32"
};

#endif