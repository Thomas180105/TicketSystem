//可调参内容：N 以及 maxSize，其中后者的调参应该在允许范围内尽可能的大（这样缓存明中率更高）

#ifndef TICKETSYSTEM_LINKEDHASHMAP_H
#define TICKETSYSTEM_LINKEDHASHMAP_H
namespace sjtu{
    //实现一个linkedHashMap，作为BPT中缓存技术的支撑
    //首先实现了传统的哈希表功能（insert find remove）
    //其次是为了实现LRU算法维护了一个由时间为基准的双链表，每次调用哈希表的基本功能时这个链表的结构都会发生变化（addTime delTime modifyTime）
    //最后是由于缓存的大小是有限制的，所以实现了一个checkSize以及popNode
    template<class T>
    class linkedHashMap{//从int（在外部文件中的下标）->T的映射,需要支持T的赋值
    public:
        struct node
        {
            node *next;
            node *timePre;
            node *timeNext;
            int key;
            T value;
        public:
            node() : next(nullptr), timePre(nullptr), timeNext(nullptr) {}
            node(const int &k, const T &v) : next(nullptr), timePre(nullptr), timeNext(nullptr), key(k), value(v) {}
            node &operator=(const node &rhs)
            {
                if (&rhs == this) return *this;
                next = rhs.next;
                timePre = rhs.timePre;
                timeNext = rhs.timeNext;
                key = rhs.key;
                value = rhs.value;
            }
        };
    private:
        const static int N = 9973;
        const static int maxSize = 100;//缓存的最大容量//TODO
        node *head[N];
        node *beg;
        node *cur;
        int curSize;
    public:
        linkedHashMap()
        {
            for (int i = 0; i < N; ++i) head[i] = nullptr;
            cur = beg = new node();
            curSize = 0;
        }
        ~linkedHashMap()
        {
            node *p = beg->timeNext, *q;
            while(p)
            {
                q = p->timeNext;
                delete p;
                p = q;
            }
            delete beg;
            curSize = 0;
        }
        bool find(int key, T &val)//val为输出参数
        {
            int pos = key % N;
            node *p = head[pos];
            while(p && p->key != key) p = p->next;
            if (p)
            {
                val = p->value;
                modifyTime(p);
                return true;
            }
            else return false;
        }
        void insert(int key, const T &val)//如果key已经存在，则直接覆盖原有值
        {
            int pos = key % N;
            node *p = head[pos];
            while(p && p->key != key) p = p->next;
            if (p)
            {
                p->value = val;
                modifyTime(p);
                return;
            }
            else
            {
                p = new node(key, val);
                p->next = head[pos];
                head[pos] = p;
                addTime(p);
                ++curSize;
            }
        }
        bool remove(const int &key, const T &val)
        {
            int pos = key % N;
            node *p = head[pos];
            while(p && !(p->key == key && p->value == val)) p = p->next;
            if (p)
            {
                remove(p);
                return true;
            }
            return false;
        }
        void remove(node *p)//为了时间戳服务的，一般删除的时候直到确切地要删哪一个结点，直接传结点即可,保证被删结点存在
        {
            int pos = p->key % N;
            node *q = head[pos];
            if (p == q) head[pos] = q->next;
            else
            {
                while(q->next != p) q = q->next;
                q->next = p->next;
            }
            delTime(p);
            delete p;
            --curSize;
        }
        int size(){return curSize;}

        bool checkSize(T &val)//如果需要调整则返回true，保证被删结点存在,一般配合insert一起调用
        {
            if (curSize > maxSize)
            {
                node *p = beg->timeNext;
                val = p->value;
                remove(p);
                return true;
            }
            return false;
        }
        bool popNode(T &val)//按时间从历史到最近进行pop,且保证此操作之后哈希表仍然是正常工作的。
        {
            if (!curSize) return false;
            node *p = beg->timeNext;
            val = p->value;
            remove(p);
            return true;
        }
    private:
        void addTime(node *p)//只维护时间戳双链表的信息，不负责开点
        {
            cur->timeNext = p;
            p->timePre = cur;
            cur = p;
        }
        void delTime(node *p)//只维护时间戳双链表，不负责结点的释放
        {
            if (p == cur) cur = cur->timePre;
            if (p->timePre) p->timePre->timeNext = p->timeNext;
            if (p->timeNext) p->timeNext->timePre = p->timePre;
            p->timeNext = p->timePre = nullptr;//一个花了3小时断点测试才找到的bug!
        }
        void modifyTime(node *p)
        {
            if (p == cur) return;
            delTime(p);
            addTime(p);
        }
    };
}
#endif //TICKETSYSTEM_LINKEDHASHMAP_H
