#ifndef NET_SSL_AES_H
#define NET_SSL_AES_H

#include <string>

namespace net
{
namespace ssl
{

// 随机生成密钥
std::string generateAESKey();

int paddingSizeForAES(int len);

int encryptAES(const void* in, int inLen, void* out, int outLen, const std::string& key);

int decryptAES(const void* in, void* out, int len, const std::string& key);


} // namespace ssl
} // namespace net

#endif // NET_SSL_AES_H
