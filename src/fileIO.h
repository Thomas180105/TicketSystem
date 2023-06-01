#ifndef TICKETSYSTEM_FILEIO_H
#define TICKETSYSTEM_FILEIO_H

#include <fstream>
using std::fstream;

//使用时需要初始化cnt
template <typename T1, typename T2>//T1是大类（二维数组），T2是小类（二维数组的第一维退化之后的一维数组）
class TwoDArrayFile {
private:
    fstream helper;
public:
    static int cnt;
    TwoDArrayFile(const char dateBase[])
    {
        helper.open(dateBase, fstream::in);
        bool flag = helper.is_open();
        helper.close();
        if (!flag)
        {
            helper.open(dateBase, fstream::out);
            helper.close();
            helper.open(dateBase, fstream::in | fstream::out | fstream::binary);
            cnt = 0;
            helper.seekp(0);
            helper.write(reinterpret_cast<const char *>(&cnt), sizeof(int));
        }
        else
        {
            helper.open(dateBase, fstream::in | fstream::out | fstream::binary);
            helper.seekg(0);
            helper.read(reinterpret_cast<char *>(&cnt), sizeof(int));
        }
    }
    ~TwoDArrayFile()
    {
        helper.seekp(0);
        helper.write(reinterpret_cast<const char *>(&cnt), sizeof(int));
    }
    void read(int fileID, int date, T2 &obj)
    {
        helper.seekg(sizeof(int) + (fileID - 1) * sizeof(T1) + (date) * sizeof(T2));
        helper.read(reinterpret_cast<char *>(&obj), sizeof(obj));
    }
    void write(int fileID, int date, const T2 &obj)
    {
        helper.seekp(sizeof(int) + (fileID - 1) * sizeof(T1) + (date) * sizeof(T2));
        helper.write(reinterpret_cast<const char *>(&obj), sizeof(obj));
    }
    void write(int fileID, const T1 &obj)
    {
        helper.seekg(sizeof(int) + (fileID - 1) * sizeof(T1));
        helper.write(reinterpret_cast<const char *>(&obj), sizeof(obj));
    }
};

template<class T>
class normalFile{
private:
    fstream helper;
public:
    explicit normalFile(const char dataBase[])
    {
        helper.open(dataBase, fstream::in);
        bool flag = helper.is_open();
        helper.close();
        if (!flag)
        {
            helper.open(dataBase, fstream::out);
            helper.close();
        }
        helper.open(dataBase, fstream::in | fstream::out | fstream::binary);
    }
    ~normalFile() {}
    void read(int fileID, T &obj)
    {
        helper.seekg((fileID - 1) * sizeof(T));
        helper.read(reinterpret_cast<char *>(&obj), sizeof(obj));
    }
    void write(int fileID, const T &obj)
    {
        helper.seekg((fileID - 1) * sizeof(T));
        helper.write(reinterpret_cast<const char *>(&obj), sizeof(obj));
    }
};




#endif //TICKETSYSTEM_FILEIO_H
