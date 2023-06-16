#ifndef TICKETSYSTEM_USER_H
#define TICKETSYSTEM_USER_H
#include <iostream>
#include <cstring>
#include <filesystem>
#include "base.h"
#include "../BPT/bpt.h"
#include "../STL/map.hpp"
#include "../STL/utility.h"
#include "../STL/vector.h"
#include "fileIO.h"
using std::string;
using sjtu::diyString;

sjtu::BPlusTree<diyString, int> userDataBase("user.db", "user_rec.db");
normalCounterFile<user> userFile("userFile.db");
template<> int normalCounterFile<user>::cnt = 0;
sjtu::map<diyString, bool> user_login;

bool isLogin(const diyString &rhs) {return user_login.find(rhs) != user_login.end() && user_login[rhs];}
sjtu::vector<int> res;
user userQuery;
int add_user(const string *m)
{
    if (userDataBase.empty())
    {
        user tmpUser(m['u'], m['p'], m['n'], m['m'], "10");
        int tmpID = ++normalCounterFile<user>::cnt;
        userDataBase.Insert(m['u'], tmpID);
        userFile.write(tmpID, tmpUser);
        return 0;
    }
    diyString curName = m['c'];
    if (!isLogin(curName)) return -1;
    userDataBase.Find(curName, res);
    if (res.empty()) return -1;
    userFile.read(res[0], userQuery);
    if (userQuery.privilege <= stoi(m['g'])) return -1;
    userDataBase.Find(m['u'], res);
    if (!res.empty()) return -1;
    int tmpID = ++normalCounterFile<user>::cnt;
    user tmpUser(m['u'], m['p'], m['n'], m['m'], m['g']);
    userDataBase.Insert(m['u'], tmpID);
    userFile.write(tmpID, tmpUser);
    return 0;
}

int login(const string *m)
{
    if (isLogin(m['u'])) return -1;
    userDataBase.Find(m['u'], res);
    if (res.empty()) return -1;
    userFile.read(res[0], userQuery);
    if (!userQuery.checkPassword(m['p'])) return -1;
    user_login[m['u']] = true;
    return 0;
}

int logout(const string *m)
{
    if (!isLogin(m['u'])) return -1;
    user_login[m['u']] = false;
    return 0;
}

int query_profile(const string *m)
{
    diyString curName = m['c'], objName = m['u'];
    if (!isLogin(curName)) return -1;
    userDataBase.Find(curName, res);
    if (res.empty()) return -1;
    user curUser, objUser;
    userFile.read(res[0], curUser);
    userDataBase.Find(objName, res);
    if (res.empty()) return -1;
    userFile.read(res[0], objUser);
    if (curUser.privilege < objUser.privilege) return -1;
    if (curUser.privilege == objUser.privilege && curUser != objUser) return -1;
//    if (curUser.privilege <= objUser.privilege && curUser != objUser) return -1;

    std::cout<<objUser;

    return 0;
}

int modify_profile(const string *m)
{
    diyString curName = m['c'], objName = m['u'];
    if (!isLogin(curName)) return -1;
    userDataBase.Find(curName, res);
    if (res.empty()) return -1;
    user curUser, objUser;
    userFile.read(res[0], curUser);
    userDataBase.Find(objName, res);
    if (res.empty()) return -1;
    userFile.read(res[0], objUser);
    if (curUser.privilege < objUser.privilege) return -1;
    if (curUser.privilege == objUser.privilege && curUser != objUser) return -1;
    if (!m['g'].empty() && curUser.privilege <= stoi(m['g'])) return -1;

//    userDataBase.Delete(objName, objUser);
    if (!m['g'].empty()) objUser.modifyG(stoi(m['g']));
    if (!m['p'].empty()) objUser.modifyP(m['p']);
    if (!m['n'].empty()) objUser.modifyN(m['n']);
    if (!m['m'].empty()) objUser.modifyM(m['m']);
//    userDataBase.Insert(objName, objUser);
    userFile.write(res[0], objUser);
    std::cout<<objUser;
    return 0;
}
/*
sjtu::BPlusTree<diyString, user> userDataBase("user.db", "user_rec.db");
sjtu::map<diyString, bool> user_login;*/

void cleanUser()
{
    user_login.clear();
    userDataBase.~BPlusTree();//不用注明sjtu域
    std::filesystem::remove("user.db");
    std::filesystem::remove("user_rec.db");
    new (&userDataBase) sjtu::BPlusTree<diyString, user>("user.db", "user_rec.db");//placement new 必须注明sjtu域

    userFile.~normalCounterFile();
    std::filesystem::remove("userFile.db");
    new (&userFile) normalCounterFile<user>("userFile.db");
}
#endif //TICKETSYSTEM_USER_H
