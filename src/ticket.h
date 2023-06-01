#ifndef TICKETSYSTEM_TICKET_H
#define TICKETSYSTEM_TICKET_H
#include <iostream>
#include <cstring>
#include <filesystem>
#include "base.h"
#include "fileIO.h"
#include "user.h"
#include "../BPT/bpt.h"
#include "../STL/utility.h"
#include "../STL/vector.h"

using sjtu::diyString;
using std::string;
sjtu::BPlusTree<size_t, trainStation> stationDataBase("trainStation.db", "trainStation_rec.db");
sjtu::BPlusTree<size_t, Order> OrderDataBase("OrderDataBase.db", "OrderDataBase_rec.db");//从strHash(userId)到对应的order
sjtu::BPlusTree<int, int> waitDataBase("waitDataBase.db", "waitDataBase_rec.db");//基于外存的一个有序vector，存放时间戳
normalFile<Order> waitFile("waitFile.db");
extern sjtu::BPlusTree<size_t, int> trainDataBase;
extern normalFile<train> trainFile;
extern TwoDArrayFile<seat_Train, seat_DateAndTrain> seatFile;
extern std::hash<std::string> strHash;

sjtu::vector<int> queryRec;
sjtu::vector<Order> orderVec;

void cleanTicket()
{
    stationDataBase.~BPlusTree();
    std::filesystem::remove("trainStation.db");
    std::filesystem::remove("trainStation_rec.db");
    new (&stationDataBase) sjtu::BPlusTree<size_t, trainStation>("trainStation.db", "trainStation_rec.db");

    OrderDataBase.~BPlusTree();
    std::filesystem::remove("OrderDataBase.db");
    std::filesystem::remove("OrderDataBase_rec.db");
    new (&OrderDataBase) sjtu::BPlusTree<size_t, Order>("OrderDataBase.db", "OrderDataBase_rec.db");

    waitDataBase.~BPlusTree();
    std::filesystem::remove("waitDataBase.db");
    std::filesystem::remove("waitDataBase_rec.db");
    new (&waitDataBase) sjtu::BPlusTree<int, int>("waitDataBase.db", "waitDataBase_rec.db");

    waitFile.~normalFile();
    std::filesystem::remove("waitFile.db");
    new (&waitFile) normalFile<Order>("waitFile.db");
}

sjtu::vector<trainStation> stVec, edVec;
bool comp(int key, int obj_key, const diyString &str, const diyString &obj_str)
{
    if (key != obj_key) return key > obj_key;
    return str > obj_str;
}

void sort(sjtu::vector<std::pair<int, int>> &vec, int *key, int l, int r)
{
    if (r - l <= 1) return;
    int pos1 = l, pos2 = r - 1;
    int obj_key = key[l];
    diyString obj_str = stVec[vec[l].first].trainID;
    while (pos1 < pos2)
    {
        while (pos2 > pos1 && comp(key[pos2], obj_key, stVec[vec[pos2].first].trainID, obj_str)) --pos2;
        std::swap(vec[pos1], vec[pos2]), std::swap(key[pos1], key[pos2]);
        while (pos1 < pos2 && !comp(key[pos1], obj_key, stVec[vec[pos1].first].trainID, obj_str)) ++pos1;
        std::swap(vec[pos1], vec[pos2]), std::swap(key[pos1], key[pos2]);
    }
    sort(vec, key, l, pos1);
    sort(vec, key, pos1 + 1, r);
}

class Ticket {
    diyString trainID;
    int fileID;
    Moment Leaving; // 从该路途起点出发的日期 && 时刻
    diyString ST, ED;
    int st, ed, date, price, time;//date指列车从始发站出发的日期
public:
    Ticket() {}
    ~Ticket() {}
    Ticket(diyString trainID_, int fileID_, Moment Leaving_, diyString ST_, diyString ED_, int st_, int ed_, int date_, int price_, int time_) :
            trainID(trainID_), fileID(fileID_), Leaving(Leaving_), ST(ST_), ED(ED_), st(st_), ed(ed_), date(date_), price(price_), time(time_) {}
    friend std::ostream& operator<<(std::ostream& os, const Ticket& rhs)//最后带换行
    {
        os<<rhs.trainID<<' '<<rhs.ST<<' '<<rhs.Leaving<<" -> "<<rhs.ED<<' ';
        Moment tmp(rhs.Leaving); tmp += rhs.time;
        os<<tmp<<' '<<rhs.price<<' ';
        int seat = 1e6;
        seat_DateAndTrain tmpSeat;
        seatFile.read(rhs.fileID, rhs.date, tmpSeat);
        for (int i = rhs.st; i < rhs.ed; ++i) seat = std::min(seat, tmpSeat[i]);
        os<<seat<<'\n';
        return os;
    }
};

int query_ticket(const string *m)
{
    diyString ST(m['s']), ED(m['t']);
    int D = Date(m['d']);
    int op = (m['p'] == "time" || m['p'].empty());
    sjtu::vector<std::pair<int, int>> v;
    stationDataBase.Find(strHash(ST), stVec);//trainID为严格升序排列
    stationDataBase.Find(strHash(ED), edVec);
    int pos1 = 0, pos2 = 0, l1 = stVec.size(), l2 = edVec.size(), tot = 0;
    while(pos1 < l1 && pos2 < l2)
    {
        if (stVec[pos1].trainID < edVec[pos2].trainID) ++pos1;
        else if (stVec[pos1].trainID > edVec[pos2].trainID) ++pos2;
        else
        {
            if (stVec[pos1].kth >= edVec[pos2].kth) {++pos1, ++pos2; continue;}//非常重要！！！
            Moment arr = Moment(D, stVec[pos1].startTime) + (stVec[pos1].arriveTime + stVec[pos1].stopoverTime);
            int date = 2 * D - arr.date;
            if (date < stVec[pos1].saleDateBegin || date > stVec[pos1].saleDateEnd) {++pos1, ++pos2; continue;}//非常重要！！！
            v.push_back(std::make_pair(pos1, pos2));
            ++tot, ++pos1, ++pos2;
        }
    }

    int *firstKey = new int [tot];
    int *secondKey = new int [tot];
    for (int i = 0; i < tot; ++i)
    {
        int x = v[i].first, y = v[i].second;
        if (op) firstKey[i] = edVec[y].arriveTime - stVec[x].arriveTime - stVec[x].stopoverTime;
        else firstKey[i] = edVec[y].price - stVec[x].price;
    }

    sort(v, firstKey, 0, tot);
    std::cout<<tot<<'\n';
    for (int i = 0; i < tot; ++i)
    {
        int x = v[i].first, y = v[i].second;
        if (op) secondKey[i] = edVec[y].price - stVec[x].price;
        else secondKey[i] = edVec[y].arriveTime - stVec[x].arriveTime - stVec[x].stopoverTime;
        Moment arr = Moment(D, stVec[x].startTime) + (stVec[x].arriveTime + stVec[x].stopoverTime);
        int date = 2 * D - arr.date;
        arr.date = D;
        if (op) std::cout<<Ticket(stVec[x].trainID, stVec[x].fileID, arr, ST, ED, stVec[x].kth, edVec[y].kth, date, secondKey[i], firstKey[i]);
        else std::cout<<Ticket(stVec[x].trainID, stVec[x].fileID, arr, ST, ED, stVec[x].kth, edVec[y].kth, date, firstKey[i], secondKey[i]);
    }
    delete [] firstKey;
    delete [] secondKey;
    return 0;
}

bool cmp(int op, int p1, int p2, int bp, int bt, int i, int j, int pri, int ti) {
    if (op)
    {//time
        if (bt != ti) return ti < bt;
        if (bp != pri) return pri < bp;
    }
    else
    {
        if (bp != pri) return pri < bp;
        if (bt != ti) return ti < bt;
    }
    if (stVec[p1].trainID != stVec[i].trainID) return stVec[i].trainID < stVec[p1].trainID;
    return edVec[j].trainID < edVec[p2].trainID;
}


int query_transfer(const string *m)
{
    diyString ST(m['s']), ED(m['t']);
    int D = Date(m['d']);
    int op = (m['p'] == "time" || m['p'].empty());
    sjtu::vector<std::pair<int, int>> v;
    stationDataBase.Find(strHash(ST), stVec);//trainID为严格升序排列
    stationDataBase.Find(strHash(ED), edVec);
    int l1 = stVec.size(), l2 = edVec.size();
    if (!l1 || !l2) {std::cout<<"0\n"; return 0;}
    bool flag = false;
    int p1 = -1, p2 = -1;
    train A, B;
    int bestPrice = 0, bestTime = 0;
    Ticket firstTicket, secondTicket;
    for (int i = 0; i < l1; ++i)
    {
        //这一层绑定了一个与起点关联的火车
        trainDataBase.Find(strHash(stVec[i].trainID), queryRec);
        trainFile.read(queryRec[0], A);
        int s1 = stVec[i].kth, t1 = A.stationNum - 1;
        Moment lea = Moment(D, A.startTime) + (stVec[i].arriveTime + stVec[i].stopoverTime);
        int date = 2 * D - lea.date;
        lea.date = D;
        if (date < A.saleDateBegin || date > A.saleDateEnd) continue;
        Moment lea_st(lea);//上A车的ddl

        for (int k = 0; k < l2; ++k)
        {
            //这一层绑定了一个和终点关联的火车
            if (edVec[k].trainID == A.trainID) continue;
            trainDataBase.Find(strHash(edVec[k].trainID), queryRec);
            trainFile.read(queryRec[0], B);
            int t2 = edVec[k].kth;
            for (int j = s1 + 1; j <= t1; ++j)
            {
                //这一层绑定了中转站(A->j b->s2) (A: s1 -> j  B : s2 -> t2)
                diyString G(A.stations[j]);
                int s2 = -1;
                for (int t = 0; t < B.stationNum; ++t) if (B.stations[t] == G) s2 = t;
                if (s2 == -1 || s2 >= t2) continue;//注意不是s2 > t2
                int price = 0, tmpTime = 0;
                for (int t = s1; t < j; ++t) price += A.prices[t], tmpTime += A.travelTimes[t];
                for (int t = s1; t < j - 1; ++t) tmpTime += A.stopoverTimes[t];
                int price_first = price, time_first = tmpTime;
                Moment arr_mid = lea_st + tmpTime;//到达G
                int da = arr_mid.date;
                tmpTime = 0;
                Moment lea_mid(arr_mid.date, B.startTime);
                for (int t = 0; t < s2; ++t) tmpTime += B.travelTimes[t] + B.stopoverTimes[t];
                lea_mid += tmpTime;
                da -= int(lea_mid.date) - da;
                lea_mid.date = arr_mid.date;
                if (lea_mid - arr_mid < 0) ++da, ++lea_mid.date.num;
                if (da < B.saleDateBegin)
                {
                    lea_mid.date.num += (B.saleDateBegin - da);
                    da = B.saleDateBegin;
                }
                else if (da > B.saleDateEnd) continue;

                //Moment so = lea_mid;
                int time = 0;
                for (int t = s2; t < t2; ++t) price += B.prices[t], time += B.travelTimes[t];
                for (int t = s2; t < t2 - 1; ++t) time += B.stopoverTimes[t];
                Moment arr_fin = lea_mid + time;//o : arr_fin so:lea_mid
                if (!flag || cmp(op, p1, p2, bestPrice, bestTime, i, j, price, arr_fin - lea_st))
                {
                    p1 = i, p2 = k, flag = true, bestPrice = price, bestTime = arr_fin - lea_st;
                    firstTicket = Ticket(stVec[i].trainID, stVec[i].fileID, lea_st, ST, G, s1, j, date, price_first, time_first);
                    secondTicket = Ticket(edVec[k].trainID, edVec[k].fileID, lea_mid, G, ED, s2, t2, da, price - price_first, time);
                }
            }
        }
    }
    if (!flag) {std::cout<<"0\n"; return 0;}
    std::cout<<firstTicket<<secondTicket;
    return 0;
}

int buy_ticket(const string *m)
{
    if (!isLogin(m['u'])) return -1;
    trainDataBase.Find(strHash(m['i']), queryRec);
    if (queryRec.empty()) return -1;
    train A;
    trainFile.read(queryRec[0], A);
    if (!A.is_released) return -1;
    int num = stoi(m['n']);
    if (num > A.seatNum) return -1;
    diyString ST(m['f']), ED(m['t']);
    int D = Date(m['d']), date = D;
    int st = -1, ed = -1;
    for (int i = 0; i < A.stationNum; ++i)
    {
        if (A.stations[i] == ST) st = i;
        if (A.stations[i] == ED) ed = i;
    }
    if (st == -1 || ed == -1 || ed <= st) return -1;
    Moment arr(D, A.startTime);
    for (int i = 0; i < st; ++i) arr += A.stopoverTimes[i] + A.travelTimes[i];
    date -= arr.date - date;
    if (date < A.saleDateBegin || date > A.saleDateEnd) return -1;
    arr.date = D;
    Moment lea = arr;
    for (int i = st; i < ed; ++i) lea += A.travelTimes[i];
    for (int i = st; i < ed - 1; ++i) lea += A.stopoverTimes[i];
    int price = 0, seat = 1e6;
    seat_DateAndTrain tmpSeat;
    seatFile.read(A.fileID, date, tmpSeat);
    for (int i = st; i < ed; ++i)
    {
        price += A.prices[i];
        seat = std::min(tmpSeat[i], seat);
    }

    if (seat >= num)
    {
        Order cur("[success]", m['i'], m['u'], ST, ED, st, ed, stoi(m[0]), date, price, num, arr, lea);
        for (int i = st; i < ed; ++i) tmpSeat[i] -= num;
        seatFile.write(A.fileID, date, tmpSeat);
        OrderDataBase.Insert(strHash(m['u']), cur);
        std::cout<<1ll * num * price<<'\n';
    }
    else
    {
        if (m['q'] == "false" || !m['q'].size()) return -1;
        Order cur("[pending]", m['i'], m['u'], ST, ED, st, ed, stoi(m[0]), date, price, num, arr, lea);
        OrderDataBase.Insert(strHash(m['u']), cur);
        waitDataBase.Insert(cur.timestamp, cur.timestamp);
        waitFile.write(cur.timestamp, cur);
        std::cout<<"queue\n";
    }
    return 0;
}

int query_order(const string *m)
{
    if (!isLogin(m['u'])) return -1;
    OrderDataBase.Find(strHash(m['u']), orderVec);
    std::cout<<orderVec.size()<<'\n';
    for (int i = orderVec.size() - 1; i >= 0; --i) std::cout<<orderVec[i];
    return 0;
}

int refund_ticket(const string *m)
{
    if (!isLogin(m['u'])) return -1;
    int n = (m['n'].size() ? stoi(m['n']) : 1);
    OrderDataBase.Find(strHash(m['u']), orderVec);
    n = orderVec.size() - n;
    if (n < 0) return -1;
    Order objOrder = orderVec[n];
    if (objOrder.state == "[refunded]") return -1;
    OrderDataBase.Delete(strHash(m['u']), objOrder);

    if (objOrder.state == "[pending]") waitDataBase.Delete(objOrder.timestamp, objOrder.timestamp);
    else
    {
        seat_DateAndTrain tmpSeat;
        train A;
        trainDataBase.Find(strHash(objOrder.trainID), queryRec);
        trainFile.read(queryRec[0], A);
        seatFile.read(A.fileID, objOrder.date, tmpSeat);
        int st = objOrder.from_int, ed = objOrder.to_int;
        for (int i = st; i < ed; ++i) tmpSeat[i] += objOrder.num;
        seatFile.write(A.fileID, objOrder.date, tmpSeat);

        //开始检查候补队列
        sjtu::vector<int> waitQueue;
        waitDataBase.FindALL(waitQueue);//已经测试过，FIndAll功能正常
        Order B;
        for (int i = 0, l = waitQueue.size(); i < l; ++i)
        {
            waitFile.read(waitQueue[i], B);
            trainDataBase.Find(strHash(B.trainID), queryRec);
            trainFile.read(queryRec[0], A);
            st = B.from_int, ed = B.to_int;
            int seat = 1e6;
            seatFile.read(A.fileID, B.date, tmpSeat);
            for (int j = st; j < ed; ++j)
            {
                seat = std::min(seat, tmpSeat[j]);
                if (seat < B.num) break;
            }
            if (seat < B.num) continue;

            for (int j = st; j < ed; ++j) tmpSeat[j] -= B.num;
            seatFile.write(A.fileID, B.date, tmpSeat);
            waitDataBase.Delete(B.timestamp, B.timestamp);
            OrderDataBase.Delete(strHash(B.userID), B);
            B.state = "[success]";
            OrderDataBase.Insert(strHash(B.userID), B);
        }
    }
    objOrder.state = "[refunded]";//注意不可以提前！！！
    OrderDataBase.Insert(strHash(m['u']), objOrder);
    return 0;
}

#endif //TICKETSYSTEM_TICKET_H
