#ifndef __HPID__HPP__
#define __HPID__HPP__

#include <vector>
#include <set>
#include <parallel/algorithm>

class mpid{
  public:
    double comm;
    std::set<std::uint32_t> sets;
    
    mpid() {}
    ~mpid()
    {    
    }
    void set_comm(vid_t pid){
       auto it = sets.find(pid);
    }
};


#endif