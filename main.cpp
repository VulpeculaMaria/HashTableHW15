#include"chat.h"


int main()
{
    Chat chat;
    string login = "maria";
    uint* pass;
    chat.AddUser(login, pass);
    chat.print();

    getchar();

}