#include "crypto.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <memory>

bool generate_symmetric_key(std::string &key) {
    for (int i = 0; i < SYMMETRIC_KEY_SIZE; ++i) {
        key += (char) (rand() % 95 + 32);
    }
    return true;
}

// Function to extend the key to match the length of the data
std::string extend_key(const std::string &key, size_t data_length) {
    std::string extended_key = key;
    while (extended_key.length() < data_length) {
        extended_key += key;
    }
    extended_key.resize(data_length);
    return extended_key;
}

// XOR Encrypt or Decrypt (symmetric operation)
int xor_encrypt_decrypt(const std::string &data, const std::string &key, std::string &result) {
    if (key.empty()) {
        return -1;   // Key should not be empty
    }

    std::string extended_key = extend_key(key, data.size());
    result.resize(data.size());

    for (size_t i = 0; i < data.size(); ++i) {
        result[i] = data[i] ^ extended_key[i];
    }

    return result.length();
}

int encrypt_data(const std::string &aesKey, const std::string &plaintext, std::string &ciphertext) {
    return xor_encrypt_decrypt(plaintext, aesKey, ciphertext);
}

int decrypt_data(const std::string &aesKey, const std::string &ciphertext, std::string &plaintext) {
    return xor_encrypt_decrypt(ciphertext, aesKey, plaintext);
}

bool generate_key_pair(std::string &public_key, std::string &private_key) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    if (!ctx)
        return false;

    if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    EVP_PKEY *pkey = NULL;
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    BIO *pub = BIO_new(BIO_s_mem());
    BIO *pri = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(pub, pkey);
    PEM_write_bio_PrivateKey(pri, pkey, NULL, NULL, 0, NULL, NULL);

    size_t pub_len = BIO_pending(pub);
    size_t pri_len = BIO_pending(pri);

    public_key.resize(pub_len);
    private_key.resize(pri_len);

    BIO_read(pub, &public_key[0], pub_len);
    BIO_read(pri, &private_key[0], pri_len);

    BIO_free(pub);
    BIO_free(pri);
    EVP_PKEY_free(pkey);

    return true;
}

bool encrypt_symmetric_key(const std::string &publicKeyPEM, const std::string &plaintext,
                           std::string &encrypted) {
    auto bio = BIO_new_mem_buf(publicKeyPEM.data(), -1);
    auto publicKey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!publicKey)
        return false;

    auto ctx = EVP_PKEY_CTX_new(publicKey, nullptr);
    EVP_PKEY_free(publicKey);

    if (!ctx)
        return false;

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    size_t outlen;
    if (EVP_PKEY_encrypt(ctx, nullptr, &outlen,
                         reinterpret_cast<const unsigned char *>(plaintext.data()),
                         plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_encrypt(ctx, outbuf.data(), &outlen,
                         reinterpret_cast<const unsigned char *>(plaintext.data()),
                         plaintext.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    encrypted.assign(reinterpret_cast<const char *>(outbuf.data()), outlen);
    EVP_PKEY_CTX_free(ctx);
    return true;
}

bool decrypt_symmetric_key(const std::string &privateKeyPEM, const std::string &encrypted,
                           std::string &decrypted) {
    auto bio = BIO_new_mem_buf(privateKeyPEM.data(), -1);
    auto privateKey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!privateKey)
        return false;

    auto ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
    EVP_PKEY_free(privateKey);

    if (!ctx)
        return false;

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> encryptedData(encrypted.begin(), encrypted.end());

    size_t outlen;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outlen, encryptedData.data(), encryptedData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    std::vector<unsigned char> outbuf(outlen);
    if (EVP_PKEY_decrypt(ctx, outbuf.data(), &outlen, encryptedData.data(), encryptedData.size()) <=
        0) {
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    decrypted.assign(reinterpret_cast<char *>(outbuf.data()), outlen);
    EVP_PKEY_CTX_free(ctx);
    return true;
}
