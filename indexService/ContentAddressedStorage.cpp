#include <sstream>
#include <fstream>
#include <iomanip>

#include <openssl/sha.h>

#include "ContentAddressedStorage.h"

ContentAddressedStorage::ContentAddressedStorage(const std::string &storage_dir)
    :m_storage_dir(storage_dir) 
{
    /* remove trailing / from storage dir */
}

std::string ContentAddressedStorage::store(const std::string &content) {
    std::vector<Bytef> compressed_content;
    std::string hash = compute_sha256(content);

    try {
        compressed_content = compress_content(content); 
    } catch (std::exception &e) {
        throw std::runtime_error("Compressing the content failed during storage");
    }

    /* store in filesystem */
    std::string filepath = m_storage_dir + "/" + hash + ".z";

    /* TODO: can this fail? */
    std::ofstream out(filepath, std::ios::binary);
    out.write(reinterpret_cast<char*>(compressed_content.data()), compressed_content.size());

    return hash;
}

std::string ContentAddressedStorage::load(const std::string &hash) const {

    //std::ifstream in(compressed_content_path, std::ios::binary);
    //std::vector<Bytef> compressed_data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return "";
}

bool ContentAddressedStorage::exists(const std::string &hash) const {
    return true;
}


std::string ContentAddressedStorage::compute_sha256(const std::string &data) const {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    return oss.str();
}

std::vector<Bytef> ContentAddressedStorage::compress_content(const std::string &data) const {
    uLong src_len = data.size();
    uLong dest_len = compressBound(src_len);
    std::vector<Bytef> compressed_data(dest_len);

    int res = compress(compressed_data.data(), &dest_len, reinterpret_cast<const Bytef*>(data.data()), src_len);

    if (res != Z_OK) {
        throw std::runtime_error("Compression failed");
    }

    return compressed_data;
}

std::string ContentAddressedStorage::decompress_content(std::vector<Bytef> &data) const {
    /* 10 MB Buffer */
    uLongf decompressed_size = 10 * 1024 * 1024; 
    std::vector<char> decompressed(decompressed_size);

    int res = uncompress(reinterpret_cast<Bytef*>(decompressed.data()), &decompressed_size, data.data(), data.size());

    if (res != Z_OK) {
        throw std::runtime_error("Failed to decompress document content.");
    }

    return std::string(decompressed.data(), decompressed_size);
}