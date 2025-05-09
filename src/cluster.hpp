#pragma once

#include "util.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <string>

extern std::vector<double> comps;
extern std::vector<std::vector<double>> comms;
extern double  arverage_comm;
extern std::vector<int> min_comm;
extern sset comms_set;
// extern char *comm_condition;
//实验版本
void set_ability();
void set_ability_haep();
void set_delta(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities);
void set_delta_haep(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities);
void experiments(vid_t num_edges,vid_t num_vertices);
void calculateMinComm();




class tset {
public:
    // 插入或更新元素
    void insert(vid_t id, double value) {
        data[id] = value;
    }

    void insert_comm_id(int pid){
        comm_id+=std::to_string(pid);
    }

    bool update(vid_t id,double value){
        auto it = data.find(id);
        if (it != data.end()) {
            
            return true;
        }
        return false; // 没有找到
    }
    bool updates_comm(vid_t vid){
        double value;
        if(!comms_set.get(comm_id,data[vid])){
            for(int i=0;i<comm_id.size();i++){
                value+=comms[comm_id[comm_id.size()-1]-'0'][comm_id[i]-'0'];
            }
            comms_set.insert(comm_id,value);
            data[vid]=value;
            // return value;
        }
        
        // for(auto it=data.begin();it!=data.end();it++){
        //     data[vid]+=comms[vid][it->first];
        // }
        return true;
    }
    bool updates_hcsg(vid_t vid){//这里的id是分区号
        for(auto it=data.begin();it!=data.end();it++){
        //    update(it->first,comms[it->first][vid]);
        //    update(vid,); 
           data[vid]+=comms[vid][it->first];
           data[it->first]+=comms[it->first][vid];
        }
    }
    // 根据 ID 获取值
    bool get(vid_t id, double &value) {
        auto it = data.find(id);
        if (it != data.end()) {
            value = it->second; // 找到元素，返回值
            return true;
        }
        return false; // 没有找到
    }

    // 根据 ID 删除元素
    bool remove(vid_t id) {
        auto it = data.find(id);
        if (it != data.end()) {
            data.erase(it); // 删除元素
            return true; // 删除成功
        }
        return false; // 没有找到元素，删除失败
    }

    // 删除所有元素
    void clear() {
        data.clear(); // 清空所有元素
    }

    // 可选：检查集合是否为空
    bool isEmpty() const {
        return data.empty();
    }
    bool find(vid_t vid){
        auto it = data.find(vid);
        if(it!=data.end()){
            return true;
        }else{
            return false;
        }
    }
public:
    std::unordered_map<int, double> data; // id -> value 映射
    std::string comm_id;
};

class TSetManager {//这里存的是所有顶点，到vid的时候才是存某个顶点的value
public:
    // 添加或更新 tset
    void insertTSet(int id, vid_t vid, double value) {
        tset &ts = tset_temp[id]; // 获取或创建 tset
        ts.insert(vid, value); // 在 tset 中插入或更新值
        ts.insert_comm_id(vid);
    }

    // 从 tset 获取值
    double getValue(int id, vid_t vid, double &value) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
             it->second.get(vid, value); // 从 tset 中获取值
             return true;
        }
        return false; // 没有找到对应的 tset
    }

    // 从 tset 删除元素
    bool removeValue(int id, vid_t vid) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            return it->second.remove(vid); // 从 tset 中删除元素
        }
        return false; // 没有找到对应的 tset
    }

    // 清空指定 tset
    void clearTSet(int id) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            it->second.clear(); // 清空 tset
        }
    }

    // 清空所有 tset
    void clearAll() {
        tset_temp.clear(); // 清空所有 tset
    }

    // 检查 tset 是否为空
    bool isTSetEmpty(int id) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            return it->second.isEmpty(); // 检查 tset 是否为空
        }
        return true; // 没有找到对应的 tset
    }


    bool undate_value(int id,int vid){ //这个vid 是哪个分区
        auto it = tset_temp.find(id);//首先看有没有这个顶点
        if (it != tset_temp.end()) {//
             it->second.updates_comm(vid); // 找到元素，然后遍历值，更新这个顶点的每个通信链路
             return true;
        }
        return false; // 没有找到
       
    }

    bool undate_value_hcsg(int id,int vid){ //这个vid 是哪个分区

        auto it = tset_temp.find(id);//首先看有没有这个顶点
        if (it != tset_temp.end()) {//
            it->second.updates_hcsg(vid); // 找到元素，然后遍历值，更新这个顶点的每个通信链路
            return true;
        }
        return false; // 没有找到
       
    }
    bool find(vid_t vid){
        auto it = tset_temp.find(vid);
        if(it!=tset_temp.end()){
            return true;
        }else{
            return false;
        }
    }
public:
    std::unordered_map<int, tset> tset_temp; // id -> tset 映射
};



