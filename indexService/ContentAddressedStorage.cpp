#include "ContentAddressedStorage.h"

ContentAddressedStorage::ContentAddressedStorage(const std::string &storage_dir)
    :m_storage_dir(storage_dir) 
{

}

std::string ContentAddressedStorage::store(const std::string &content) {
    return "";
}

std::string ContentAddressedStorage::load(const std::string &hash) const {
    return "";
}

bool ContentAddressedStorage::exists(const std::string &hash) const {
    return true;
}