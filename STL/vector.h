#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.h"

#include <climits>
#include <cstddef>

namespace sjtu
{
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
    template<typename T>
    class vector
    {
    public:
        /**
         * TODO
         * a type for actions of the elements of a vector, and you should write
         *   a class named const_iterator with same interfaces.
         */
        class const_iterator;
        class iterator
        {
            // The following code is written for the C++ type_traits library.
            // Type traits is a C++ feature for describing certain properties of a type.
            // For instance, for an iterator, iterator::value_type is the type that the
            // iterator points to.
            // STL algorithms and containers may use these type_traits (e.g. the following
            // typedef) to work properly. In particular, without the following code,
            // @code{std::sort(iter, iter1);} would not compile.
            // See these websites for more information:
            // https://en.cppreference.com/w/cpp/header/type_traits
            // About value_type: https://blog.csdn.net/u014299153/article/details/72419713
            // About iterator_category: https://en.cppreference.com/w/cpp/iterator
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::output_iterator_tag;

        private:
            T* beg;
            int num;

        public:
            iterator() {}
            iterator(T* b, int n) : beg(b), num(n) {}
            iterator(const iterator &rhs) : beg(rhs.beg), num(rhs.num) {}
            const int &getNum() {return num;}
            /**
             * return a new iterator which pointer n-next elements
             * as well as operator-
             */
            iterator operator+(const int &n) const
            {
                return iterator(beg, num + n);
            }
            iterator operator-(const int &n) const
            {
                return iterator(beg, num - n);
            }
            // return the distance between two iterators,
            // if these two iterators point to different vectors, throw invaild_iterator.
            int operator-(const iterator &rhs) const
            {
                if (beg == rhs.beg) throw invalid_iterator();
                return num - rhs.num;
            }
            iterator& operator+=(const int &n)
            {
                num += n;
                return *this;
            }
            iterator& operator-=(const int &n)
            {
                num -= n;
                return *this;
            }
            /**
             * TODO iter++
             */
            iterator operator++(int)
            {
                iterator tmp = *this;
                ++num;
                return tmp;
            }
            /**
             * TODO ++iter
             */
            iterator& operator++()
            {
                ++num;
                return *this;
            }
            /**
             * TODO iter--
             */
            iterator operator--(int)
            {
                iterator tmp = *this;
                --num;
                return tmp;
            }
            /**
             * TODO --iter
             */
            iterator& operator--()
            {
                --num;
                return *this;
            }
            /**
             * TODO *it
             */
            T& operator*() const
            {
                return *(beg + num);
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return (beg == rhs.beg && num == rhs.num);
            }
            bool operator==(const const_iterator &rhs) const
            {
                return (beg == rhs.beg && num == rhs.num);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return !(beg == rhs.beg && num == rhs.num);
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return !(beg == rhs.beg && num == rhs.num);
            }
        };
        /**
         * TODO
         * has same function as iterator, just for a const object.
         */
        class const_iterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::output_iterator_tag;

        private:
            T* beg;
            int num;

        public:
            const_iterator() {}
            const_iterator(T* b, int n) : beg(b), num(n) {}
            const_iterator(const const_iterator &rhs) : beg(rhs.beg), num(rhs.num) {}
            const int &getNum() {return num;}

            const_iterator operator+(const int &n) const
            {
                return const_iterator(beg, num + n);
            }
            const_iterator operator-(const int &n) const
            {
                return const_iterator(beg, num - n);
            }
            int operator-(const const_iterator &rhs) const
            {
                if (beg == rhs.beg) throw invalid_iterator();
                return num - rhs.num;
            }
            const_iterator& operator+=(const int &n)
            {
                num += n;
                return *this;
            }
            const_iterator& operator-=(const int &n)
            {
                num -= n;
                return *this;
            }
            /**
             * TODO iter++
             */
            const_iterator operator++(int)
            {
                const_iterator tmp = *this;
                ++num;
                return tmp;
            }
            /**
             * TODO ++iter
             */
            const_iterator& operator++()
            {
                ++num;
                return *this;
            }
            /**
             * TODO iter--
             */
            const_iterator operator--(int)
            {
                const_iterator tmp = *this;
                --num;
                return tmp;
            }
            /**
             * TODO --iter
             */
            const_iterator& operator--()
            {
                --num;
                return *this;
            }
            /**
             * TODO *it
             */
            const T& operator*() const
            {
                return *(beg + num);
            }
            /**
             * a operator to check whether two iterators are same (pointing to the same memory address).
             */
            bool operator==(const iterator &rhs) const
            {
                return (beg == rhs.beg && num == rhs.num);
            }
            bool operator==(const const_iterator &rhs) const
            {
                return (beg == rhs.beg && num == rhs.num);
            }

            /**
             * some other operator for iterator.
             */
            bool operator!=(const iterator &rhs) const
            {
                return !(beg == rhs.beg && num == rhs.num);
            }
            bool operator!=(const const_iterator &rhs) const
            {
                return !(beg == rhs.beg && num == rhs.num);
            }
        };

    private:
        static const int initSize = 10;
        T* storage;
        size_t curSize;
        size_t maxSize;
        std::allocator<T> helper;
        void doubleSpace()
        {
            T *tmp = storage;
            maxSize *= 2;
            storage = helper.allocate(maxSize);
            for (int i = 0; i < curSize; ++i) helper.template construct(storage + i, tmp[i]);
            for (int i = 0; i < curSize; ++i) helper.template destroy(tmp + i);
            helper.deallocate(tmp, curSize);
        }

    public:
        /**
        * TODO Constructs
        * At least two: default constructor, copy constructor
        */
        vector()
        {
            storage = helper.allocate(initSize);
            curSize = 0;
            maxSize = initSize;
        }
        vector(const vector &other)
        {
            curSize = other.curSize;
            maxSize = other.maxSize;
            storage = helper.allocate(maxSize);
            for (int i = 0; i < curSize; ++i)
            {
                helper.template construct(storage + i, other.storage[i]);
            }
        }
        ~vector()
        {
            for (int i = 0; i < curSize; ++i)
            {
                helper.template destroy(storage + i);
            }
            helper.deallocate(storage, maxSize);
        }

        /**
         * TODO Assignment operator
         */
        vector &operator=(const vector &other)
        {
            if (this == &other) return *this;
            for (int i = 0; i < curSize; ++i)
            {
                helper.template destroy(storage + i);
            }
            helper.deallocate(storage, maxSize);
            curSize = other.curSize;
            maxSize = other.maxSize;
            storage = helper.allocate(maxSize);
            for (int i = 0; i < curSize; ++i)
            {
                helper.template construct(storage + i, other.storage[i]);
            }
            return *this;
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         */
        T & at(const size_t &pos)
        {
            if (pos < 0 || pos >= curSize) throw index_out_of_bound();
            return storage[pos];
        }
        const T & at(const size_t &pos) const
        {
            if (pos < 0 || pos >= curSize) throw index_out_of_bound();
            return storage[pos];
        }
        /**
         * assigns specified element with bounds checking
         * throw index_out_of_bound if pos is not in [0, size)
         * !!! Pay attentions
         *   In STL this operator does not check the boundary but I want you to do.
         */
        T & operator[](const size_t &pos)
        {
            if (pos < 0 || pos >= curSize) throw index_out_of_bound();
            return storage[pos];
        }
        const T & operator[](const size_t &pos) const
        {
            if (pos < 0 || pos >= curSize) throw index_out_of_bound();
            return storage[pos];
        }

        /**
         * access the first element.
         * throw container_is_empty if size == 0
         */
        const T & front() const
        {
            if (curSize == 0) throw container_is_empty();
            return storage[0];
        }
        /**
         * access the last element.
         * throw container_is_empty if size == 0
         */
        const T & back() const
        {
            if (curSize == 0) throw container_is_empty();
            return storage[curSize - 1];
        }
        /*
        iterator() {}
        iterator(T* b, int n) : beg(b), num(n) {}
        iterator(const iterator &rhs) : beg(rhs.beg), num(rhs.num) {}

        const_iterator() {}
        const_iterator(T* b, int n) : beg(b), num(n) {}
        const_iterator(const const_iterator &rhs) : beg(rhs.beg), num(rhs.num) {}
        */

        /**
         * returns an iterator to the beginning.
         */
        iterator begin()
        {
            return iterator(storage, 0);
        }
        const_iterator cbegin() const
        {
            return const_iterator(storage, 0);
        }
        /**
         * returns an iterator to the end.
         */
        iterator end()
        {
            return iterator(storage, curSize);
        }
        const_iterator cend() const
        {
            return const_iterator(storage, curSize);
        }
        /**
         * checks whether the container is empty
         */
        bool empty() const
        {
            return curSize == 0;
        }
        /**
         * returns the number of elements
         */
        size_t size() const
        {
            return curSize;
        }
        /**
         * clears the contents
         */
        void clear()
        {
            curSize = 0;
        }
        /**
         * inserts value before pos
         * returns an iterator pointing to the inserted value.
         */
        iterator insert(iterator pos, const T &value)
        {
            if (curSize == maxSize) doubleSpace();
            int theNum = pos.getNum();
            helper.template construct(storage + curSize, value);
            for (int i = curSize; i > theNum; --i)
            {
                storage[i] = storage[i - 1];
            }
            storage[theNum] = value;
            ++curSize;
            return pos;
        }
        /**
         * inserts value at index ind.
         * after inserting, this->at(ind) == value
         * returns an iterator pointing to the inserted value.
         * throw index_out_of_bound if ind > size (in this situation ind can be size because after inserting the size will increase 1.)
         */
        iterator insert(const size_t &ind, const T &value)
        {
            if (ind > curSize) throw index_out_of_bound();
            if (curSize == maxSize) doubleSpace();
            helper.template construct(storage + curSize, value);
            for (int i = curSize; i > ind; --i)
            {
                storage[i] = storage[i - 1];
            }
            storage[ind] = value;
            ++curSize;
            return iterator(storage, ind);
        }
        /**
         * removes the element at pos.
         * return an iterator pointing to the following element.
         * If the iterator pos refers the last element, the end() iterator is returned.
         */
        iterator erase(iterator pos)
        {
            bool flag = (pos.getNum() == curSize - 1);
            for (int i = pos.getNum(); i < curSize - 1; ++i)
            {
                storage[i] = storage[i + 1];
            }
            helper.template destroy(storage + curSize - 1);
            --curSize;
            if (flag) return iterator(storage, curSize - 1);
            return pos;
        }
        /**
         * removes the element with index ind.
         * return an iterator pointing to the following element.
         * throw index_out_of_bound if ind >= size
         */
        iterator erase(const size_t &ind)
        {
            if (ind >= curSize) throw index_out_of_bound();
            for (int i = ind; i < curSize - 1; ++i)
            {
                storage[i] = storage[i + 1];
            }
            helper.template destroy(storage + curSize - 1);
            --curSize;
            return iterator(storage, ind - (ind == curSize));
        }
        /**
         * adds an element to the end.
         */
        void push_back(const T &value)
        {
            if (curSize == maxSize) doubleSpace();
            helper.template construct(storage + curSize, value);
            ++curSize;
        }
        /**
         * remove the last element from the end.
         * throw container_is_empty if size() == 0
         */
        void pop_back()
        {
            if (curSize == 0) throw container_is_empty();
            helper.template destroy(storage + curSize - 1);
            --curSize;
        }
    };
}

#endif
