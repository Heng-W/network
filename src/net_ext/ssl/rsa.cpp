
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

namespace net
{
namespace ssl
{
namespace rsa
{

constexpr int kKeyLen = 2048; // 密钥长度

// 制造密钥对：私钥和公钥
std::pair<std::string, std::string> generateKey()
{
    size_t pri_len = 0; // 私钥长度
    size_t pub_len = 0; // 公钥长度
    char* pri_key = nullptr; // 私钥
    char* pub_key = nullptr; // 公钥

    // 生成密钥对
    RSA* keypair = RSA_generate_key(kKeyLen, RSA_3, NULL, NULL);

    BIO* pri = BIO_new(BIO_s_mem());
    BIO* pub = BIO_new(BIO_s_mem());

    // 生成私钥
    PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
    // 注意------生成第1种格式的公钥
    //PEM_write_bio_RSAPublicKey(pub, keypair);
    // 注意------生成第2种格式的公钥（此处代码中使用这种）
    PEM_write_bio_RSA_PUBKEY(pub, keypair);

    // 获取长度
    pri_len = BIO_pending(pri);
    pub_len = BIO_pending(pub);

    // 密钥对读取到字符串
    pri_key = (char*)malloc(pri_len + 1);
    pub_key = (char*)malloc(pub_len + 1);

    BIO_read(pri, pri_key, pri_len);
    BIO_read(pub, pub_key, pub_len);

    pri_key[pri_len] = '\0';
    pub_key[pub_len] = '\0';

    std::pair<std::string, std::string> keyPair;
    keyPair.first = pri_key;
    keyPair.second = pub_key;

    // 释放内存
    RSA_free(keypair);
    BIO_free_all(pri);
    BIO_free_all(pub);

    free(pri_key);
    free(pub_key);

    return keyPair;
}


// 制造并保存密钥对
void generateKey(const std::string& fileName)
{
    auto key = generateKey();

    // 将私钥写入文件
    std::ofstream pri_file(fileName, std::ios::out);
    if (!pri_file.is_open())
    {
        perror("private key file open fail");
        return;
    }
    pri_file << key.first;
    pri_file.close();

    // 将公钥写入文件
    std::ofstream pub_file(fileName + ".pub", std::ios::out);
    if (!pub_file.is_open())
    {
        perror("public key file open fail");
        return;
    }
    pub_file << key.second;
    pub_file.close();
}


/**
 * @brief   私钥加密
 * @param   plainText   需要进行加密的明文
 *          privateKey  私钥
 * @return  加密后的数据
 */
std::string privateEncrypt(const std::string& plainText, const std::string& privateKey)
{
    std::string encrypt_text;
    BIO* keybio = BIO_new_mem_buf((unsigned char*)privateKey.c_str(), -1);
    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    // 获取RSA单次可以处理的数据块的最大长度
    int key_len = RSA_size(rsa);
    int block_len = key_len - 11;    // 因为填充方式为RSA_PKCS1_PADDING, 所以要在key_len基础上减去11

    // 申请内存：存贮加密后的密文数据
    char* sub_text = new char[key_len + 1];
    memset(sub_text, 0, key_len + 1);
    int ret = 0;
    size_t pos = 0;
    std::string sub_str;
    // 对数据进行分段加密（返回值是加密后数据的长度）
    while (pos < plainText.length())
    {
        sub_str = plainText.substr(pos, block_len);
        memset(sub_text, 0, key_len + 1);
        ret = RSA_private_encrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
        if (ret >= 0)
        {
            encrypt_text.append(std::string(sub_text, ret));
        }
        pos += block_len;
    }

    // 释放内存
    delete sub_text;
    BIO_free_all(keybio);
    RSA_free(rsa);

    return encrypt_text;
}


/**
 * @brief   公钥解密
 * @param   cipherText 加密的密文
 *          publicKey  公钥
 * @return  解密后的数据
 */
std::string publicDecrypt(const std::string& cipherText, const std::string& publicKey)
{
    std::string decrypt_text;
    BIO* keybio = BIO_new_mem_buf((unsigned char*)publicKey.c_str(), -1);
    RSA* rsa = RSA_new();

    // 注意-------使用第1种格式的公钥进行解密
    //rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);
    // 注意-------使用第2种格式的公钥进行解密（我们使用这种格式作为示例）
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);
    if (!rsa)
    {
        unsigned long err = ERR_get_error(); //获取错误号
        char err_msg[1024] = { 0 };
        ERR_error_string(err, err_msg); // 格式：error:errId:库:函数:原因
        printf("err msg: err:%ld, msg:%s\n", err, err_msg);
        BIO_free_all(keybio);

        return decrypt_text;
    }

    // 获取RSA单次处理的最大长度
    int len = RSA_size(rsa);
    char* sub_text = new char[len + 1];
    memset(sub_text, 0, len + 1);
    int ret = 0;
    std::string sub_str;
    size_t pos = 0;
    // 对密文进行分段解密
    while (pos < cipherText.length())
    {
        sub_str = cipherText.substr(pos, len);
        memset(sub_text, 0, len + 1);
        ret = RSA_public_decrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
        if (ret >= 0)
        {
            decrypt_text.append(std::string(sub_text, ret));
            //printf("pos:%d, sub: %s\n", pos, sub_text);
            pos += len;
        }
    }

    // 释放内存
    delete sub_text;
    BIO_free_all(keybio);
    RSA_free(rsa);

    return decrypt_text;
}

/**
 * @brief   公钥加密
 * @param   plainText   需要进行加密的明文
 *          publicKey   公钥
 * @return  加密后的数据
 */
std::string publicEncrypt(const std::string& plainText, const std::string& publicKey)
{
    std::string encrypt_text;
    BIO* keybio = BIO_new_mem_buf((unsigned char*)publicKey.c_str(), -1);
    RSA* rsa = RSA_new();
    // 注意-----第1种格式的公钥
    //rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);
    // 注意-----第2种格式的公钥（这里以第二种格式为例）
    rsa = PEM_read_bio_RSA_PUBKEY(keybio, &rsa, NULL, NULL);

    // 获取RSA单次可以处理的数据块的最大长度
    int dataLen = RSA_size(rsa);
    int blockLen = dataLen - 11;    // 因为填充方式为RSA_PKCS1_PADDING, 所以要在dataLen基础上减去11

    // 申请内存：存贮加密后的密文数据
    char* sub_text = new char[dataLen + 1];
    memset(sub_text, 0, dataLen + 1);
    int ret = 0;
    size_t pos = 0;
    std::string sub_str;
    // 对数据进行分段加密（返回值是加密后数据的长度）
    while (pos < plainText.length())
    {
        sub_str = plainText.substr(pos, blockLen);
        memset(sub_text, 0, dataLen + 1);
        ret = RSA_public_encrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
        if (ret >= 0)
        {
            encrypt_text.append(std::string(sub_text, ret));
        }
        pos += blockLen;
    }

    // 释放内存
    BIO_free_all(keybio);
    RSA_free(rsa);
    delete[] sub_text;

    return encrypt_text;
}

/**
 * @brief   私钥解密
 * @param   cipherText   加密的密文
 *          privateKey   私钥
 * @return  解密后的数据
 */
std::string privateDecrypt(const std::string& cipherText, const std::string& privateKey)
{
    std::string decrypt_text;
    RSA* rsa = RSA_new();
    BIO* keybio;
    keybio = BIO_new_mem_buf((unsigned char*)privateKey.c_str(), -1);

    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    if (rsa == nullptr)
    {
        unsigned long err = ERR_get_error(); //获取错误号
        char err_msg[1024] = { 0 };
        ERR_error_string(err, err_msg); // 格式：error:errId:库:函数:原因
        printf("err msg: err:%ld, msg:%s\n", err, err_msg);
        return std::string();
    }

    // 获取RSA单次处理的最大长度
    int dataLen = RSA_size(rsa);
    char* sub_text = new char[dataLen + 1]();
    int ret = 0;
    std::string sub_str;
    size_t pos = 0;
    // 对密文进行分段解密
    while (pos < cipherText.length())
    {
        sub_str = cipherText.substr(pos, dataLen);
        memset(sub_text, 0, dataLen + 1);
        ret = RSA_private_decrypt(sub_str.length(), (const unsigned char*)sub_str.c_str(), (unsigned char*)sub_text, rsa, RSA_PKCS1_PADDING);
        if (ret >= 0)
        {
            decrypt_text.append(std::string(sub_text, ret));
            // printf("pos:%d, sub: %s\n", pos, sub_text);
            pos += dataLen;
        }
    }
    // 释放内存
    delete[] sub_text;
    BIO_free_all(keybio);
    RSA_free(rsa);

    return decrypt_text;
}


} // namespace rsa
} // namespace ssl
} // namespace net

#if 0

int main()
{
    // 原始明文
    std::string src_text = "test begin\n this is an rsa test example!!! \
    this is an rsa test example!!! this is an rsa test example!!! \
    this is an rsa test example!!! this is an rsa test example!!! \ntest end";
    //src_text = "rsa test";

    std::string encrypt_text;
    std::string decrypt_text;

    // 生成密钥对
    std::string publicKey;
    std::string privateKey;
    rsa::generateKey(publicKey, privateKey);
    printf("public key:\n");
    printf("%s\n", publicKey.c_str());
    printf("private key:\n");
    printf("%s\n", privateKey.c_str());

    // 私钥加密-公钥解密
    encrypt_text = rsa::privateEncrypt(src_text, privateKey);
    printf("encrypt: len=%d\n", encrypt_text.length());
    decrypt_text = rsa::publicDecrypt(encrypt_text, publicKey);
    printf("decrypt: len=%d\n", decrypt_text.length());
    std::cout << "aaaa: " << encrypt_text << std::endl;
    printf("decrypt: %s\n", decrypt_text.c_str());

    // 公钥加密-私钥解密
    encrypt_text = rsa::publicEncrypt(src_text, publicKey);
    printf("encrypt: len=%d\n", encrypt_text.length());
    decrypt_text = rsa::privateDecrypt(encrypt_text, privateKey);
    printf("decrypt: len=%d\n", decrypt_text.length());
    printf("decrypt: %s\n", decrypt_text.c_str());

    return 0;
}

#endif
