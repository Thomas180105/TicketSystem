//TODO：未测试的点：Trie树的初始化以及使用的正确性
#include <iostream>
#include <cstring>
#include "../BPT/bpt.h"
#include "../BPT/bpt_main.cpp"
#include "user.h"
#include "train.h"
#include "ticket.h"

int clean (const string *m)
{
    cleanUser();
    cleanTrain();
    cleanTicket();
    return 0;
}

using std::string;
string funcName[16] = {"add_user", "login", "logout", "query_profile", "modify_profile","add_train", "delete_train", "release_train", "query_train",
                        "query_ticket", "query_transfer", "buy_ticket", "query_order","refund_ticket", "clean", "exit" };

int (*func[16])(const string *m) = {add_user, login, logout, query_profile, modify_profile,add_train, delete_train, release_train, query_train,query_ticket, query_transfer, buy_ticket, query_order,refund_ticket, clean};

int out[16] = {1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1};

int num(char ch) {return ch == '_' ? 26 : ch - 'a';}
const int N = 305;//trie树的大小
string m[256], s[24];//s储存切割之后的字符串，m是一个小map,存key对应的argument
int ch[N][27], mark[N], tot = 0;

void teeter_strCut(int cur)
{
    for (int i = 0; i <= cur; ++i) std::cout<<s[i]<<' ';

    std::cout<<"\nthe header : "<<m[0]<<'\n';
    for (int i = 'a'; i <= 'z'; ++i)
    {
        if (m[i].empty()) continue;
        std::cout<<"the content of -"<<char(i)<<" is "<<m[i]<<'\n';
    }
}

void init_trie()
{
    int len, p, nxt;
    for (int i = 0; i < 16; ++i)
    {
        len = funcName[i].size(), p = 0;
        for (int j = 0; j < len; ++j)
        {
            nxt = num(funcName[i][j]);
            if (!ch[p][nxt]) ch[p][nxt] = ++tot;
            p = ch[p][nxt];
        }
        mark[p] = i;
    }
}

bool process(const char *str)
{
    //字符串处理
    int cur = 0, pos = 0, len = strlen(str);
    s[cur].clear();
    while(pos < len && str[pos] == ' ') ++pos;
    for (; pos < len; ++pos)
    {
        if (str[pos] != ' ') s[cur] += str[pos];
        else if (pos + 1 < len && str[pos + 1] != ' ') {++cur; s[cur].clear();}
    }
    for (int i = 'a'; i <= 'z'; ++i) m[i].clear();
    for (int i = 2; i <= cur; i += 2) m[s[i][1]] = s[i + 1];
    m[0].clear(), m[0] = s[0].substr(1, s[0].size() - 2);

    //调用有关函数
    cur = 0, len = s[1].size();
    for (int i = 0; i < len; ++i) cur = ch[cur][num(s[1][i])];
    std::cout<<s[0]<<' ';
    if (mark[cur] == 15) {std::cout<<"bye\n"; return false;}
    else
    {
        int res = func[mark[cur]](m);
        if (out[mark[cur]] || res) std::cout<<res<<"\n";
        return true;
    }
}

char input[4005];
int main()
{
//    freopen("one_mineAns", "w", stdout);
    /*std::cout<<sizeof(size_t) + sizeof (trainStation)<<'\n';
    std::cout<<sizeof(size_t) + sizeof (Order)<<'\n';
    std::cout<<sizeof(int) + sizeof (int)<<'\n';
    std::cout<<sizeof(size_t) + sizeof (int)<<'\n';
    std::cout<<sizeof(diyString) + sizeof (user)<<'\n';*/
    /*sjtu::BPlusTree<size_t, Order> sizeTester("size.db", "size_rec.db");
    std::cout<<double (sizeTester.get_node_size()) / 4096;*/
    init_trie();
    while(true)
    {
        if (!std::cin.getline(input, 2000) || !process(input)) break;
    }
    return 0;
}
/*
sjtu::BPlusTree<size_t, trainStation> stationDataBase("trainStation.db", "trainStation_rec.db");
sjtu::BPlusTree<size_t, Order> OrderDataBase("OrderDataBase.db", "OrderDataBase_rec.db");//从strHash(userId)到对应的order
sjtu::BPlusTree<int, int> waitDataBase("waitDataBase.db", "waitDataBase_rec.db");//基于外存的一个有序vector，存放时间戳
sjtu::BPlusTree<size_t, int> trainDataBase("train.db", "train_rec.db");
sjtu::BPlusTree<diyString, user> userDataBase("user.db", "user_rec.db");
*/