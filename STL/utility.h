#ifndef SJTU_UTILITY_HPP
#define SJTU_UTILITY_HPP

#include <utility>
#include <iostream>
#include <istream>
#include <fstream>
#include <cstring>
namespace sjtu {

    template<class T1, class T2>
    class pair {
    public:
        T1 first;
        T2 second;
        constexpr pair() : first(), second() {}
        pair(const pair &other) = default;
        pair(pair &&other) = default;
        pair(const T1 &x, const T2 &y) : first(x), second(y) {}
        template<class U1, class U2>
        pair(U1 &&x, U2 &&y) : first(x), second(y) {}
        template<class U1, class U2>
        pair(const pair<U1, U2> &other) : first(other.first), second(other.second) {}
        template<class U1, class U2>
        pair(pair<U1, U2> &&other) : first(other.first), second(other.second) {}


        //以下是自行添加的部分,实现了复制重载以及== != < <=
        pair &operator=(const pair &rhs)
        {
            if (this == &rhs) return *this;
            first = rhs.first;
            second = rhs.second;
            return *this;
        }
        bool operator==(const pair &rhs) {return first == rhs.first && second == rhs.second;}
        bool operator!=(const pair &rhs) {return !((*this) == rhs);}
        bool operator<(const pair &rhs) {return first != rhs.first ? first < rhs.first : second < rhs.second;}
        bool operator>(const pair &rhs) {return first != rhs.first ? first > rhs.first : second > rhs.second;}
        bool operator<=(const pair &rhs) {return ((*this) == rhs || (*this) < rhs);}
        bool operator>=(const pair &rhs) {return ((*this) == rhs || (*this) > rhs);}
    };

    template<class U1, class U2>
    std::ostream &operator<<(std::ostream &os, const pair<U1, U2> &rhs)
    {
        os<<"("<<rhs.first<<", "<<rhs.second<<")";
        return os;
    }

    class diyString{
    public:
        char ch[45] = {};
        diyString() {memset(ch, 0, sizeof (ch));}
        diyString(const char *rhs) {;strcpy(ch, rhs);}
        diyString(const std::string &rhs) {strcpy(ch, rhs.c_str());}
        diyString(const diyString &rhs) {strcpy(ch, rhs.ch);}
        diyString &operator=(const char *rhs) {strcpy(ch, rhs); return *this;}
        diyString &operator=(const std::string &rhs) {strcpy(ch, rhs.c_str()); return *this;}
        diyString &operator=(const diyString &rhs)
        {
            if (&rhs == this) return *this;
            strcpy(ch, rhs.ch);
            return *this;
        }
        diyString operator+(const diyString &rhs) {strcat(ch, rhs.ch); return *this;}
        operator std::string() {return std::string(ch);}
        bool operator < (const diyString &rhs) const {return strcmp(ch, rhs.ch) < 0;}
        bool operator > (const diyString &rhs) const {return strcmp(ch, rhs.ch) > 0;}
        bool operator == (const diyString &rhs) const {return strcmp(ch, rhs.ch) == 0;}
        bool operator >= (const diyString &rhs) const {return strcmp(ch, rhs.ch) >= 0;}
        bool operator <= (const diyString &rhs) const {return strcmp(ch, rhs.ch) <= 0;}
        bool operator != (const diyString &rhs) const {return strcmp(ch, rhs.ch) != 0;}
        friend std::ostream &operator<<(std::ostream &os, const diyString &rhs) {os << rhs.ch; return os;}
    };
}

#endif
