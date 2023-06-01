#ifndef TICKETSYSTEM_TRAIN_H
#define TICKETSYSTEM_TRAIN_H
#include <iostream>
#include <cstring>
#include <filesystem>
#include "base.h"
#include "fileIO.h"
#include "../BPT/bpt.h"
#include "../STL/utility.h"
#include "../STL/vector.h"

using sjtu::diyString;
using std::string;

sjtu::BPlusTree<size_t, int> trainDataBase("train.db", "train_rec.db");
extern sjtu::BPlusTree<size_t, trainStation> stationDataBase;

normalFile<train> trainFile("trainFile.db");
TwoDArrayFile<seat_Train, seat_DateAndTrain> seatFile("seatFile.db");
template<> int TwoDArrayFile<seat_Train, seat_DateAndTrain>::cnt = 0;

//sjtu::vector<int> placeRec;
std::hash<std::string> strHash;
train tmp;
sjtu::vector<int> queryVec;

int add_train(const string *m)
{
    trainDataBase.Find(strHash(m['i']), queryVec);
    if (!queryVec.empty()) return -1;
    train objTrain;
    objTrain.trainID = m['i'];
    objTrain.fileID = ++TwoDArrayFile<seat_Train, seat_DateAndTrain>::cnt;
    objTrain.stationNum = stoi(m['n']), objTrain.seatNum = stoi(m['m']), objTrain.startTime = Clock(m['x']), objTrain.type = m['y'][0], objTrain.is_released = false;

    int pos = 0;
    string str;
    //s stations
    for (int i = 0, l = m['s'].size(); i < l; ++i)
    {
        if (m['s'][i] != '|') str += m['s'][i];
        else objTrain.stations[pos++] = str, str.clear();
    }
    objTrain.stations[pos] = str, str.clear(), pos = 0;

    //p prices(int)
    for (int i = 0, l = m['p'].size(); i < l; ++i)
    {
        if (m['p'][i] != '|') str += m['p'][i];
        else objTrain.prices[pos++] = stoi(str), str.clear();
    }
    objTrain.prices[pos] = stoi(str), str.clear(), pos = 0;

    //t travelTimes(int)
    for (int i = 0, l = m['t'].size(); i < l; ++i)
    {
        if (m['t'][i] != '|') str += m['t'][i];
        else objTrain.travelTimes[pos++] = stoi(str), str.clear();
    }
    objTrain.travelTimes[pos] = stoi(str), str.clear(), pos = 0;

    //o stopoverTimes
    if (objTrain.stationNum != 2)
    {
        for (int i = 0, l = m['o'].size(); i < l; ++i)
        {
            if (m['o'][i] != '|') str += m['o'][i];
            else objTrain.stopoverTimes[pos++] = stoi(str), str.clear();
        }
        objTrain.stopoverTimes[pos] = stoi(str), str.clear(), pos = 0;
    }

    //d saleDate
    for (int i = 0, l = m['d'].size(); i < l; ++i)
    {
        if (m['d'][i] != '|') str += m['d'][i];
        else objTrain.saleDateBegin = Date(str), str.clear();
    }
    objTrain.saleDateEnd = Date(str), str.clear();

    trainDataBase.Insert(strHash(objTrain.trainID), objTrain.fileID);
    trainFile.write(objTrain.fileID, objTrain);

    seat_DateAndTrain s;
    for (int i = 0; i < objTrain.stationNum; ++i) s[i] = objTrain.seatNum;
    seat_Train S;
    for (int i = 1; i <= 92; ++i) S[i] = s;
    seatFile.write(objTrain.fileID, S);
    return 0;
}

int delete_train(const string *m)
{
    size_t hashRes = strHash(m['i']);
    trainDataBase.Find(hashRes, queryVec);
    if (queryVec.empty()) return -1;
    trainFile.read(queryVec[0], tmp);
    if (tmp.is_released) return -1;

    trainDataBase.Delete(hashRes, tmp.fileID);
    return 0;
}

int release_train(const string *m)//改trainFile， 写stationDataBase
{
    size_t hashRes = strHash(m['i']);
    trainDataBase.Find(hashRes, queryVec);
    if (queryVec.empty()) return -1;
    trainFile.read(queryVec[0], tmp);
    if (tmp.is_released) return -1;
    tmp.is_released = true;
    trainFile.write(tmp.fileID, tmp);

    //发布前的车次，不可发售车票，无法被 query_ticket 和 query_transfer 操作所查询到
    int time = 0, price = 0;
    trainStation cur(tmp);
    for (int i = 0; i < tmp.stationNum; ++i)
    {
        cur.kth = i, cur.price = price, cur.arriveTime = time;
        if (i && i != tmp.stationNum - 1) cur.stopoverTime = tmp.stopoverTimes[i - 1];//否则为初始值0
        stationDataBase.Insert(strHash(tmp.stations[i]), cur);
        price += tmp.prices[i], time += (cur.stopoverTime + tmp.travelTimes[i]); //i == tmp.stationNum - 1时，对于price和time的访问溢出一格，访问到默认值0
        /*Moment m1(Date("07-30"), tmp.startTime);
        m1 += cur.arriveTime;
        Moment m2(m1);
        m2 += cur.stopoverTime;
        std::cout<<tmp.stations[i]<<" "<<m1<<" "<<m2<<'\n';*/
    }
    return 0;
}

int query_train(const string *m)
{
    trainDataBase.Find(strHash(m['i']), queryVec);
    if (queryVec.empty()) return -1;
    trainFile.read(queryVec[0], tmp);
    int stDay = Date(m['d']);
    if (!(stDay >= tmp.saleDateBegin && stDay <= tmp.saleDateEnd)) return -1;

    std::cout<<tmp.trainID<<' '<<tmp.type<<'\n';
    seat_DateAndTrain seatInfo;
    seatFile.read(tmp.fileID, stDay, seatInfo);
    Moment mom(Date(m['d']), Clock(tmp.startTime));
    for (int i = 0, price = 0; i < tmp.stationNum; ++i)
    {
        std::cout<<tmp.stations[i]<<' ';
        if (!i) std::cout<<"xx-xx xx:xx -> ";
        else std::cout<<mom<<" -> ";
        if (i) mom += tmp.stopoverTimes[i - 1];
        if (i != tmp.stationNum - 1) std::cout<<mom<<' ';
        else std::cout<<"xx-xx xx:xx ";
        std::cout<<price<<' ';
        if (i != tmp.stationNum - 1) std::cout<<seatInfo[i]<<'\n';
        else std::cout<<"x\n";
        price += tmp.prices[i], mom += tmp.travelTimes[i];
    }
    return 0;
}

void cleanTrain()
{
    trainDataBase.~BPlusTree();
    std::filesystem::remove("train.db");
    std::filesystem::remove("train_rec.db");
    new (&trainDataBase) sjtu::BPlusTree<diyString, int>("train.db", "train_rec.db");
    trainFile.~normalFile();
    std::filesystem::remove("trainFile.db");
    new (&trainFile) normalFile<train>("trainFile.db");
    seatFile.~TwoDArrayFile();
    std::filesystem::remove("seatFile.db");
    new (&seatFile) TwoDArrayFile<seat_Train, seat_DateAndTrain>("seatFile.db");
}

#endif //TICKETSYSTEM_TRAIN_H
