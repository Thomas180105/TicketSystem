//感谢蒋老师推荐的博客https://www.luogu.com.cn/blog/bfqaq/qian-tan-hong-hei-shu
//尧神指出将指针作为node()的参数传入有巨大风险，正确修改了node构造函数
//在陈一星的建议下，参照算法导论的做法引入NIL，即代码中的markNode
//erase_fix中交换结点一开始写的是直接交换，王鲲鹏指出这样会导致iterator异常，需要写一个函数专门来“彻底地”交换两个结点

//一些简要的工程经验：将指针作为node()的参数传入有巨大风险、尤其当这个指针指向临时变量时
//内嵌类要使用外部类的函数时，外部类的这一个函数需要是static类型的
//函数传值时想清楚类型，该传指针的引用的时候不能只传指针
//在copy()时，注意是三叉链表，写法与二叉链表不同
//写leftNode的时候，注意并不是和写rightNode完全等价，这个教训很重要
#include <functional>
#include <cstddef>
#include "utility.h"
#include "exceptions.h"

//Key = Integer, T = std::string, Compare = Compare
namespace sjtu {
    template<
            class Key,
            class T,
            class Compare = std::less<Key>
    >class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<const Key, T> value_type;
        struct node{
            value_type *data;
            int colour;//0 is black(default), 1 is red
            node *fa;
            node *ch[2];//ch[0] is the lChild, and ch[1] is the rChild
            node (const value_type &element, int c = 1, node *lt = nullptr, node *rt = nullptr, node *pa = nullptr) : colour(c), fa(pa)
            {
                data = new value_type(element.first,element.second), ch[0] = lt, ch[1] = rt;
            }
            node (const value_type &element, node *t) : colour(1)
            {
                data = new value_type(element.first,element.second);
                fa = ch[0] = ch[1] = t;
            }
            node () : colour(1), fa(nullptr), data(nullptr)
            {
                ch[0] = ch[1] =nullptr;
            }
            ~node()
            {
                if (data != nullptr) delete data;
                data = nullptr;
                fa = ch[0] = ch[1] = nullptr;
            }
        };
        static node *leftNode(node *t, node *mark, node *root)//中序遍历中的前序节点
        {
            node *cur = nullptr;
            if (t == mark)
            {
                cur = root;
                while(cur->ch[1] != mark) cur = cur->ch[1];
                if (cur != mark) return cur;
                else throw invalid_iterator();
            }
            if (t->ch[0] != mark)
            {
                cur = t->ch[0];
                while(cur->ch[1] != mark) cur = cur->ch[1];
            }
            else
            {
                cur = t;
                while(cur->fa != mark && cur->fa->ch[1] != cur) //not the rChild of his father
                    cur = cur->fa;
                if (cur->fa == mark) throw invalid_iterator();
                cur = cur->fa;
            }
            return cur;
        }
        static node *rightNode(node *t, node *mark)//中序遍历中的后序节点
        {
            if (t == mark) throw invalid_iterator();
            node *cur = nullptr;
            if (t->ch[1] != mark)
            {
                cur = t->ch[1];
                while(cur ->ch[0] != mark) cur = cur->ch[0];
            }
            else
            {
                cur = t;
                while(cur->fa != mark && cur->fa->ch[0] != cur)
                    cur = cur->fa;
                cur = cur->fa;
            }
            return cur;
        }

        node *rt;//根节点
        node *markNode;
        int sz;
        Compare compare;

//        这个也是非常神奇的错误，当p1和p2存在父子关系时，会完全乱套
        /*void bounding(node *& son, node *&fa, int pos)
        {
            if (son != markNode) son->fa = fa;
            if (fa != markNode) fa->ch[pos] = son;
        }
        void swap_Node(node *& p1, node *&p2)
        {
            int x = (p1->fa != markNode) ? getPosition(p1) : 0;
            int y = (p2->fa != markNode) ? getPosition(p2) : 0;
            std::swap(p1->fa, p2->fa);
            bounding(p1, p1->fa, y), bounding(p2, p2->fa, x);
            std::swap(p1->ch, p2->ch);
            for (int i = 0; i < 2; ++i)
            {
                bounding(p1->ch[i], p1, i), bounding(p2->ch[i], p2, i);
            }
            std::swap(p1->colour, p2->colour);
            //sz未进行swap
            if (rt == p1) rt = p2;
            if (rt == p2) rt = p1;
            std::swap(x, y);
        }*/
        /*void swap_node(node*& x, node*& y)
        {
            //维护ch[0], ch[1], fa
            bool a = (x->fa != markNode) ? getPosition(x) : 0;
            bool b = (y->fa != markNode) ? getPosition(y) : 0;
            if (x->fa != y && y->fa != x)
            {
                std::swap(x->fa, y->fa);
                bounding(x, x->fa, b), bounding(y, y->fa, a);
                std::swap(x->ch, y->ch);
                for (int i = 0; i < 2; ++i) bounding(x->ch[i], x, i), bounding(y->ch[i], y, i);
            }
            else if (x->fa == y)
            {
                //x的不正常儿子&y的不正常父亲
                x->ch[a] = y, y->fa = x;
                //y的不正常儿子
                y->ch[a] = x->ch[a];
//                if (y->ch[a] != markNode) y->ch[a]->fa = y;
                //x的不正常父亲
                x->fa = y->fa;
//                if (x->fa != markNode) x->fa->ch[b] = y;
                //另一个正常的儿子
                std::swap(x->ch[!a], y->ch[!a]);
                bounding(x->ch[!a], x, !a), bounding(y->ch[!a], y, !a);
            }
            else if (y->fa == x)
            {
                //x的不正常父亲&y的不正常儿子
                y->ch[b] = x, x->fa = y;
                //x的不正常儿子
                x->ch[b] = y->ch[b];
//                if (x->ch[b] != markNode) x->ch[b]->fa = x;
                //y的不正常父亲
                y->fa = x->fa;
//                if (y->fa != markNode) y->fa->ch[a] = x;
                //另一个正常的儿子
                std::swap(x->ch[!b], y->ch[!b]);
                bounding(x->ch[!b], x, !b), bounding(y->ch[!b], y, !b);
            }
            //colour、rt、x & y
            std::swap(x->colour, y->colour);
            if (rt == x) rt = y;
            else if (rt == y) rt = x;
            std::swap(x, y);
        }*/
        void swap_node(node*& x, node*& y) {
            //维护ch[0], ch[1], fa
            if (y->ch[0] != markNode && y->ch[0] != x) y->ch[0]->fa = x;
            if (y->ch[1] != markNode && y->ch[1] != x) y->ch[1]->fa = x;
            if (x->ch[0] != markNode && x->ch[0] != y) x->ch[0]->fa = y;
            if (x->ch[1] != markNode && x->ch[1] != y) x->ch[1]->fa = y;
            if (x->fa != markNode && x->fa != y) x->fa->ch[getPosition(x)] = y;
            if (y->fa != markNode && y->fa != x) y->fa->ch[getPosition(y)] = x;
            if (x->fa != y && y->fa != x) {
                std::swap(x->ch[0], y->ch[0]);
                std::swap(x->ch[1], y->ch[1]);
                std::swap(x->fa, y->fa);
            }
            if (x->fa == y) {
                bool p = getPosition(x);
                y->ch[p] = x->ch[p]; x->ch[p] = y;
                std::swap(x->ch[!p], y->ch[!p]);
                x->fa = y->fa; y->fa = x;
            }
            if (y->fa == x) {
                bool p = getPosition(y);
                x->ch[p] = y->ch[p]; y->ch[p] = x;
                std::swap(x->ch[!p], y->ch[!p]);
                y->fa = x->fa; x->fa = y;
            }
            //colour、rt、x & y
            std::swap(x->colour, y->colour);
            if (rt == x) rt = y;
            else if (rt == y) rt = x;
            std::swap(x, y);
        }
        static bool getPosition(node *ptr)//前提：需确保当前结点有父亲节点，否则调用函数将会带来问题
        {
            return ptr->fa->ch[1] == ptr;
        }
        //rotate(o, 1)表示以o为根，进行右旋；rotate（o，0）表示以o为根，进行左旋
        void rotate(node *ptr, bool c)    //TODO:如果s1 == nullptr怎么办？
        {
            //      |                       |
            //      N                       S
            //     / \     l-rotate(N)     / \
            //    L   S    ==========>    N   R
            //       / \                 / \
            //      M   R               L   M
            //
            //        |                   |
            //        N                   S
            //       / \   r-rotate(N)   / \
            //      S   R  ==========>  L   N
            //     / \                     / \
            //    L   M                   M   R
            //注意维护root
            node *s1 = ptr->ch[!c];
            ptr->ch[!c] = s1->ch[c];
            if (s1->ch[c] != markNode) s1->ch[c]->fa = ptr;
            s1->fa = ptr->fa;
            if (ptr->fa == markNode) rt = s1;
            else ptr->fa->ch[getPosition(ptr)] = s1;
            s1->ch[c] = ptr;
            ptr->fa = s1;
        }
        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */

        class const_iterator;
        class iterator {
        public:
            /**
             * TODO add data members
             *   just add whatever you want.
             */
            node *ptr;
            const map *belong;
        public:
            node *getPtr() {return ptr;}
            iterator(node *p, const map *b) : ptr(p), belong(b) {}
            iterator(const map *b) : ptr(nullptr), belong(b) {}
            iterator() : ptr(nullptr), belong(nullptr) {}
            iterator(const iterator &other) : ptr(other.ptr), belong(other.belong) {}
            /**
             * TODO iter++
             */
            iterator operator++(int)
            {
                iterator res = *this;
                ptr = rightNode(ptr, belong->markNode);
                return res;
            }

            iterator & operator++()
            {
                ptr = rightNode(ptr, belong->markNode);
                return *this;
            }

            iterator operator--(int)
            {
                iterator res = *this;
                ptr = leftNode(ptr, belong->markNode, belong->rt);
                return res;
            }
            /**
             * TODO --iter
             */
            iterator & operator--()
            {
                ptr = leftNode(ptr, belong->markNode, belong->rt);
                return *this;
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            value_type & operator*() const
            {
                return *(ptr->data);
            }
            bool operator==(const iterator &rhs) const {return ptr == rhs.ptr;}
            bool operator==(const const_iterator &rhs) const {return ptr == rhs.ptr;}
            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const {return ptr != rhs.ptr;}
            bool operator!=(const const_iterator &rhs) const {return ptr != rhs.ptr;}

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            value_type* operator->() const noexcept
            {
                return ptr->data;
            }
        };
        class const_iterator {//在重载*和->处添加了const修饰符
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        public:
            // data members.
            node *ptr;
            const map* belong;
        public:
            const_iterator(node *p, const map *b) : ptr(p), belong(b) {}
            const_iterator(const map *b) : ptr(nullptr), belong(b) {}
            const_iterator() : ptr(nullptr), belong(nullptr) {}
            const_iterator(const iterator &other) : ptr(other.ptr), belong(other.belong) {}
            const_iterator(const const_iterator &other) : ptr(other.ptr), belong(other.belong) {}

            // And other methods in iterator.
            // And other methods in iterator.
            // And other methods in iterator.
            const_iterator operator++(int)
            {
                const_iterator res = *this;
                ptr = rightNode(ptr, belong->markNode);
                return res;
            }
            const_iterator & operator++()
            {
                ptr = rightNode(ptr, belong->markNode);
                return *this;
            }
            const_iterator operator--(int)
            {
                const_iterator res = *this;
                ptr = leftNode(ptr, belong->markNode, belong->rt);
                return res;
            }
            const_iterator & operator--()
            {
                ptr = leftNode(ptr, belong->markNode, belong->rt);
                return *this;
            }
            value_type operator*() const//TODO:
            {
                return *(ptr->data);
            }
            bool operator==(const iterator &rhs) const {return ptr == rhs.ptr;}
            bool operator==(const const_iterator &rhs) const {return ptr == rhs.ptr;}

            bool operator!=(const iterator &rhs) const {return ptr != rhs.ptr;}
            bool operator!=(const const_iterator &rhs) const {return ptr != rhs.ptr;}

            const value_type* operator->() const noexcept
            {
                return ptr->data;
            }
        };
        /**
         * TODO two constructors
         */
        void init_markNode()
        {
            markNode = new node;
            markNode->colour = 0;
            markNode->ch[0] = markNode->ch[1] = markNode->fa = markNode;
            rt = markNode;
            sz = 0;
        }
        map()
        {
            init_markNode();
        }
        /*
        //以下写法有重要问题，即本结点的fa连接有问题，因此以下这种写法不适合于三叉链表
        node *copy(node *rhs, node *rhs_markNode)
        {
            if (rhs == rhs_markNode) return markNode;
            return new node(*(rhs->data), rhs->colour, copy(rhs->ch[0], rhs_markNode), copy(rhs->ch[1], rhs_markNode), (rhs->fa == rhs_markNode ? markNode : rhs->fa));
        }*/

        void copy(node*& ptr, node* other, node* other_nullNode)
        {
            if (other == other_nullNode)
            {
                ptr = markNode;
                return;
            }

            ptr = new node (*(other->data), other->colour, markNode, markNode, markNode);
            copy(ptr->ch[0], other->ch[0], other_nullNode);
            if (ptr->ch[0] != markNode) ptr->ch[0]->fa = ptr;
            copy(ptr->ch[1], other->ch[1], other_nullNode);
            if (ptr->ch[1] != markNode) ptr->ch[1]->fa = ptr;
        }
        void clear(node *ptr)
        {
            if (ptr == markNode) return;
            clear(ptr->ch[0]);
            clear(ptr->ch[1]);
            delete ptr;
        }
        map(const map &other)
        {
            init_markNode();
//            rt = copy(other.rt, other.markNode);
            rt = markNode;
            copy(rt, other.rt, other.markNode);
            sz = other.sz;
        }
        /**
         * TODO assignment operator
         */
        map & operator=(const map &other)
        {
            if (this == &other) return *this;
            clear(rt);
//            rt = copy(other.rt, other.markNode);
            rt = markNode;
            copy(rt, other.rt, other.markNode);
            sz = other.sz;
            return *this;
        }
        /**
         * TODO Destructors
         */
        ~map()
        {
            clear(rt);
            delete markNode;
            sz = 0;
        }
        /**
         * TODO
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
        T & at(const Key &key)
        {
            node *cur = rt;
            while(cur != markNode)
            {
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return cur->data->second;
            }
            throw index_out_of_bound();
        }
        const T & at(const Key &key) const
        {
            node *cur = rt;
            while(cur != markNode)
            {
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return cur->data->second;
            }
            throw index_out_of_bound();
        }
        /**
         * TODO
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */

        T & operator[](const Key &key)
        {
            node *cur = rt;
            node *his = markNode;
            while(cur != markNode)
            {
                his = cur;
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return cur->data->second;
            }
            cur = new node(value_type (key, T()) , markNode);
            if (his != markNode) his->ch[compare(his->data->first, key)] = cur;
            else rt = cur;
            cur->fa = his;
            //更新大小
            ++sz;
            insert_fix(cur);
            return cur->data->second;
        }
        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const T & operator[](const Key &key) const
        {
            node *cur = rt;
            while(cur != markNode)
            {
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return cur->data->second;
            }
            throw index_out_of_bound();
        }
        /**
         * return a iterator to the beginning
         */
        iterator begin()
        {
            iterator res(this);
            if (rt != markNode)
            {
                node *tmp = rt;
                while (tmp->ch[0] != markNode) tmp = tmp->ch[0];
                res.ptr = tmp;
            }
            else res.ptr = markNode;
            return res;
        }
        const_iterator cbegin() const
        {
            const_iterator res(this);
            if (rt != markNode)
            {
                node *tmp = rt;
                while (tmp->ch[0] != markNode) tmp = tmp->ch[0];
                res.ptr = tmp;
            }
            else res.ptr = markNode;
            return res;
        }
        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end()
        {
            return iterator(markNode, this);
        }
        const_iterator cend() const
        {
            return iterator(markNode, this);
        }
        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty() const {return sz == 0;}
        /**
         * returns the number of elements.
         */
        size_t size() const
        {
            return sz;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            clear(rt);
            rt = markNode;
            sz = 0;
        }
        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
        void insert_fix(node *cur)
        {
            while(cur->fa->colour)
            {
                node *f = cur->fa;
                node *gf = f->fa;
                bool wh = !getPosition(f);
                node *ul = gf->ch[wh];
                if (ul->colour)//不用再写诸如if(ul && ul->colour)的东东了
                {
                    f->colour = ul->colour = 0;
                    gf->colour = 1;
                    cur = gf;
                }
                else
                {
                    if (cur == f->ch[wh]) rotate(cur = f, !wh);
                    else
                    {
                        gf->colour = 1, f->colour = 0;
                        rotate(gf, wh);
                    }
                }
            }
            rt->colour = 0;
        }
        pair<iterator, bool> insert(const value_type &value)
        {
            node *cur = rt;
            node *his = markNode;
            Key v = value.first;

            bool flag = true;
            //找结点位置
            while(cur != markNode)
            {
                his = cur;
                if (!(compare(v, cur->data->first) || compare(cur->data->first, v)))
                {
                    flag = false;
                    break;
                }
                cur = cur->ch[compare(cur->data->first, v)];
            }
            //没有发生碰撞
            if (flag)
            {
                cur = new node(value, markNode);//the new node is RED
                if (his != markNode) his->ch[compare(his->data->first, v)] = cur;
                else rt = cur;
                cur->fa = his;
                //更新大小
                ++sz;
                insert_fix(cur);
            }
            return pair<iterator, bool> (iterator(cur, this), flag);
        }
        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
        void erase_fix(node *ptr)
        {
            while(ptr != rt && !ptr->colour)
            {
                bool wh = !getPosition(ptr);
                node *f = ptr->fa;
                node *bro = f->ch[wh];
                if (bro->colour)//bro的两个儿子均为黑色（nullptr也是黑色）,父节点为黑色
                {
                    bro->colour = 0;
                    f->colour = 1;
                    rotate(f, !wh);
                }
                else
                {
                    if (!bro->ch[0]->colour && !bro->ch[1]->colour)//bro的两个儿子均为黑色,bro为黑色
                    {
                        bro->colour = 1;
                        if (f->colour)
                        {
                            f->colour = 0;
                            break;
                        }
                        else ptr = f;
                    }
                    else//bro为黑节点，且至少有一个红儿子
                    {
                        if (!bro->ch[wh]->colour)
                        {
                            bro->ch[!wh]->colour = 0;
                            bro->colour = 1;
                            rotate(bro, wh);
                            bro = f->ch[wh];
                        }
                        //确保bro的wh儿子是红儿子
                        bro->colour = f->colour;
                        bro->ch[wh]->colour = f->colour = 0;
                        rotate(f, !wh);
                        break;
                    }
                }
            }
            ptr->colour = 0;
        }
        void erase(iterator pos)
        {
            if (pos.belong != this || pos.ptr == markNode) throw invalid_iterator();
            node *org = pos.ptr;//上一条if可以保证此处的org一定不是nullptr
            node *cur = pos.ptr;
            node *son;
            if (org->ch[0] != markNode && org->ch[1] != markNode)
            {
                cur = org->ch[1];
                while(cur->ch[0] != markNode) cur = cur->ch[0];//此时cur为org的后缀节点
            }
            if (org != cur) swap_node(org, cur);


            //接下来删除cur节点即可
            if (cur->ch[0] == markNode) son = cur->ch[1];
            else son = cur->ch[0];


            son->fa = cur->fa;//就算son此时是markNode,也可以记录下其“父节点“，这使得处理方式大大简化，体现出markNode体系的优越性
            if (cur->fa == markNode) rt = son;
            else cur->fa->ch[getPosition(cur)] = son;
            //然后是维护size
            --sz;
            //删除后的维护
//            if (son != markNode) son->colour = 0;
//            if (son == markNode && !cur->colour && cur->fa != markNode) erase_fix(cur->fa);
            if (!cur->colour) erase_fix(son);
            delete cur;
        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key &key) const
        {
            node *cur = rt;
            node *his;
            while(cur != markNode)
            {
                his = cur;
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return 1;
            }
            return 0;
        }
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find(const Key &key)
        {
            node *cur = rt;
            while(cur != markNode)
            {
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return iterator(cur, this);
            }
            return end();
        }
        const_iterator find(const Key &key) const
        {
            node *cur = rt;
            while(cur != markNode)
            {
                if (compare(cur->data->first, key)) cur = cur->ch[1];
                else if (compare(key, cur->data->first)) cur = cur->ch[0];
                else return const_iterator(cur, this);
            }
            return cend();
        }
    };
}
