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
using std::string;
using sjtu::diyString;

sjtu::BPlusTree<diyString, user> userDataBase("user.db", "user_rec.db");
sjtu::map<diyString, bool> user_login;

bool isLogin(const diyString &rhs) {return user_login.find(rhs) != user_login.end() && user_login[rhs];}
sjtu::vector<user> res;
int add_user(const string *m)
{
    if (userDataBase.empty())
    {
        userDataBase.Insert(m['u'], user(m['u'], m['p'], m['n'], m['m'], "10"));
        return 0;
    }
    diyString curName = m['c'];
    if (!isLogin(curName)) return -1;
    userDataBase.Find(curName, res);
    if (res.empty() || res[0].privilege <= stoi(m['g'])) return -1;
    userDataBase.Find(m['u'], res);
    if (!res.empty()) return -1;
    userDataBase.Insert(m['u'], user(m['u'], m['p'], m['n'], m['m'], m['g']));
    return 0;
}

int login(const string *m)
{
    if (isLogin(m['u'])) return -1;
    userDataBase.Find(m['u'], res);
    if (res.empty() || !res[0].checkPassword(m['p'])) return -1;
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
    user curUser = res[0];
    userDataBase.Find(objName, res);
    if (res.empty()) return -1;
    user objUser = res[0];
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
    user curUser = res[0];
    userDataBase.Find(objName, res);
    if (res.empty()) return -1;
    user objUser = res[0];
    if (curUser.privilege < objUser.privilege) return -1;
    if (curUser.privilege == objUser.privilege && curUser != objUser) return -1;
    if (!m['g'].empty() && curUser.privilege <= stoi(m['g'])) return -1;

    userDataBase.Delete(objName, objUser);
    if (!m['g'].empty()) objUser.modifyG(stoi(m['g']));
    if (!m['p'].empty()) objUser.modifyP(m['p']);
    if (!m['n'].empty()) objUser.modifyN(m['n']);
    if (!m['m'].empty()) objUser.modifyM(m['m']);
    userDataBase.Insert(objName, objUser);
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
}
#endif //TICKETSYSTEM_USER_H
