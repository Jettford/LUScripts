#include <iostream>
#include <fstream>
#include <vector>

struct ChunkHeader {
	uint32_t type;
	uint32_t unknown;
	uint32_t length;
	uint32_t offset;
};

struct InfoChunk {
	uint32_t version;
	uint32_t revision;
	uint32_t sky_address;
	uint32_t objects_adress;
	uint32_t env_adress;
};

inline std::unique_ptr<unsigned char[]> ReadFileCompletely(std::string filename, uint32_t* fsize = 0) {
    FILE* file = fopen(filename.c_str(), "rb");
    if (file == nullptr) return nullptr;
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    std::unique_ptr<unsigned char[]> data = std::make_unique<unsigned char[]>(size);
    fseek(file, 0, SEEK_SET);
    int bytes_read = fread(data.get(), sizeof(unsigned char), size, file);
    fclose(file);
    if (fsize != nullptr) *fsize = bytes_read;
    return data;
}

void load_and_update_file(std::string path, std::string output_path) {
    char* data = nullptr;
    bool foundParticle = false;
    uint32_t version = 0;
    uint32_t size = 0;

    std::vector<uint32_t> offsetsToStrip = std::vector<uint32_t>();

    auto dataPtr = ReadFileCompletely(path, &size);

    data = reinterpret_cast<char*>(dataPtr.get());

    uint32_t i = 0;
    while (i < size) {
        i += (i % 16);

        char* ChunkMagic = (data + i);
        if (std::string(ChunkMagic, 4) != "CHNK") {
            if (i != 0) {
                throw "We appear to have hit a invalid chunk or the alignment is incorrect";
            }
            return;
        }

        ChunkHeader* header = reinterpret_cast<ChunkHeader*>(data + i + 4);

        if (header->type == 2002) { // particle
            foundParticle = true;

            i = header->offset;

            // read in the particle data
            uint32_t particleSize = *reinterpret_cast<uint32_t*>(data + i);
            i += 4;

            header->length = header->length - (particleSize * 2);

            for (uint32_t j = 0; particleSize > j; j++) {
                if (version >= 43 && j == 0) i += 2;

                i += 28;

                uint32_t nextStrSize = *reinterpret_cast<uint32_t*>(data + i);

                i += 4 + (nextStrSize * 2);

                offsetsToStrip.push_back(i);
                offsetsToStrip.push_back(i + 1);

                i += 2;

                nextStrSize = *reinterpret_cast<uint32_t*>(data + i);

                i += 4 + (nextStrSize * 2);

                std::cout << "Changed stuff" << std::endl;
            }

        } else if (header->type == 1000) {
            version = reinterpret_cast<InfoChunk*>(data + header->offset)->version;
            if (version < 46) {
                return;
            }
            reinterpret_cast<InfoChunk*>(data + header->offset)->version = 45;
        }

        i += header->length;
    }

    std::ofstream ofs;
    ofs.open(output_path, std::ios::binary | std::ios::out);
    for (uint32_t j = 0; size > j; j++) {
        bool breakOut = false;
        for (auto i: offsetsToStrip) {
            if (j == i) {
                breakOut = true;
                break;
            }
        }
        if (breakOut) continue;
        ofs << *reinterpret_cast<char*>(data + j);
    }
    ofs.close();
}

// main function
int main(int argc, char** argv) {
    // read in the file
    
    load_and_update_file(std::string(argv[1]), std::string(argv[2]));

    return 0;
}
