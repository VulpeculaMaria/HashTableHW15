#include"chat.h"
#include<cstring>
#include<iostream>

uint bring_to_human_view(uint val) {
    return  ((val & 0x000000FF) << 24) | 
            ((val & 0x0000FF00) << 8) | 
            ((val & 0x00FF0000) >> 8) | 
            ((val & 0xFF000000) >> 24);
}
uint cycle_shift_left(uint val, int bit_count) {
    return (val << bit_count | val >> (32-bit_count));
}

uint* sha1(char* message, uint msize_bytes) {
    //инициализация
    uint A = H[0];
    uint B = H[1];
    uint C = H[2];
    uint D = H[3];
    uint E = H[4];
    
    // подсчет целого числа блоков
    uint totalBlockCount = msize_bytes  / one_block_size_bytes;
    
    // подсчет, сколько байт нужно, чтобы дополнить последний блок
    uint needAdditionalBytes = 
        one_block_size_bytes - (msize_bytes - totalBlockCount * one_block_size_bytes);
        
    if(needAdditionalBytes < 8) {
        totalBlockCount += 2;
        needAdditionalBytes += one_block_size_bytes;
    } else {
        totalBlockCount += 1;
    }
    
    // размер дополненного по всем правилам сообщения
    uint extendedMessageSize = msize_bytes + needAdditionalBytes;
    
    // выделяем новый буфер и копируем в него исходный
    unsigned char* newMessage = new unsigned char[extendedMessageSize];
    memcpy(newMessage, message, msize_bytes);
    
    // первый бит ставим '1', остальные обнуляем
    newMessage[msize_bytes] = 0x80;    
    memset(newMessage + msize_bytes + 1, 0, needAdditionalBytes - 1);
    
    // задаем длину исходного сообщения в битах
    uint* ptr_to_size = (uint*) (newMessage + extendedMessageSize - 4); 
    *ptr_to_size =  bring_to_human_view(msize_bytes * 8);
    
    ExpendBlock exp_block;
    //раунды поехали
    for(int i=0; i<totalBlockCount; i++) {
        
        // берем текущий блок и дополняем его
        unsigned char* cur_p = newMessage + one_block_size_bytes * i;
        Block block = (Block) cur_p;
        
        // первые 16 4байтовых чисел
        for (int j=0; j<one_block_size_uints; j++) {
            exp_block[j] = bring_to_human_view(block[j]);
        }
        // следующие 64...
        for (int j=one_block_size_uints; j < block_expend_size_uints; j++) {
            exp_block[j] = 
                exp_block[j-3] ^ 
                    exp_block[j-8] ^ 
                      exp_block[j-14] ^ 
                        exp_block[j-16];
            exp_block[j] = cycle_shift_left(exp_block[j], 1);
        }
            
        // инициализация 
        uint a = H[0];
        uint b = H[1];
        uint c = H[2];
        uint d = H[3];
        uint e = H[4];

        // пересчитываем
        for(int j=0;j < block_expend_size_uints; j++) {
            uint f;
            uint k;
            // в зависимости от раунда считаем по-разному
            if (j < 20) {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            } else if (j < 40) {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            } else if (j < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            } else {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
        
            // перемешивание
            uint temp = cycle_shift_left(a,5) + f + e + k + exp_block[j];
            e = d;
            d = c;
            c = cycle_shift_left(b,30);
            b = a;
            a = temp;   
        }
        // пересчитываем
        A = A + a;
        B = B + b;
        C = C + c;
        D = D + d;
        E = E + e;
    }
    
    // A,B,C,D,E являются выходными 32б составляющими посчитанного хэша
    uint* digest = new uint[5];
    digest[0] = A;
    digest[1] = B;
    digest[2] = C;
    digest[3] = D;
    digest[4] = E;
    
    // чистим за собой
    delete[] newMessage;
    return digest;
}
Chat::Chat() 
{
    _data_count = 0;
    _del_data_count = 0;
    mem_size = 8;
    _data = new UserAuthData[mem_size];
}
int Chat::hash_func(string& login, int offset)
{
    int sum = 0;
    for(int i = 0; i < login.size();i++)
    {
        sum += login[i];
    }
    return (multiply(sum) + offset*offset) % mem_size;
}

int Chat::multiply(int val)
{   
    const double num = 0.5;
    return int(mem_size * (num * val - int(num * val)));
}
void Chat::reg(string& login,  string& pass) {
    uint  lenght = pass.size();
    char* password = new char[lenght];
    for(int i = 0; i < lenght;++i)
    {
        password[i] = pass[i];
    }
    uint* _sha1_hash = sha1(password, lenght);
    AddUser(login, _sha1_hash); 
    cout << "pass_sha1_hash = " << _sha1_hash << endl;
}

bool Chat::login(string &login, uint &pass) {
    
   for(int i = 0; i < mem_size; i++)
   {
            int index = hash_func(login, i);
            if(_data[index]._login == login)
            {
                return  true;
            }
   }
   return false;
}

void Chat::AddUser(string login, uint* password)
{
    if(_data_count >= mem_size)
        resize();
    int index = -1;
    int i = 0;
    for(; i < mem_size; i++)
    {
        index = hash_func(login, i);
        if(_data[index]._status == enPairStatus::free)
            break;
    }
    if(i >= mem_size)
    {
        resize();
        AddUser(login, password);
    }
    else
    {  
        _data[index] = UserAuthData(login, password);
        _data_count++;   
    }
}

void Chat::DelUser(string& login)
{
    int index = -1;
    for(int i = 0; i < mem_size; i++)
    {
        index = hash_func(login, i);

        if(_data[index]._status == engaged && _data[index]._login == login)
        {
            _data[index]._status = deleted;
            _data_count--;
            cout << "login" << _data[index]._login << "is deleted" << endl;
            if(_del_data_count >= (mem_size / 2))
            {
                sizeReduction();
            }
            return;
        }
        if(_data[index]._status == free)
        {
            return;
        }
        else
        {
            cout << "is already deleted" << endl;
            return;
        }
    }
}

void Chat::resize()
{
    UserAuthData* save_data = _data;
    int OldSize = mem_size;
    mem_size *= 2;
    _data_count = 0;
    _data = new UserAuthData[mem_size];

    for(int i = 0; i < mem_size; i++)
    {
        UserAuthData& old_data = save_data[i];
        if(old_data._status == engaged)
        {
            uint* sha_hash_copy = new uint[SHA1HASHLENGTHUINTS];
            memcpy(sha_hash_copy, old_data._sha1_hash, SHA1HASHLENGTHBYTES);
            AddUser(old_data._login, sha_hash_copy);
        }
    }
    delete[] save_data;
}

void Chat::sizeReduction()
{
    UserAuthData* save_data = _data;
    int OldSize = mem_size;
    mem_size /= 2;
    _data_count = 0;
    _data = new UserAuthData[mem_size];

    for(int i = 0; i < mem_size; i++)
    {
        UserAuthData& old_data = save_data[i];
        if(old_data._status == engaged)
        {
           uint* sha_hash_copy = new uint[SHA1HASHLENGTHUINTS];
            memcpy(sha_hash_copy, old_data._sha1_hash, SHA1HASHLENGTHBYTES);
            AddUser(old_data._login, sha_hash_copy);
        }
    }
    delete[] save_data;
}

void Chat::print()
{
    cout << "\nPrint count users = " << _data_count << endl;
    cout << "Print delete count users = " << _del_data_count << endl;
    for (int i = 0; i < mem_size; i++)
    {
        cout << "index = " << i;
            cout << "\tLogin = " << _data[i]._login;
            cout << "\tPassword hash = " << _data[i]._sha1_hash;
            cout << "\tStatus = " << _data[i]._status << endl;
    }
}



