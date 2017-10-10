#include <iostream>
#include <random>
#include <cassert>
#include "flatbuffers/flatbuffers.h"
#include "flatbuffers/idl.h"
#include "flatbuffers/util.h"
#include "manifest_generated.h"  // 参考自 http://blog.csdn.net/menggucaoyuan/article/details/34409433

//
namespace fb = flatbuffers;
#define fb_offset                 fb::Offset
#define fb_string                 fb::String
#define fb_vector                 fb::Vector
#define fb_table                  fb::Table
#define fb_builder                fb::FlatBufferBuilder
#define fb_create_string(b, ...)  (b).CreateString(__VA_ARGS__)
#define fb_create_vector(b, ...)  (b).CreateVector(__VA_ARGS__)
#define fb_vector_size(v)         (unsigned)(*(v)).Length()
#define fb_vector_length(v)       (unsigned)(*(v)).Length()
#define fb_vector_at(v, i)        (*(v)).Get(i)
#define fb_get_buf(b)             (b).GetBufferPointer()
#define fb_get_size(b)            (unsigned)(b).GetSize()
#define fb_clear(b)               (b).Clear()
#define fb_finish(b, buf)         (b).Finish(buf)

#define TEST_OUTPUT_LINE(...) { printf(__VA_ARGS__); printf("\n"); }

template<typename T, typename U>
void TestEq(T expval, U val, const char *exp, const char *file, int line) {
    if (expval != val) {
        auto expval_str = flatbuffers::NumToString(expval);
        auto val_str = flatbuffers::NumToString(val);
        TEST_OUTPUT_LINE("TEST FAILED: %s:%d, %s (%s) != %s", file, line,
            exp, expval_str.c_str(), val_str.c_str());
        assert(0);
    }
}

#define TEST_EQ(exp, val) TestEq(exp,         val,   #exp, __FILE__, __LINE__)
#define TEST_NOTNULL(exp) TestEq(exp == NULL, false, #exp, __FILE__, __LINE__)


void decode_manifest(const char* data_buf, int len)
{
    // First, verify the buffers integrity (optional)
    flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t *>(data_buf), len);
    TEST_EQ(VerifyManifestDataBuffer(verifier), true);

    unsigned idx = 0;
    const ManifestData * mani_ptr = NULL;
    const AssetData * asset_ptr = NULL;
    unsigned asset_vector_size = 0;
    const fb_vector<fb_offset<AssetData>>* asset_vector = NULL;
 
    if (!data_buf || !len)
    {
    return;
    }
  
    printf("buf size:%u\n", len);
    mani_ptr = fb::GetRoot<ManifestData>(data_buf);
    asset_vector = mani_ptr->assets();
    asset_vector_size = fb_vector_size(asset_vector);
    printf("asset info vector size:%u\n", asset_vector_size);
    printf("appVersion:%s\n", mani_ptr->appVersion()->c_str());
    printf("resourceVersion:%s\n", mani_ptr->resourceVersion()->c_str());

    for(idx = 0; idx < asset_vector_size; idx++)
    {
        asset_ptr = fb_vector_at(asset_vector, idx);
        printf("info idx=%d{path = %s, crc32 = %u, size = %.2f, compressed = %d, state = %d}\n", 
              idx,
              asset_ptr->path()->c_str(),
              asset_ptr->crc32(),
              asset_ptr->size(),
              asset_ptr->compressed(),
              asset_ptr->state());
    }
    return;
}

#define ARRAY_SIZE 37

std::string encode_manifest()
{
    flatbuffers::FlatBufferBuilder builder;    
    unsigned idx = 0;
    char appVersion[32] = "0.0.0.0";
    char resVersion[32] = "50235";
    char path[32] = "";
    unsigned crc32 = 0;
    float size = 0.0;
    bool  compressed = false;
    DownloadState eState = DownloadState_Unknown;
    
    fb_offset<fb_string> fb_appVersion;
    fb_offset<fb_string> fb_resVersion;
    fb_offset<fb_string> fb_path;
    fb_offset<AssetData> info[ARRAY_SIZE];
    fb_offset<fb_vector<fb_offset<AssetData>>> info_array;
    fb_offset<ManifestData> mani_data;
    
    fb_appVersion = fb_create_string(builder, appVersion, strlen(appVersion));
    fb_resVersion = fb_create_string(builder, resVersion, strlen(resVersion));
    
    for (idx = 0; idx < ARRAY_SIZE; idx++)
    {
        sprintf(path, "src/%d", idx);
        fb_path = fb_create_string(builder, path, strlen(path));
        
        crc32 = idx;
        size = 2.0 * idx;
        compressed = false;
        eState = DownloadState_Unknown;     
        info[idx] = CreateAssetData(builder,  
        fb_path,
        crc32,
        size,
        compressed,
        eState);
    }
    
    info_array = fb_create_vector(builder, info, sizeof(info) / sizeof(info[0]));
    mani_data = CreateManifestData(builder,
                                fb_appVersion,
                                fb_resVersion,
                                info_array);
    fb_finish(builder, mani_data);
    
    return std::string(reinterpret_cast<const char *>(builder.GetBufferPointer()),
        builder.GetSize());;
}

int main()
{
    {
        /*
        FILE* file = fopen("version.xxx", "rb");
        fseek(file, 0L, SEEK_END);
        int length = ftell(file);
        fseek(file, 0L, SEEK_SET);
        char *data = new char[length];
        fread(data, sizeof(char), length, file);
        fclose(file);
        decode_manifest((const char*) data, length);
        delete []data;
        */
        
        std::string flatbuf = encode_manifest();
        printf("data size[%ld]\n",flatbuf.size());
        
        decode_manifest(flatbuf.c_str(), flatbuf.size());
        printf("---------------------OK---------------------\n");
        
    }
    return 0;
}

