#pragma once


#include<string>
#include<iostream>
#include<cstring>
typedef unsigned int uint;

#define one_block_size_bytes 64 // количество байб в блоке
#define one_block_size_uints 16 // количество 4байтовых  в блоке
#define block_expend_size_uints 80 // количество 4байтовых в дополненном блоке

#define SHA1HASHLENGTHBYTES 20
#define SHA1HASHLENGTHUINTS 5

typedef uint* Block;
typedef uint ExpendBlock[block_expend_size_uints];

const uint H[5] = {
    0x67452301,
    0xEFCDAB89,
    0x98BADCFE,
    0x10325476,
    0xC3D2E1F0
};
using namespace std;

class Chat
{
    public:
    void reg(string &login, string &pass); // registration
    bool login(string &login, uint &pass); // validating an existing login
    void print();
    void DelUser(string &login);
    void AddUser(string login, uint* password);
    Chat();

    private:
    enum enPairStatus {
            free, // свободен
            engaged, //занят
            deleted //удален
        };
    
    struct UserAuthData
    {
        UserAuthData():_login(), _sha1_hash(0), _status(enPairStatus::free) {}
        ~UserAuthData() {}
        UserAuthData(string login, uint* sh1)
        {
            _login = login;
            _sha1_hash = sh1;
            _status = enPairStatus::engaged;
        }

    enPairStatus _status;
    string _login;
    uint* _sha1_hash; 
    };

    UserAuthData* _data;
    int _data_count;
    int _del_data_count;
    int mem_size;

    int hash_func(string& login, int offset);
    int multiply(int val);
    void resize();
    void sizeReduction();
};

