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

void set_ability();
void set_ability_haep();
void set_delta(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities);
void set_delta_haep(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities);
void experiments(vid_t num_edges,vid_t num_vertices);
void calculateMinComm();




class tset {
public:
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
        return false; 
    }
    bool updates_comm(vid_t vid){
        double value=0.0;
        if(!comms_set.get(comm_id,data[vid])){
            for(size_t i=0;i<comm_id.size();i++){
                value+=comms[comm_id[comm_id.size()-1]-'0'][comm_id[i]-'0'];
            }
            comms_set.insert(comm_id,value);
            data[vid]=value;
        }
        return true;
    }
    void updates_hcsg(vid_t vid){
        for(auto it=data.begin();it!=data.end();it++){
           data[vid]+=comms[vid][it->first];
           data[it->first]+=comms[it->first][vid];
        }
    }
    bool get(vid_t id, double &value) {
        auto it = data.find(id);
        if (it != data.end()) {
            value = it->second; 
            return true;
        }
        return false; 
    }

    bool remove(vid_t id) {
        auto it = data.find(id);
        if (it != data.end()) {
            data.erase(it); 
            return true; 
        }
        return false; 
    }

    void clear() {
        data.clear(); 
    }


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
    std::unordered_map<int, double> data; 
    std::string comm_id;
};

class TSetManager {
public:

    void insertTSet(int id, vid_t vid, double value) {
        tset &ts = tset_temp[id]; 
        ts.insert(vid, value);
        ts.insert_comm_id(vid);
    }

    bool getValue(int id, vid_t vid, double &value) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
             it->second.get(vid, value); 
             return true;
        }
        return false; 
    }

    bool removeValue(int id, vid_t vid) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            return it->second.remove(vid); 
        }
        return false; 
    }


    void clearTSet(int id) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            it->second.clear(); 
        }
    }

    
    void clearAll() {
        tset_temp.clear(); 
    }


    bool isTSetEmpty(int id) {
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
            return it->second.isEmpty(); 
        }
        return true; 
    }


    bool undate_value(int id,int vid){ 
        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {
             it->second.updates_comm(vid); 
        }
        return false; 
       
    }

    bool undate_value_hcsg(int id,int vid){ 

        auto it = tset_temp.find(id);
        if (it != tset_temp.end()) {//
            it->second.updates_hcsg(vid); 
            return true;
        }
        return false; 
       
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
    std::unordered_map<int, tset> tset_temp; 
};



