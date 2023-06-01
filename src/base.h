#ifndef TICKETSYSTEM_BASE_H
#define TICKETSYSTEM_BASE_H
#define TICKET_SYSTEM_TRAIN_H
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstring>
#include "../STL/utility.h"
using std::string;
using sjtu::diyString;

class user {
private:
    sjtu::diyString username, password, name, mailAddr;
public:
    int privilege;
    user() : privilege(0) {}
    user(const string &username_, const string &password_, const string &name_, const string &mailAddr_, const string &privilege_) :
            username(username_), password(password_), name(name_), mailAddr(mailAddr_), privilege(stoi(privilege_)) {}
    user(const user &rhs) : username(rhs.username), password(rhs.password), name(rhs.name), mailAddr(rhs.mailAddr), privilege(rhs.privilege) {}
    user &operator=(const user &rhs)
    {
        if (&rhs == this) return *this;
        username = rhs.username, password = rhs.password, name = rhs.name, mailAddr = rhs.mailAddr, privilege = rhs.privilege;
        return *this;
    }
    bool operator<(const user &rhs) const {return username < rhs.username;}
    bool operator>(const user &rhs) const {return username > rhs.username;}
    bool operator<=(const user &rhs) const {return username <= rhs.username;}
    bool operator>=(const user &rhs) const {return username >= rhs.username;}
    bool operator==(const user &rhs) const {return username == rhs.username;}
    bool operator!=(const user &rhs) const {return username != rhs.username;}

    bool checkPassword(const diyString &pw) {return password == pw;}
    void modifyP(const diyString &pw) {password = pw;}
    void modifyN(const diyString &n) { name = n;}
    void modifyM(const diyString &mail) { mailAddr = mail;}
    void modifyG(const int &num) {privilege = num;}
    void print() {std::cout<<username<<' '<<name<<' '<<mailAddr<<' '<<privilege<<'\n';}
    friend std::ostream &operator<<(std::ostream &os, const user &rhs)
    {
        os<<rhs.username<<' '<<rhs.name<<' '<<rhs.mailAddr<<' '<<rhs.privilege<<'\n';
        return os;
    }
};

class Date{//发车时间每一项均为 2023 年 6 月至 8 月的某一日期
public:
    static const int days[4];
    int num = 0;
    Date() {}
    Date(int n_) : num(n_) {}
    Date(const string &str)
    {
        if (str[1] < '6' || str[1] > '9') num = -1;
        else num = (str[3] - '0') * 10 + (str[4] - '0') + days[str[1] - '0' - 6];
    }
    operator string() const
    {
        if (!num) return"xx-xx";
        else if (num <= 30) return "06-" + string(num < 10 ? "0" : "") + std::to_string(num);
        else if (num <= 61) return "07-" + string(num < 40 ? "0" : "") + std::to_string(num - 30);
        else if (num <= 92) return "08-" + string(num < 71 ? "0" : "") + std::to_string(num - 61);
        else return "09-" + string(num < 102 ? "0" : "") + std::to_string(num - 92);
    }
    operator int() const {return num;}
    void print() {std::cout<<string(*this);}
    friend std::ostream &operator<<(std::ostream &os, const Date &rhs)
    {
        os<<string(rhs);
        return os;
    }
};
const int Date::days[4] = {0, 30, 61, 92};

class Clock {
public:
    int num = -1; // 特殊值不能保留为0
    Clock() {}
    Clock(int num_): num(num_) {}
    Clock(const string &s)
    {
        num = ((s[0] - '0') * 10 + s[1] - '0') * 60 + ((s[3] - '0') * 10 + s[4] - '0');
    }
    operator string() const
    {
        if (num == -1) return "xx:xx";
        else return std::string(num / 60 < 10 ? "0" : "") + std::to_string(num / 60) + ":" + std::string(num % 60 < 10 ? "0" : "") + std::to_string(num % 60);
    }
    operator int() const {return num;}
    void print() const {std::cout << string(*this);}
    friend std::ostream& operator<<(std::ostream& os, const Clock& rhs)
    {
        os<<string(rhs);
        return os;
    }
};

class Moment{
public:
    Date date;
    Clock clock;
    Moment() {}
    Moment(Date d, Clock c) : date(d), clock(c) {}
    Moment(const string &d, const string &c) : date(d), clock(c) {}
    Moment(const Moment &other): date(other.date), clock(other.clock) {}
    Moment operator+ (int x)
    {
        int t = clock, d = date, h = t / 60, m = t % 60 + x;
        h += m / 60, m %= 60;
        d += h / 24, h %= 24;
        return {d, h * 60 + m};
    }
    Moment &operator+=(int x)
    {
        int t = clock, d = date, h = t / 60, m = t % 60 + x;
        h += m / 60, m %= 60;
        d += h / 24, h %= 24;
        date = d;
        clock = h * 60 + m;
        return *this;
    }
    int operator-(const Moment &other) {return ((int)date - (int)other.date) * 24 * 60 + (int)clock - (int)other.clock;}
    void print() const {std::cout << string(date) << ' ' << string(clock);}
    friend std::ostream& operator<<(std::ostream& os, const Moment& rhs)
    {
        os<<rhs.date<<' '<<rhs.clock;
        return os;
    }
};

class base{
public:
    diyString trainID;
    int saleDateBegin, saleDateEnd;
    int seatNum, startTime;
    int fileID;//不是实际存储位置，只是一个位置编号

    base() {}
    base(const base &rhs): trainID(rhs.trainID), saleDateBegin(rhs.saleDateBegin), saleDateEnd(rhs.saleDateEnd), seatNum(rhs.seatNum), startTime(rhs.startTime), fileID(rhs.fileID) {}
    base &operator= (const base &rhs)
    {
        if (&rhs == this) return *this;
        trainID = rhs.trainID, saleDateBegin = rhs.saleDateBegin, saleDateEnd = rhs.saleDateEnd, seatNum = rhs.seatNum, startTime = rhs.startTime, fileID = rhs.fileID;
        return *this;
    }
    bool operator<(const base &rhs) const { return trainID <  rhs.trainID; }
    bool operator>(const base &rhs) const { return trainID >  rhs.trainID; }
    bool operator<=(const base &rhs) const { return trainID <= rhs.trainID; }
    bool operator>=(const base &rhs) const { return trainID >= rhs.trainID; }
    bool operator==(const base &rhs) const { return trainID == rhs.trainID; }
    bool operator!=(const base &rhs) const { return trainID != rhs.trainID; }
};

class trainStation : public base{
public:
    int price, arriveTime, stopoverTime = 0;//和前一站之间的票价，到达时间，停靠时间
    int kth;

    trainStation() {}
    trainStation(const base &rhs): base(rhs) {}
    trainStation(const trainStation &rhs): base(rhs), price(rhs.price), arriveTime(rhs.arriveTime), stopoverTime(rhs.stopoverTime), kth(rhs.kth) {}
    trainStation &operator=(const trainStation &rhs)
    {
        if (&rhs == this) return *this;
        base::operator= (rhs);
        price = rhs.price, arriveTime = rhs.arriveTime, stopoverTime = rhs.stopoverTime, kth = rhs.kth;
        return *this;
    }
};

class train : public base{
public:
    static const int N = 101;//保证火车的座位数大于 0,站的数量不少于2不多于100
    int stationNum;
    diyString stations[N];
    int prices[N], travelTimes[N], stopoverTimes[N];
    char type;
    bool is_released = false;

    train() {}
    train(const train &rhs): base(rhs), stationNum(rhs.stationNum), type(rhs.type), is_released(rhs.is_released)
    {
        memcpy(stations, rhs.stations, sizeof(stations));
        memcpy(prices, rhs.prices, sizeof(prices));
        memcpy(travelTimes, rhs.travelTimes, sizeof(travelTimes));
        memcpy(stopoverTimes, rhs.stopoverTimes, sizeof(stopoverTimes));
    }
    train &operator=(const train &rhs)
    {
        if (&rhs == this) return *this;
        base::operator= (rhs);
        stationNum = rhs.stationNum, type = rhs.type, is_released = rhs.is_released;
        memcpy(stations, rhs.stations, sizeof(stations));
        memcpy(prices, rhs.prices, sizeof(prices));
        memcpy(travelTimes, rhs.travelTimes, sizeof(travelTimes));
        memcpy(stopoverTimes, rhs.stopoverTimes, sizeof(stopoverTimes));
        return *this;
    }
};

//逻辑上是train类的一个数据成员，但是由于被频繁读写，所以单独拎出来储存
class seat_DateAndTrain{
public:
    static const int N = 102;//空间维度，”保证站的数量不少于 2 不多于 100“
    int g[N] = {};

    seat_DateAndTrain() {}
    seat_DateAndTrain(const seat_DateAndTrain &rhs) { memcpy(g, rhs.g, sizeof(g)); }
    seat_DateAndTrain(const int *p) { memcpy(g, p, sizeof(g)); }
    seat_DateAndTrain &operator=(const seat_DateAndTrain &rhs)
    {
        if (this == &rhs) return *this;
        memcpy(g, rhs.g, sizeof(g));
        return *this;
    }
    int &operator[] (int x) {return g[x];}
    const int &operator[] (int x) const {return g[x];}
};

class seat_Train{
public:
    static const int N = 97;//时间维度，”保证每一辆车从始发站到终点站用时不超过72小时, 且合法的发车时间的跨度为92天“
    seat_DateAndTrain f[N] = {};
    seat_Train() {}
    seat_DateAndTrain &operator[] (int x) {return f[x];}
};

class Order{
public:
    diyString state, trainID, userID, from_str, to_str;
    int from_int, to_int, timestamp, date, price, num;
    Moment Leaving, Arriving;
    Order() {}
    Order(const diyString &state_, const diyString &trainID_, const diyString &userID_, const diyString &from_str_, const diyString &to_str_, const int from_int_, const int to_int_,
          int timestamp_, int date_, int price_, int num_, const Moment &Leaving_, const Moment &Arriving_):
          state(state_), trainID(trainID_), userID(userID_), from_str(from_str_), to_str(to_str_), from_int(from_int_), to_int(to_int_), timestamp(timestamp_),date(date_),
          price(price_), num(num_), Leaving(Leaving_), Arriving(Arriving_) {}
    bool operator<(const Order &rhs) const {return timestamp < rhs.timestamp;}
    bool operator>(const Order &rhs) const {return timestamp > rhs.timestamp;}
    bool operator<=(const Order &rhs) const {return timestamp <= rhs.timestamp;}
    bool operator>=(const Order &rhs) const {return timestamp >= rhs.timestamp;}
    bool operator==(const Order &rhs) const {return timestamp == rhs.timestamp;}
    bool operator!=(const Order &rhs) const {return timestamp != rhs.timestamp;}
    friend std::ostream& operator<<(std::ostream& os, const Order& rhs)//最后带换行
    {
        os<<rhs.state<<' '<<rhs.trainID<<' '<<rhs.from_str<<' '<<rhs.Leaving<<" -> "<<rhs.to_str<<' '<<rhs.Arriving<<' '<<rhs.price<<' '<<rhs.num<<'\n';
        return os;
    }
};

#endif //TICKETSYSTEM_BASE_H
