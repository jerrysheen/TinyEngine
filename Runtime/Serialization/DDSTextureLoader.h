#pragma once
#include "AssetHeader.h"
#include "StreamHelper.h"
#include "Resources/IResourceLoader.h"
#include "Graphics/Texture.h"
#include "Settings/ProjectSettings.h"
#include "StreamHelper.h"
#include "Graphics/IGPUResource.h"
#include <algorithm>  // for std::min, std::max
/*
Can load easier and more indepth with https://github.com/Hydroque/DDSLoader
Because a lot of crappy, weird DDS file loader files were found online. The resources are actually VERY VERY limited.
Written in C, can very easily port to C++ through casting mallocs (ensure your imports are correct), goto can be replaced.
https://www.gamedev.net/forums/topic/637377-loading-dds-textures-in-opengl-black-texture-showing/
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-5-a-textured-cube/
^ Two examples of terrible code
https://gist.github.com/Hydroque/d1a8a46853dea160dc86aa48618be6f9
^ My first look and clean up 'get it working'
https://ideone.com/WoGThC
^ Improvement details
File Structure:
  Section     Length
  ///////////////////
  FILECODE    4
  HEADER      124
  HEADER_DX10* 20	(https://msdn.microsoft.com/en-us/library/bb943983(v=vs.85).aspx)
  PIXELS      fseek(f, 0, SEEK_END); (ftell(f) - 128) - (fourCC == "DX10" ? 17 or 20 : 0)
* the link tells you that this section isn't written unless its a DX10 file
Supports DXT1, DXT3, DXT5.
The problem with supporting DX10 is you need to know what it is used for and how opengl would use it.
File Byte Order:
typedef unsigned int DWORD;           // 32bits little endian
  type   index    attribute           // description
///////////////////////////////////////////////////////////////////////////////////////////////
  DWORD  0        file_code;          //. always `DDS `, or 0x20534444
  DWORD  4        size;               //. size of the header, always 124 (includes PIXELFORMAT)
  DWORD  8        flags;              //. bitflags that tells you if data is present in the file
                                      //      CAPS         0x1
                                      //      HEIGHT       0x2
                                      //      WIDTH        0x4
                                      //      PITCH        0x8
                                      //      PIXELFORMAT  0x1000
                                      //      MIPMAPCOUNT  0x20000
                                      //      LINEARSIZE   0x80000
                                      //      DEPTH        0x800000
  DWORD  12       height;             //. height of the base image (biggest mipmap)
  DWORD  16       width;              //. width of the base image (biggest mipmap)
  DWORD  20       pitchOrLinearSize;  //. bytes per scan line in an uncompressed texture, or bytes in the top level texture for a compressed texture
                                      //     D3DX11.lib and other similar libraries unreliably or inconsistently provide the pitch, convert with
                                      //     DX* && BC*: max( 1, ((width+3)/4) ) * block-size
                                      //     *8*8_*8*8 && UYVY && YUY2: ((width+1) >> 1) * 4
                                      //     (width * bits-per-pixel + 7)/8 (divide by 8 for byte alignment, whatever that means)
  DWORD  24       depth;              //. Depth of a volume texture (in pixels), garbage if no volume data
  DWORD  28       mipMapCount;        //. number of mipmaps, garbage if no pixel data
  DWORD  32       reserved1[11];      //. unused
  DWORD  76       Size;               //. size of the following 32 bytes (PIXELFORMAT)
  DWORD  80       Flags;              //. bitflags that tells you if data is present in the file for following 28 bytes
                                      //      ALPHAPIXELS  0x1
                                      //      ALPHA        0x2
                                      //      FOURCC       0x4
                                      //      RGB          0x40
                                      //      YUV          0x200
                                      //      LUMINANCE    0x20000
  DWORD  84       FourCC;             //. File format: DXT1, DXT2, DXT3, DXT4, DXT5, DX10. 
  DWORD  88       RGBBitCount;        //. Bits per pixel
  DWORD  92       RBitMask;           //. Bit mask for R channel
  DWORD  96       GBitMask;           //. Bit mask for G channel
  DWORD  100      BBitMask;           //. Bit mask for B channel
  DWORD  104      ABitMask;           //. Bit mask for A channel
  DWORD  108      caps;               //. 0x1000 for a texture w/o mipmaps
                                      //      0x401008 for a texture w/ mipmaps
                                      //      0x1008 for a cube map
  DWORD  112      caps2;              //. bitflags that tells you if data is present in the file
                                      //      CUBEMAP           0x200     Required for a cube map.
                                      //      CUBEMAP_POSITIVEX 0x400     Required when these surfaces are stored in a cube map.
                                      //      CUBEMAP_NEGATIVEX 0x800     ^
                                      //      CUBEMAP_POSITIVEY 0x1000    ^
                                      //      CUBEMAP_NEGATIVEY 0x2000    ^
                                      //      CUBEMAP_POSITIVEZ 0x4000    ^
                                      //      CUBEMAP_NEGATIVEZ 0x8000    ^
                                      //      VOLUME            0x200000  Required for a volume texture.
  DWORD  114      caps3;              //. unused
  DWORD  116      caps4;              //. unused
  DWORD  120      reserved2;          //. unused
*/

namespace EngineCore{
    struct DDSHeader {
        uint32_t magic;              // 'DDS ' (0x20534444)
        uint32_t fileSize;               // 124
        uint32_t flags;
        uint32_t height;
        uint32_t width;
        uint32_t pitchOrLinearSize;
        uint32_t depth;
        uint32_t mipMapCount;
        uint32_t reserved1[11];
        uint32_t size;           // 应该是32
        uint32_t flagsData;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
        uint32_t caps;
        uint32_t caps2;
        uint32_t caps3;
        uint32_t caps4;
        uint32_t reserved2;
    };

    struct DDS_HEADER_DXT10 {
        uint32_t dxgiFormat;      // DXGI_FORMAT枚举值
        uint32_t resourceDimension; // D3D11_RESOURCE_DIMENSION
        uint32_t miscFlag;        // D3D11_RESOURCE_MISC_FLAG
        uint32_t arraySize;       // 数组大小
        uint32_t miscFlags2;      // 额外标志
    };

    struct DDSLoadResult {
        bool success = false;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t mipMapCount = 0;
        TextureFormat format;        // DXT1/DXT3/DXT5标识
        uint32_t blockSize = 0;     // 8 for DXT1, 16 for DXT3/DXT5
        std::vector<uint8_t> pixelData;
    };

    class DDSTextureLoader : public IResourceLoader
    {
    public:
        virtual ~DDSTextureLoader() = default;
        virtual Resource* Load(const std::string& relativePath) override
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
    
            Texture* tex = new Texture();
            tex->SetAssetCreateMethod(AssetCreateMethod::Serialization);
            tex->SetAssetID(AssetIDGenerator::NewFromFile(path));
            
            DDSLoadResult ddsResult = LoadDDSFromFile(relativePath);
            tex->textureDesc.format = ddsResult.format;
            tex->textureDesc.width = ddsResult.width;
            tex->textureDesc.height = ddsResult.height;
            tex->textureDesc.dimension = TextureDimension::TEXTURE2D;
            tex->textureDesc.texUsage = TextureUsage::ShaderResource;
            tex->textureDesc.mipCount = ddsResult.mipMapCount;
            tex->cpuData = ddsResult.pixelData;
            // 计算mip Count
            uint32_t offset = 0;
            tex->textureDesc.mipOffset[0] = 0;  // 第 0 级从 0 开始
            int width = ddsResult.width;
            int height = ddsResult.height;
            // ++i 和 i++在循环体中区别不大， 因为循环跑完才会走++操作，
            // 不过针对迭代器，++i更好， 因为i++相当于要返回一个原值，并且在原来的迭代器上叠加
            for (uint32_t i = 0; i < ddsResult.mipMapCount; ++i)
            {
                // 当前 mip level 的尺寸
                uint32_t mipWidth = std::max(1, width >> i);
                uint32_t mipHeight = std::max(1, height >> i);
                
                // 计算当前 mip level 的字节大小
                uint32_t mipSize = CalculateDXTMipSize(mipWidth, mipHeight, ddsResult.blockSize);
                
                if (i > 0) {
                    tex->textureDesc.mipOffset[i] = offset;  // 记录当前 mip 的 offset
                }
                
                offset += mipSize;  // 累加到下一个 mip level
            }


            return tex;
        }
    
        std::vector<uint8_t> LoadMipData(const std::string& realativePath, int mipCount){}
    
    
        int CalculateDXTMipSize(uint32_t width, uint32_t height, uint32_t blockSize)
        {
            //DXT1 (BC1)：每 4×4 块占 8 字节
            //DXT3 (BC2)：每 4×4 块占 16 字节
            //DXT5 (BC3)：每 4×4 块占 16 字节
            uint32_t blockWidth = (width + 3) / 4;
            uint32_t blockHeight = (height + 3) / 4;
            return blockWidth * blockHeight * blockSize;
        }
    
    private:
    
    
        DDSLoadResult LoadDDSFromFile(const std::string& relativePath)
        {
            std::string path = PathSettings::ResolveAssetPath(relativePath);
            std::ifstream file(path, std::ios::binary);
            ASSERT(file.is_open());
    
            file.seekg(0, std::ios::end);      // 先移动到文件末尾
            std::streamsize fileSize = file.tellg();  // 获取文件大小
            file.seekg(0, std::ios::beg);      // 再移回文件开始
    
            ASSERT(fileSize > sizeof(DDSHeader));
    
            DDSHeader header;
            file.read(reinterpret_cast<char*>(&header), sizeof(DDSHeader));
    
            if(header.magic != 0x20534444 || header.fileSize != 124)
            {
                ASSERT(false);
            }
    
            DDSLoadResult result;
            result.width = header.width;
            result.height = header.height;
            result.mipMapCount = header.mipMapCount > 0 ? header.mipMapCount : 1;
        
            // 7. 判断压缩格式 (DXT1/DXT3/DXT5)
            uint32_t fourCC = header.fourCC;
            
            // 提取fourCC字符串来判断
            char fourCCStr[5] = {0};
            std::memcpy(fourCCStr, &fourCC, 4);
            
            if (std::memcmp(fourCCStr, "DXT1", 4) == 0) {
                result.format = TextureFormat::DXT1;  // DXT1
                result.blockSize = 8;
            }
            else if (std::memcmp(fourCCStr, "DXT3", 4) == 0) {
                result.format = TextureFormat::DXT3;  // DXT3
                result.blockSize = 16;
            }
            else if (std::memcmp(fourCCStr, "DXT5", 4) == 0) {
                result.format = TextureFormat::DXT5;  // DXT5
                result.blockSize = 16;
            }
            else if (std::memcmp(fourCCStr, "DX10", 4) == 0) {
                // 读取DX10扩展头
                DDS_HEADER_DXT10 dx10Header;
                file.read(reinterpret_cast<char*>(&dx10Header), sizeof(DDS_HEADER_DXT10));

                // 根据DXGI_FORMAT映射到您的TextureFormat
                // DXGI_FORMAT_BC1_UNORM = 71 (DXT1)
                // DXGI_FORMAT_BC2_UNORM = 74 (DXT3)
                // DXGI_FORMAT_BC3_UNORM = 77 (DXT5)
                // DXGI_FORMAT_BC7_UNORM = 98 (BC7, 更高质量)
                switch (dx10Header.dxgiFormat) {
                case 71: // DXGI_FORMAT_BC1_UNORM
                    result.format = TextureFormat::DXT1;
                    result.blockSize = 8;
                    break;
                case 74: // DXGI_FORMAT_BC2_UNORM
                    result.format = TextureFormat::DXT3;
                    result.blockSize = 16;
                    break;
                case 77: // DXGI_FORMAT_BC3_UNORM
                    result.format = TextureFormat::DXT5;
                    result.blockSize = 16;
                    break;
                case 98: // BC7_UNORM
                    result.format = TextureFormat::BC7;
                    result.blockSize = 16;
                    break;
                case 99: // BC7_UNORM_SRGB ← 您的情况
                    result.format = TextureFormat::BC7_SRGB;
                    result.blockSize = 16;
                    break;
                default:
                    ASSERT(false && "Unsupported DX10 format");
                }
            }
            else {
                ASSERT(false);  // 不支持的格式
            }
    
            // 计算像素数据大小时，需要考虑是否有DX10扩展头
            size_t headerSize = sizeof(DDSHeader);
            if (std::memcmp(fourCCStr, "DX10", 4) == 0) {
                headerSize += sizeof(DDS_HEADER_DXT10);
            }
            size_t pixelDataSize = fileSize - headerSize;
            result.pixelData.resize(pixelDataSize);
            file.read(reinterpret_cast<char*>(result.pixelData.data()), pixelDataSize);
            result.success = true;
            return result;
        }
    };


};