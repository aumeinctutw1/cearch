#ifndef _H_CAS
#define _H_CAS

#include <string>

class ContentAddressedStorage {
    public:
        ContentAddressedStorage(const std::string &storage_dir);
        ~ContentAddressedStorage() = default;

        std::string store(const std::string &content);
        std::string load(const std::string &hash) const;

        bool exists(const std::string &hash) const;

    private:
        std::string m_storage_dir;
};

#endif