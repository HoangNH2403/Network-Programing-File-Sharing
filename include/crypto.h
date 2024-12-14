#ifndef __CYPHER_H__
#define __CYPHER_H__

#include <string>
#include <vector>

#define SYMMETRIC_KEY_SIZE 16

/**
 * Generate RSA key pair
 */
bool generate_key_pair(std::string &public_key, std::string &private_key);

/**
 * Generate symmetric key
 */
bool generate_symmetric_key(std::string &key);

/**
 * Encrypt data with symmetric key
 * @param key symmetric key
 * @param plaintext plaintext
 * @param ciphertext ciphertext
 * @return true if success, false otherwise
 */
int encrypt_data(const std::string &aesKey, const std::string &plaintext, std::string &ciphertext);
/**
 * Decrypt data with symmetric key
 * @param key symmetric key
 * @param ciphertext ciphertext
 * @param plaintext plaintext
 * @return true if success, false otherwise
 */
int decrypt_data(const std::string &aesKey, const std::string &ciphertext, std::string &plaintext);
/**
 * Encrypt symmetric key with RSA public key
 * @param publicKeyPEM RSA public key
 * @param plaintext symmetric key
 * @param encrypted encrypted symmetric key
 * @return true if success, false otherwise
 */
bool encrypt_symmetric_key(const std::string &publicKeyPEM, const std::string &plaintext,
                           std::string &encrypted);

/**
 * Decrypt symmetric key with RSA private key
 * @param privateKeyPEM RSA private key
 * @param encrypted encrypted symmetric key
 * @param decrypted decrypted symmetric key
 * @return true if success, false otherwise
 */
bool decrypt_symmetric_key(const std::string &privateKeyPEM, const std::string &encrypted,
                           std::string &decrypted);

#endif   // !__CYPHER_H__
