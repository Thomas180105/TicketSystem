//diyStream出现的问题，没有写close()导致错误，这一错误可以通过void test_diyStream(）中直观地感受出来
//~BPT()时出现了问题，不能在此处close()流对象，而是在fileSystem中close()
//node()没有写operator=，导致后续linkedHashMap的insert()需要赋值覆盖的时候出现问题
//TODO:写一个对拍的标准程序，用set实现
//TODO：写一个题目生成器
#include <iostream>
#include <fstream>
#include <ostream>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include "bpt.h"

class str{
    const static int L = 66;
public:
    char ch[L];
public:
    friend std::ostream &operator<<(std::ostream &os, const str &rhs);
    str() {}
    str(const char *rhs) {strcpy(ch, rhs);}
    str(const str &rhs) { strcpy(ch, rhs.ch);}
    str &operator=(const str &rhs)
    {
        if (this == &rhs) return *this;
        strcpy(ch, rhs.ch);
        return *this;
    }
    /*bool operator==(const str &rhs) {return !strcmp(ch, rhs.ch);}
    bool operator!=(const str &rhs) {return !(*this == rhs);}
    bool operator<(const str &rhs) {return strcmp(ch, rhs.ch) < 0;}
    bool operator>(const str &rhs) {return strcmp(ch, rhs.ch) > 0;}
    bool operator<=(const str &rhs) {return (*this == rhs || *this < rhs);}
    bool operator>=(const str &rhs) {return (*this == rhs || *this > rhs);}*/
};
inline bool operator==(const str &a, const str &b) {return !strcmp(a.ch, b.ch);}
inline bool operator!=(const str &a, const str &b) {return strcmp(a.ch, b.ch);}
inline bool operator<(const str &a, const str &b) {return strcmp(a.ch, b.ch) < 0;}
inline bool operator>(const str &a, const str &b) {return strcmp(a.ch, b.ch) > 0 ;}
inline bool operator<=(const str &a, const str &b) {return strcmp(a.ch, b.ch) <= 0;}
inline bool operator>=(const str &a, const str &b) {return strcmp(a.ch, b.ch) >= 0;}
inline std::ostream &operator<<(std::ostream &os, const str &rhs)
{
    os<<rhs.ch;
    return os;
}

//#define error_find //line 755处使用了stl::queue的库

inline int final()
{
    sjtu::BPlusTree<str, int> worker("dataFile.db", "recycleFile.db");
    sjtu::vector<int> vec;
    std::ios::sync_with_stdio(false);
    int n;
    std::cin>>n;
    std::string op;
    char index[66];
    int val;
    int copy = n;

    while(n--)
    {
        std::cin>>op;
        if (op == "insert")
        {
            std::cin>>index>>val;
            worker.Insert(str(index), val);
#ifdef error_find
            if (worker.find_error())
            {
                std::cout<<"now the num is "<<copy - n<<'\n';
                break;
            }
#endif
        }
        else if (op == "delete")
        {
            std::cin>>index>>val;
            worker.Delete(str(index), val);
#ifdef error_find
            if (worker.find_error())
            {
                std::cout<<"now the num is "<<copy - n<<'\n';
                break;
            }
//            else std::cout<<"when we do this delete, no error occur\n";
#endif
        }
        else if (op == "find")
        {
            std::cin>>index;
            worker.Find(str(index), vec);
            if (vec.empty()) std::cout<<"null\n";
            else
            {
                for (auto v : vec) std::cout<<v<<" ";
                std::cout<<'\n';
            }
#ifdef error_find
            if (worker.find_error())
            {
                std::cout<<"now the num is "<<copy - n<<'\n';
                break;
            }
#endif
        }
        else if (op == "FindAll")
        {
            worker.FindALL(vec);
            if (vec.empty()) std::cout<<"null\n";
            else
            {
                for (auto v : vec) std::cout<<v<<" ";
                std::cout<<'\n';
            }
        }
        else if (op == "print") worker.debug();
        else if (op == "all") worker.viewAll();
    }
    //这一段被注释的代码是用来测试src/user.h/cleanUser()正确性的
    /*worker.~BPlusTree();
    std::filesystem::remove("dataFile.db");
    std::filesystem::remove("recycleFile.db");
    new (&worker) sjtu::BPlusTree<str, int>("dataFile.db", "recycleFile.db");*/
    return 0;
}


inline void output(sjtu::vector<int> &vec)
{
    if (vec.empty()) std::cout<<"null\n";
    else
    {
        for (const auto v : vec) std::cout<<v<<" ";
        std::cout<<'\n';
    }
}