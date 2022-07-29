
#include "aes.h"

#include <assert.h>
#include <string.h>
#include <string>
#include <openssl/aes.h>
#include <openssl/rand.h>

using namespace std;


namespace net
{
namespace ssl
{

std::string generateAESKey()
{
    std::string key(16, '\0');
    RAND_bytes((unsigned char*)key.data(), key.size());
    return key;
}

int paddingSizeForAES(int len)
{
    return AES_BLOCK_SIZE - len % AES_BLOCK_SIZE;
}

int encryptAES(const void* in, int inLen, void* out, int outLen, const std::string& key)
{
    AES_KEY aes_key;
    if (AES_set_encrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) < 0)
    {
        return -1;
    }
    const unsigned char* input = (const unsigned char*)in;
    unsigned char* output = (unsigned char*)out;
    int cnt = inLen / AES_BLOCK_SIZE;
    int remainSize = inLen % AES_BLOCK_SIZE;
    int paddingSize = AES_BLOCK_SIZE - remainSize;
    if (outLen < inLen + paddingSize) return -1;
    while (cnt--)
    {
        AES_encrypt(input, output, &aes_key);
        input += AES_BLOCK_SIZE;
        output += AES_BLOCK_SIZE;
    }
    unsigned char tmp[AES_BLOCK_SIZE];
    memcpy(tmp, input, remainSize);
    memset(tmp + remainSize, paddingSize, paddingSize); // PKCS7 Padding
    AES_encrypt(tmp, output, &aes_key);
    return inLen + paddingSize;
}

int decryptAES(const void* in, void* out, int len, const std::string& key)
{
    AES_KEY aes_key;
    if (AES_set_decrypt_key((const unsigned char*)key.data(), key.size() * 8, &aes_key) < 0)
    {
        return -1;
    }
    const unsigned char* input = (const unsigned char*)in;
    unsigned char* output = (unsigned char*)out;

    int cnt = len / AES_BLOCK_SIZE;
    while (cnt--)
    {
        AES_decrypt(input, output, &aes_key);
        input += AES_BLOCK_SIZE;
        output += AES_BLOCK_SIZE;
    }
    return len - *(output - 1);
}

} // namespace ssl
} // namespace net


#if 0

#include <iostream>

int main(int argc, char* argv[])
{
    using namespace net::ssl;
    if (argc < 2) return -1;
    string strOriginData = argv[1];
    string cur = strOriginData;
    int len = cur.size();

    cur.resize(len + 16);
    const char* data = cur.data();

    auto key = generateAESKey();
    int nRet = encryptAES(data, len, (char*)data, cur.size(), key);
    if (nRet < 0) return -1;
    cout << cur << endl;
    int actLen = decryptAES(data, (char*)data, nRet, key);
    cur.resize(actLen);

    cout << cur << endl;

    if (cur == strOriginData)
    {
        cout << "Encrypt And Decrypt Successful!" << endl;
    }
    return 0;
}
#endif