/*
 * Codec class for SQLite3 encryption codec.
 * (C) 2010 Olivier de Gaalon
 *
 * Distributed under the terms of the Botan license
 */

#ifndef _CODEC_H_
#define _CODEC_H_

#include <string>
#include <botan/botan.h>
#include <botan/loadstor.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__BORLANDC__)
#define __STDC__ 1
#endif

#include "./sqliteInt.h"

#if defined(__BORLANDC__)
#undef __STDC__
#endif

/* ATTENTION: Macro similar to that in pager.c
 * Needed because pager is forward declared when needed most
 * TODO: Check in case of new version of SQLite
 * ... but it's VERY unlikely to change (it'd break all past DBs)
 */
#include "./os.h"
#define CODEC_PAGER_MJ_PGNO(x) ((PENDING_BYTE/(x))+1)

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

using namespace std;
using namespace Botan;

/*These constants can be used to tweak the codec behavior as follows
 *Note that once you've encrypted a database with these settings,
 *recompiling with any different settings will give you a library that
 *cannot read that database, even given the same passphrase.*/

//BLOCK_CIPHER_STR: Cipher and mode used for encrypting the database
//make sure to add "/NoPadding" for modes that use padding schemes
const string BLOCK_CIPHER_STR = "Twofish/XTS";

//S2K_STR: Key derivation function used to derive both the encryption
//and IV derivation keys from the given database passphrase
const string S2K_STR = "PBKDF2(SHA-160)";

//SALT_STR: Hard coded salt used to derive the key from the passphrase.
const string SALT_STR = "&g#nB'9]";

//MAC_STR: CMAC used to derive the IV that is used for db page
//encryption
const string MAC_STR = "CMAC(Twofish)";

//S2K_ITERATIONS: Number of hash iterations used in the key derivation
//process.
const int S2K_ITERATIONS = 10000;

//SALT_SIZE: Size of the salt in bytes (as given in SALT_STR)
const int SALT_SIZE = 64/8; //64 bit, 8 byte salt

//KEY_SIZE: Size of the encryption key. Note that XTS splits the key
//between two ciphers, so if you're using XTS, double the intended key
//size. (ie, "AES-128/XTS" should have a 256 bit KEY_SIZE)
const int KEY_SIZE = 512/8; //512 bit, 64 byte key. (256 bit XTS key)

//IV_DERIVATION_KEY_SIZE: Size of the key used with the CMAC (MAC_STR)
//above.
const int IV_DERIVATION_KEY_SIZE = 256/8; //256 bit, 32 byte key

class Codec
{
public:
    Codec(void *db);
    Codec(const Codec& other, void *db);

    void GenerateWriteKey(const char* userPassword, int passwordLength);
    void DropWriteKey();
    void SetWriteIsRead();
    void SetReadIsWrite();

    unsigned char* Encrypt(int page, unsigned char* data, bool useWriteKey);
    void Decrypt(int page, unsigned char* data);

    void SetPageSize(int pageSize) { m_pageSize = pageSize; }

    bool HasReadKey() { return m_hasReadKey; }
    bool HasWriteKey() { return m_hasWriteKey; }
    void* GetDB() { return m_db; }

private:
    bool          m_hasReadKey;
    bool          m_hasWriteKey;

    SymmetricKey
        m_readKey,
        m_writeKey,
        m_ivReadKey,
        m_ivWriteKey;

    Pipe
        m_encipherPipe,
        m_decipherPipe,
        m_macPipe;

    Keyed_Filter *m_encipherFilter;
    Keyed_Filter *m_decipherFilter;
    MAC_Filter *m_cmac;

    int m_pageSize;
    unsigned char m_page[SQLITE_MAX_PAGE_SIZE];
    void* m_db;

    InitializationVector GetIVForPage(u32bit page, bool useWriteKey);
    void InitializeCodec(void *db);
};

#endif