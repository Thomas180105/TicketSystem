#ifndef TICKETSYSTEM_BPT_H
#define TICKETSYSTEM_BPT_H

#include "../STL/exceptions.h"
#include "../STL/utility.h"
#include "../STL/vector.h"
#include "linkedHashMap.h"
#include <iostream>
#include <fstream>
#include <istream>

#include <queue>
//bpt的预先准备：
//（1）定义结点类，type，sum，两个数组，location，next，
//（2）定义uniqueStream,重载>>以及实现读结点读vector的函数
//（3）定义文件系统fileSystem，其中readNode以及writeNode()需要实现缓存
//（4）实现bpt类的构造以及析构函数
using std::fstream;
namespace sjtu{
    //要求Key需要重载赋值函数（node（const node &rhs））,输出重载（node::printInfo()）
    template<class Key, class T>
    class BPlusTree{
        typedef sjtu::pair<Key, T> value;
    public:
        const static int M = 328;//const static int M = 52;//此时sizeof (node) = 4048，在4096之内
        const static int minSize = M / 3;
        const static int maxSize = M;
        enum {LEAF, NODE};
        class node{
            friend class BPlusTree;
            bool type;//表示是否是叶子节点
            int cnt;//表示Key的数量
            int location, next, parent;
            int ch[M + 2] = {};//注意需要预留出一位额外的位置
            value key[M + 2];//typedef sjtu::pair<Key, T> value;//同上
        public:
            node(const node &rhs) : type(rhs.type), cnt(rhs.cnt), location(rhs.location), next(rhs.next), parent(rhs.parent)
            {
                for (int i = 0; i < cnt; ++i) key[i] = rhs.key[i];
                if (rhs.type == NODE) for (int i = 0; i <= cnt; ++i) ch[i] = rhs.ch[i];
                else memset(ch, 0, sizeof (ch));
            }
            node &operator=(const node &rhs)
            {
                if (this == &rhs) return *this;
                type = rhs.type, cnt = rhs.cnt, location = rhs.location, next = rhs.next, parent = rhs.parent;
                for (int i = 0; i < cnt; ++i) key[i] = rhs.key[i];
                if (type == NODE) for (int i = 0; i <= cnt; ++i) ch[i] = rhs.ch[i];
                else memset(ch, 0, sizeof (ch));
                return *this;
            }
            void printInfo() const
            {
                std::cout<<"now the node's location is "<<location<<", the cnt is "<<cnt<<", the type is "<<(type ? "NODE" : "LEAF")<<", the parent is "<<parent<<'\n';
                for (int i = 0; i < cnt; ++i) std::cout<<"key["<<i<<"] = "<<key[i]<<" ";std::cout<<'\n';
                for (int i = 0; i <= cnt; ++i) std::cout<<"ch["<<i<<"] = "<<ch[i]<<" ";std::cout<<'\n';
            }
        public:
            node() : type(false), cnt(0), location(0), next(0), parent(0)
            {
                memset(ch, 0, sizeof (ch));
            }
        };
    public:
        int get_node_size() {return sizeof (node);}
        class diyStream: public std::fstream{
        public:
            diyStream &operator<<(const int &rhs)//写root(int类型，位置在0处)
            {
                seekp(0);
                write(reinterpret_cast<const char *> (&rhs), sizeof (int));
                return *this;
            }
            diyStream &operator<<(const node &rhs)//写节点
            {
                if (!rhs.location) return *this;
                seekp(rhs.location);
                write(reinterpret_cast<const char *> (&rhs), sizeof (node));
                return *this;
            }
            diyStream &operator<<(const vector<int> &rhs)
            {
                seekp(0);
                size_t siz = rhs.size();
                write(reinterpret_cast<char *> (&siz), sizeof (size_t));
                int tmp;
                for (int i = 0; i < siz; ++i)
                {
                    tmp = rhs[i];
                    write(reinterpret_cast<const char *> (&tmp), sizeof (int));
                }
                return *this;
            }
            void readInt(int &n)//读0位置处的int
            {
                seekg(0);
                read(reinterpret_cast<char *> (&n), sizeof (int));
            }
            void readNode(int loc, node &p)
            {
                seekg(loc);
                read(reinterpret_cast<char *> (&p), sizeof (node));
            }
            void readVector(vector<int> &p)
            {
                seekg(0);
                size_t siz;
                read(reinterpret_cast<char *> (&siz), sizeof (siz));
                int tmp;
                for (int i = 0; i < siz; ++i)
                {
                    read(reinterpret_cast<char *> (&tmp), sizeof (int));
                    p.push_back(tmp);
                }
            }
        };
        class fileSystem{//三层结构：内存->缓存->外存
            friend class BPlusTree;
        public:
            diyStream dataBase;
            diyStream recycle;
            linkedHashMap<node> map;
            fileSystem(const char *dataName, int &root, int &nodeCnt)
            {
                dataBase.open(dataName, fstream::in);
                bool flag = dataBase.is_open();
                dataBase.close();
                if (!flag)
                {
                    dataBase.open(dataName, fstream::out);//新建当前文件
                    dataBase.close();
                    int rt = 0;
                    dataBase<<rt;
                    dataBase.open(dataName, fstream::in | fstream::out | fstream::binary);
                    nodeCnt = 0;
                }
                else
                {
                    dataBase.open(dataName, fstream::in | fstream::out | fstream::binary);
                    dataBase.readInt(root);
                    dataBase.seekg(0, diyStream::end);
                    nodeCnt = ((int)dataBase.tellg() - sizeof (int)) / sizeof (node);
                    //nodeCnt此时还没有除去vacancy的影响，在后续BPT构造时会矫正
                }
            }
            ~fileSystem()//缓存信息存放到外存，保证可持久化
            {
//                std::cout<<"now we are default fileSystem: \n";
                node tmp;
                while(map.popNode(tmp))//map.timePop(tmp)
                {
                    dataBase<<tmp;
//                    std::cout<<"the location is "<<tmp.location<<'\n';
                }
//                recycle.close();
//                dataBase.close();
            }
            void writeNode(const node &rhs)//写在缓存中
            {
//                std::cout<<"the node's location is "<<rhs.location;
//                rhs.printInfo();
                map.insert(rhs.location, rhs);
                node tmp;
                if (map.checkSize(tmp)) dataBase<<tmp;//if (map.check(tmp)) dataBase<<tmp;
            }
            void readNode(int loc, node &p)//先从缓存中找
            {
                if (!map.find(loc, p))
                {
                    dataBase.readNode(loc, p);
                    map.insert(loc, p);
                    node tmp;
                    if (map.checkSize(tmp)) dataBase<<tmp;//if (map.check(tmp)) dataBase<<tmp;
                }
            }
        };
        int root = 0;
        fileSystem helper;
        vector<int> vacancy;
    public:
        int nodeCnt;//之前 "int nodeCnt = 0"会导致bug???（断点测出来的，但不知道为什么）
        BPlusTree(const char *dataName, const char *recycleName) : helper(dataName, root, nodeCnt)
        {
            helper.recycle.open(recycleName, fstream::in);
            bool flag = helper.recycle.is_open();
            helper.recycle.close();
            if (!flag)
            {
                helper.recycle.open(recycleName, fstream::out);
                helper.recycle.close();
                helper.recycle.open(recycleName, fstream::in | fstream::out | fstream::binary);
            }
            else
            {
                helper.recycle.open(recycleName, fstream::in | fstream::out | fstream::binary);
                helper.recycle.readVector(vacancy);
            }
        }
        ~BPlusTree()
        {
            helper.recycle<<vacancy;
            helper.dataBase<<root;
//            helper.recycle.close();
//            helper.dataBase.close();
        }

    private:
        int create(node &a)//不负责维护root
        {
            if (!vacancy.empty())
            {
                a.location = vacancy.back();
                vacancy.pop_back();
            }
            else
            {
                a.location = nodeCnt * sizeof (node) + sizeof (int);
                ++nodeCnt;
            }
            a.type = NODE, a.cnt = 0, a.next = 0, a.parent = 0;
            return a.location;
        }
        int searchSon(node &p, const value &val)//返回当前val在p的第几个儿子结点中去找，找第一个>
        {
            if (!p.cnt) return 0;
            if (p.key[p.cnt - 1] <= val) return p.cnt;
            int l = 0, r = p.cnt - 1;
            while (l < r)
            {
                int mid = (l + r - 1) >> 1;
                if (p.key[mid] <= val) l = mid + 1;
                else r = mid;
            }
            return l;
        }
        int searchSon_byKey(node &p, const Key &k)//找第一个>=的key的下标（0 ~ cnt - 1，不存在就返回cnt
        {
            if (!p.cnt) return 0;
            if (p.key[p.cnt - 1].first < k) return p.cnt;
            int l = 0, r = p.cnt - 1;
            while (l < r)
            {
                int mid = (l + r - 1) >> 1;
                if (p.key[mid].first < k) l = mid + 1;
                else r = mid;
            }
            return l;
        }
        void addKey(node &p, const value &val)//维护key[]数组，同时自动维护了cnt,不需要在调用此函数之后写++cnt  //TODO:可以考虑将cnt的维护放在函数外围
        {
            int pos = searchSon(p, val);
            for (int i = p.cnt; i > pos; --i) std::swap(p.key[i], p.key[i - 1]);
            p.key[pos] = val;
            ++p.cnt;
        }
        void delKey(node &p, int pos)//不传const value &key的原因：不用跑一次无意义的二分，大部分调用delKey的时候都是清楚要删除的是第一个key的
        {
            for (int i = pos; i < p.cnt - 1; ++i) std::swap(p.key[i], p.key[i + 1]);
            --p.cnt;
        }
        void modifyKey(node &p, int pos, const value &val)
        {
            p.key[pos] = val;
        }
        void addChild(node &p, int pos, int c)//addKey跟着使用addChild，由于cnt的变化，此处有越界访问的问题存在
        {
            for (int i = p.cnt; i >= pos; --i) p.ch[i + 1] = p.ch[i];
            p.ch[pos] = c;
        }
        void delChild(node &p, int pos)
        {
            for (int i = pos; i < p.cnt; ++i) p.ch[i] = p.ch[i + 1];//似乎不用写”p.ch[p.ch]“
            p.ch[p.cnt] = 0;
        }

        void split(node &a)//将结点a进行分裂，同时处理了a是叶节点和非叶节点的情况
        {
            node b, c;
            //step one: find the fa of node a (node b)
            if (root == a.location)
            {
                root = create(b);
                b.ch[0] = a.location;
                a.parent = b.location;
            }
            else helper.readNode(a.parent, b);

            //step 2: init the node c split from node a
            create(c), c.parent = a.parent;
            c.type = a.type;
            //c结点的cnt向下取整，所以后续的b从儿子拿结点的时候是选择从a那里拿
            //由于更新了a的cnt，相当于ch[]和key[]自动维护好了
            c.cnt = a.cnt / 2, a.cnt -= c.cnt;
            c.next = a.next, a.next = c.location;
            if (a.type == NODE)
            {
                for (int i = 0; i <= c.cnt; ++i)
                {
                    c.ch[i] = a.ch[i + a.cnt];
                    a.ch[i + a.cnt] = 0;
                    node tmp;
                    helper.readNode(c.ch[i], tmp);
                    tmp.parent = c.location;
                    helper.writeNode(tmp);
                }
            }
            for (int i = 0; i < c.cnt; ++i) c.key[i] = a.key[i + a.cnt];

            //step 3: get the info of node b correct
            int pos = (!b.cnt) ? 0 : searchSon(b, c.key[0]);
            b.ch[pos] = c.location;
            addChild(b, pos, a.location), a.parent = b.location;//TODO:[diff]forward

            if (a.type == LEAF) addKey(b, c.key[0]);
            else addKey(b, a.key[a.cnt - 1]), --a.cnt;

            //step 4 : write info to disk
            helper.writeNode(a);
            helper.writeNode(b);
            helper.writeNode(c);

            //step 5 :check the balance
            if (b.cnt > maxSize) balanceDown(b);
        }
        void balanceUp(node &p, const value &val)//p.cnt过小，需要调整//TODO：[diff]划分逻辑可以优化，考虑划分到merge中去
        {
            if (p.location == root)
            {
                if (!p.cnt)
                {
                    vacancy.push_back(p.location);
                    root = 0;
                }
                return;
            }

            node fa;
            helper.readNode(p.parent, fa);
            /*if (fa.location == root && fa.cnt == 1 && !fa.ch[1])//TODO:doubleCheck
            {
                vacancy.push_back(root);
                root = p.location;
                p.parent = 0;
                return;
            }*/
            //开始找兄弟
            node bro;
            int pos = searchSon(fa, val), tmp = -1, flag = -1;
            if (pos)
            {
                helper.readNode(fa.ch[pos - 1], bro);
                tmp = bro.cnt;
                flag = 0;//找到了左兄弟
            }
            if (pos < fa.cnt && fa.ch[pos + 1])
            {
                helper.readNode(fa.ch[pos + 1], bro);
                flag = 1;//找到右兄弟
                if (pos && tmp > bro.cnt)
                {
                    helper.readNode(fa.ch[pos - 1], bro);
                    flag = 0;//换回左兄弟
                }
            }

            //至此找完了兄弟，如果两个兄弟都不存在，则flag = -1，如果仅存在一个兄弟，则选择这个兄弟，如果两个兄弟都存在，就选择cnt更大的一个兄弟
            value fa_val = fa.key[fa.cnt - 1];//merge过程中fa.key[fa.cnt - 1]可能发生变化，于是提前记录
            bool isUnique;
            if (flag == 0) isUnique = merge(fa, bro, p, bro.key[bro.cnt - 1]);
            else isUnique = merge(fa, p, bro, val);

            //检查是否需要递归,注意需要特判一下父亲结点是否存在,如果merge过程中维护了root的信息，则放回true
            if (!isUnique)
            {
                helper.readNode(fa.location, fa);
                if (fa.cnt < minSize) balanceUp(fa, fa_val);//TODO:[diff]
            }
        }
        void balanceDown(node &p)//p数量超标时调用
        {
            split(p);
        }
        bool merge(node &fa, node &ls, node &rs, const value &val)//ls和rs中至少有一个< minSize时调用此函数来解决问题
        {
//            int pos = searchSon(fa, ls.key[ls.cnt - 1]);
            int pos = searchSon(fa, val);
            if (ls.cnt + rs.cnt < maxSize)//注意不写成<=，因为合并过程结束之后有可能ls.cnt = ls.cnt + rs.cnt + 1(父亲结点为NODE时)
            {
                //step one ： 维护好父亲结点的信息
                if (ls.type == NODE)//一个特判
                {
                    ls.key[ls.cnt] = fa.key[pos];
                    ++ls.cnt;
                }
                delChild(fa, pos + 1);
                delKey(fa, pos);

                //step two : 开始合并
                ls.next = rs.next, rs.next = 0;
                if (ls.type == NODE)
                {
                    for (int i = 0; i <= rs.cnt; ++i)
                    {
                        ls.ch[ls.cnt + i] = rs.ch[i];
                        node tmp;
                        helper.readNode(ls.ch[ls.cnt + i], tmp);
                        tmp.parent = ls.location;
                        helper.writeNode(tmp);
                    }
                }
                for (int i = 0; i < rs.cnt; ++i) std::swap(ls.key[ls.cnt + i], rs.key[i]);
                ls.cnt += rs.cnt, rs.cnt = 0;

                //step three : 开始写回
                bool res = false;
                if (fa.location == root && !fa.cnt)
                {
                    vacancy.push_back(root);
                    root = ls.location;
                    ls.parent = 0;
                    res = true;//表示这一次调用merge更新了根节点，也就是balanceUp的递归调用一定终止了
                }
                else
                {
                    helper.writeNode(fa);
                }
                helper.writeNode(ls), vacancy.push_back(rs.location);

//                if (ls.cnt > maxSize) split(ls); //TODO:[diff] doubleCheck
                return res;
            }


            //TODO:[to change]
            value key_[(M << 1) + 5];
            int ch_[(M << 1) + 5];
            int tot = ls.cnt;

            //step 1 : 资源上交,注意key_[]数组在type是NODE的时候有特判
            for (int i = 0; i < ls.cnt; ++i) std::swap(key_[i], ls.key[i]);
            if (ls.type == NODE)
            {
                key_[tot] = fa.key[pos];
                ++tot;
            }
            for (int i = 0; i < rs.cnt; ++i) std::swap(key_[i + tot], rs.key[i]);
            tot += rs.cnt;
            if (ls.type == NODE)
            {
                for (int i = 0; i <= ls.cnt; ++i) ch_[i] = ls.ch[i];
                for (int i = 0; i <= rs.cnt; ++i) ch_[ls.cnt + 1 + i] = rs.ch[i];
            }
            delKey(fa, pos);

            //step 2 : 资源下发
            ls.cnt = tot >> 1;
            int tmp = ls.cnt + (ls.type == NODE);
            rs.cnt = tot - tmp;
            for (int i = 0; i < ls.cnt; ++i) std::swap(ls.key[i], key_[i]);
            for (int i = 0; i < rs.cnt; ++i) rs.key[i] = key_[i + tmp];//for (int i = 0; i < rs.cnt; ++i) std::swap(rs.key[i], key_[i + tmp]);
            if (ls.type == NODE)
            {
                for (int i = 0; i <= ls.cnt; ++i)
                {
                    ls.ch[i] = ch_[i];
                    node t;
                    helper.readNode(ls.ch[i], t);
                    t.parent = ls.location;
                    helper.writeNode(t);
                }
                for (int i = 0; i <= rs.cnt; ++i)
                {
                    rs.ch[i] = ch_[ls.cnt + 1 + i];
                    node t;
                    helper.readNode(rs.ch[i], t);
                    t.parent = rs.location;
                    helper.writeNode(t);
                }
            }

            //step 3 : 简单维护一下fa

            addKey(fa, key_[ls.cnt]);
//            modifyKey(fa, pos, key_[ls.cnt]);//TODO:[diff] checked

            //step 4 : 写回磁盘
            helper.writeNode(fa), helper.writeNode(ls), helper.writeNode(rs);
            return false;
        }
    public:
        void Insert(const Key &k, const T &v)
        {
            node a;
            value val(k, v);
            if (!root)//注意特判
            {
                root = create(a);
                a.type = LEAF, a.cnt = 1, a.key[0] = val, a.parent = 0;
                helper.writeNode(a);
                return;
            }
            int cur = root;
            while(true)
            {
                helper.readNode(cur, a);
                if (a.type == NODE) cur = a.ch[searchSon(a, val)];
                else
                {
                    addKey(a, val);
                    if (a.cnt <= maxSize) helper.writeNode(a);
                    else balanceDown(a);
                    return;
                }
            }
        }
        void Delete(const Key &k, const T &v)
        {
            if (!root) return;
            node a;
            value val(k, v);
            int cur = root;
            while(true)
            {
                helper.readNode(cur, a);
                if (a.type == NODE) cur = a.ch[searchSon(a, val)];
                else
                {
                    int pos = searchSon(a, val);
                    if (!pos || a.key[pos - 1] != val) return;
                    value tmp_val = a.key[a.cnt - 1];
                    delKey(a, pos - 1);
                    helper.writeNode(a);
                    if (a.cnt < minSize) balanceUp(a, tmp_val);
//                    if (a.cnt >= minSize || a.location == root) helper.writeNode(a);
//                    else balanceUp(a);
                    return;
                }
            }
        }
        /*
        void Find(const Key &k, vector<T> &vec)
        {
            vec.clear();
            if (!root) return;
            node a;
            int cur = root;
            while(cur)
            {
                helper.readNode(cur, a);
                if (a.type == NODE) cur = a.ch[searchSon_byKey(a, k)];
                else
                {
                    int pos = searchSon_byKey(a, k);
                    if (pos == a.cnt || a.key[pos].first != k) return;
                    for (int i = pos; i < a.cnt; ++i)
                    {
                        if (a.key[i].first == k) vec.push_back(a.key[i].second);
                        else return;
                    }
                    cur = a.next;//有可能横跨多个结点，这导致了要写while(cur)而不是while(true)
                }
            }
        }*/
        void Find(const Key &k, vector<T> &vec)
        {
            vec.clear();
            if (!root) return;
            node a;
            int cur = root;
            while(cur)
            {
                helper.readNode(cur, a);
                if (a.type == NODE)
                {
                    int pos = a.cnt;
                    for (int i = 0; i < a.cnt; ++i)
                    {
                        if (k <= a.key[i].first) {pos = i; break;}
                    }
                    cur = a.ch[pos];
                }
                else
                {
                    for (int i = 0; i < a.cnt; ++i)
                    {
                        if (a.key[i].first == k) vec.push_back(a.key[i].second);
                        else if (a.key[i].first > k) return;
                    }
                    cur = a.next;
                }
            }
        }
        void FindALL(vector<T> &vec)
        {
            vec.clear();
            node a;
            int cur = root;
            while(cur)
            {
                helper.readNode(cur, a);
                if (a.type == NODE) cur = a.ch[0];
                else
                {
                    for (int i = 0; i < a.cnt; ++i) vec.push_back(a.key[i].second);
                    cur = a.next;
                }
            }
        }
        bool empty() {return !nodeCnt;}
        void debug()
        {
            if (!root) return;
            int cur = root;
            node tmp;
            while(cur)
            {
                helper.readNode(cur, tmp);
                if (tmp.type == LEAF) break;
                else cur = tmp.ch[0];
            }
            while(cur)
            {
                helper.readNode(cur, tmp);
                tmp.printInfo();
                cur = tmp.next;
            }
        }

        bool find_error()//if there is an error, then return true
        {
            if (!root) return false;
            std::queue<int> que;
            que.push(root);
            int pos;
            node tmp;
            while(!que.empty())
            {
                pos = que.front(), que.pop();
                helper.readNode(pos, tmp);

                for (int i = 0; i < tmp.cnt - 1; ++i)
                {
                    if (tmp.key[i] >= tmp.key[i + 1])
                    {
                        std::cout<<"there is an error , the info is as follows：\n";
                        tmp.printInfo();
                        return true;
                    }
                }
                if (tmp.type == NODE) for (int i = 0; i <= tmp.cnt; ++i) que.push(tmp.ch[i]);
            }
            return false;
        }

        void viewAll()
        {
            if (!root)
            {
                std::cout<<"the BPT is empty\n";
                return;
            }

            std::queue<int> que;
            que.push(root);
            int pos;
            node tmp;
            while(!que.empty())
            {
                pos = que.front(), que.pop();
                helper.readNode(pos, tmp);

                tmp.printInfo();
                if (tmp.type == NODE) for (int i = 0; i <= tmp.cnt; ++i) que.push(tmp.ch[i]);
            }
        }
    };
}
#endif //TICKETSYSTEM_BPT_H
