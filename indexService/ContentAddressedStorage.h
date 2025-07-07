#ifndef _H_CAS
#define _H_CAS

#include <string>

#include <zlib.h>

/*
*   Stores files in a directory by the files hash, files are compressed before storage.
*   The Hash of the original file content is used for storage
*/
class ContentAddressedStorage {
    public:
        ContentAddressedStorage(const std::string &storage_dir);
        ~ContentAddressedStorage() = default;

        /* stores content and returns hash */
        std::string store(const std::string &content);
        /* searches hash and returns uncompressed content */
        std::string load(const std::string &hash) const;

        bool exists(const std::string &hash) const;

    private:
        std::string m_storage_dir;

        std::string compute_sha256(const std::string &data) const;
        std::vector<Bytef> compress_content(const std::string &data) const;
        std::string decompress_content(std::vector<Bytef> &data) const;
};

#endif