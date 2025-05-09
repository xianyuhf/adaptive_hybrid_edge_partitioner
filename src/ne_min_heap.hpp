#pragma once

#include <vector>

#include "util.hpp"

template<typename ValueType, typename KeyType, typename IdxType = vid_t>
class NeMinHeap {
private:
    IdxType n;
    std::vector<std::pair<ValueType, KeyType>> heap;
    std::vector<IdxType> key2idx;

public:
    NeMinHeap() : n(0), heap(), key2idx() { }

    IdxType shift_up(IdxType cur) {
        if (cur == 0) return 0;
        IdxType p = (cur-1) / 2;//正好是回到他的上一个父节点，这里存的下标应该是一层又一层按顺序来的

        if (heap[cur].first < heap[p].first) {
            std::swap(heap[cur], heap[p]);
            std::swap(key2idx[heap[cur].second], key2idx[heap[p].second]);
            return shift_up(p);
        }
        return cur;
    }

    void shift_down(IdxType cur) {
        IdxType l = cur*2 + 1;
        IdxType r = cur*2 + 2;

        if (l >= n)
            return;

        IdxType m = cur;
        if (heap[l].first < heap[cur].first)
            m = l;
        if (r < n && heap[r].first < heap[m].first)
            m = r;

        if (m != cur) {
            std::swap(heap[cur], heap[m]);
            std::swap(key2idx[heap[cur].second], key2idx[heap[m].second]);
            shift_down(m);
        }
    }

    void insert(ValueType value, KeyType key) {
        heap[n] = std::make_pair(value, key);
        key2idx[key] = n++;
        IdxType cur = shift_up(n-1);
        shift_down(cur);
    }

    bool contains(KeyType key) {
        return key2idx[key] < n && heap[key2idx[key]].second == key;
    }


    void update_key(KeyType key, ValueType d = 1) {
        if (areEqual(d,0)) {
            // std::cout<<"occupy3"<<std::endl;
            return;
        }
        IdxType cur = key2idx[key];
        CHECK(cur < n ) << "key not found";//xyhf:这里满足条件就什么也不会说
        CHECK( heap[cur].second == key)<<"key not found ";
        heap[cur].first = d;
        cur= shift_up(cur);
        shift_down(cur);
    }

    void decrease_key(KeyType key, ValueType d = 1) {
        // if (areEqual(d,0)) {
        //     // std::cout<<"occupy3"<<std::endl;
        //     return;
        // }
        IdxType cur = key2idx[key];
        CHECK(cur < n ) << "key not found";//xyhf:这里满足条件就什么也不会说
        CHECK( heap[cur].second == key)<<"key not found ";
        // CHECK_GE(heap[cur].first, d) << "value cannot be negative";
        if(cur < n && heap[cur].second == key){
            return;
        }
        // heap[cur].first -= d;
        // if(heap[cur].first>d || areEqual(heap[cur].first,d)){  //0.4本来是小的，该选他的，结果变成减1，变成-0.6
        heap[cur].first -= d;
        // }
        // shift_up(cur);
        cur= shift_up(cur);
        shift_down(cur);
    }

    void increase_key(KeyType key, ValueType d = 1) {
        if (d == 0) return;
        IdxType cur = key2idx[key];
        CHECK(cur < n ) << "key not found";//xyhf:这里满足条件就什么也不会说
        CHECK( heap[cur].second == key)<<"key1 not found ";
        // CHECK_GE(heap[cur].first, d) << "value cannot be negative";
        // if(cur < n && heap[cur].second == key){
        //     return;
        // }
        // heap[cur].first -= d;
        // shift_up(cur);
        // if(heap[cur].first>d || areEqual(heap[cur].first,d)){  //0.4本来是小的，该选他的，结果变成减1，变成-0.6
        heap[cur].first += d;
        cur= shift_up(cur);
        shift_down(cur);
    }

    void  comp_decrease_key(KeyType key, ValueType comp) {
            
            // if (d == 0) return;
            IdxType cur = key2idx[key];
            //这里是为什么呢，我是有点懵的 ，感觉不会出现这个问题呀
            CHECK(cur < n ) << "key not found";//xyhf:这里满足条件就什么也不会说
            CHECK( heap[cur].second == key)<<"key1 not found ";
            // if(heap[cur].first>comp || areEqual(heap[cur].first,comp)){  //0.4本来是小的，该选他的，结果变成减1，变成-0.6
                heap[cur].first = comp;
                cur=shift_up(cur);
                shift_down(cur);
            // }
        }
        // void  comp_decrease_key(KeyType key, ValueType comp,ValueType d = 1) {
            
        //     if (d == 0) return;
        //     IdxType cur = key2idx[key];
        //     //这里是为什么呢，我是有点懵的 ，感觉不会出现这个问题呀
        //     CHECK(cur < n && heap[cur].second == key) << "key not found";//xyhf:这里满足条件就什么也不会说
        //     // if(comp==0){
        //     //     heap[cur].first=0;
        //     //     shift_up(cur);
        //     // }else{
            
        //     if(heap[cur].first>d || areEqual(heap[cur].first,d)){  //0.4本来是小的，该选他的，结果变成减1，变成-0.6
        //         heap[cur].first -= d;
        //         shift_up(cur);
        //     }
            
        //     // }

        // }
    bool remove(KeyType key) {
        IdxType cur = key2idx[key];
        if (cur >= n || heap[cur].second != key)
            return false;

        n--;
        if (n > 0) {
            heap[cur] = heap[n];
            key2idx[heap[cur].second] = cur;
            cur = shift_up(cur);
            shift_down(cur);
        }
        return true;
    }

    bool get_min(ValueType& value, KeyType& key) {
        if (n > 0) {
            value = heap[0].first;
            key = heap[0].second;
            return true;
        } else
            return false;
    }

    void reserve(IdxType nelements) {
        n = 0;
        heap.resize(nelements);
        key2idx.resize(nelements);
    }

    void clear() {
        n = 0;
    }
};
