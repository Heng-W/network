#ifndef NET_SSL_RSA_H
#define NET_SSL_RSA_H

#include <string>
#include <utility> // for std::pair

namespace net
{
namespace ssl
{
namespace rsa
{

// 制造密钥对：私钥和公钥
std::pair<std::string, std::string> generateKey();

// 制造并保存密钥对
void generateKey(const std::string& fileName);


/**
 * @brief   私钥加密
 * @param   plainText   需要进行加密的明文
 *          privateKey  私钥
 * @return  加密后的数据
 */
std::string privateEncrypt(const std::string& plainText, const std::string& privateKey);

/**
 * @brief   公钥解密
 * @param   cipherText 加密的密文
 *          publicKey  公钥
 * @return  解密后的数据
 */
std::string publicDecrypt(const std::string& cipherText, const std::string& publicKey);

/**
 * @brief   公钥加密
 * @param   plainText   需要进行加密的明文
 *          publicKey   公钥
 * @return  加密后的数据
 */
std::string publicEncrypt(const std::string& plainText, const std::string& publicKey);

/**
 * @brief   私钥解密
 * @param   cipherText   加密的密文
 *          privateKey   私钥
 * @return  解密后的数据
 */
std::string privateDecrypt(const std::string& cipherText, const std::string& privateKey);

} // namespace rsa
} // namespace ssl
} // namespace net

#endif // NET_SSL_RSA_H
