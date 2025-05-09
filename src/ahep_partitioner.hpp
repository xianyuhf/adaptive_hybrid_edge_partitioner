#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "util.hpp"
#include "ne_min_heap.hpp"
#include "dense_bitset.hpp"
#include "edgepart.hpp"
#include "partitioner.hpp"
#include "ne_graph.hpp"
#include "pid.hpp"
#include "cluster.hpp"


/* Neighbor Expansion (NE) */
class AHEPartitioner : public Partitioner
{
  private:
    const double BALANCE_RATIO = 1.00;

    std::string basefilename;

    vid_t num_vertices;
    size_t num_edges, assigned_edges;
    int p, bucket;
    double average_degree;
    size_t capacity1;
    std::vector<double> capacities;

    std::vector<edge_t> edges;
    std::vector<edge_t> edges_temp;
    ne_graph_t adj_out, adj_in;
    ne_graph_t adj_out_temp, adj_in_temp;
    NeMinHeap<double, vid_t> min_heap;
    std::vector<size_t> occupied;
    std::vector<vid_t> degrees;
    std::vector<int8_t> master;
    std::vector<dense_bitset> is_cores, is_boundarys;

    double  min_size = 0; // currently smallest partition
    double  max_size = 0; // currently largest partition
    double lambda;

    bool write_out_partitions = false; // whether the partitions should be written to the out-file or not
    bool hybrid_partitioning = false; // whether to apply HEP-style hybrid partitioning using NE
    bool hybrid_random_stream = false;  

    std::random_device rd;  
    std::mt19937 gen;  
    std::uniform_int_distribution<vid_t> dis;
    edgepart_writer<vid_t, uint16_t> writer;

    int check_edge(const edge_t *e)
    {
        rep (i, bucket) {
            auto &is_boundary = is_boundarys[i];
            if (is_boundary.get(e->first) && is_boundary.get(e->second) &&
                occupied[i] < capacities[i]) {
                return i;
            }
        }

        rep (i, bucket) {
            auto &is_core = is_cores[i], &is_boundary = is_boundarys[i];
            if ((is_core.get(e->first) || is_core.get(e->second)) &&
                occupied[i] < capacities[i]) {
                if (is_core.get(e->first) && degrees[e->second] > average_degree)
                    continue;
                if (is_core.get(e->second) && degrees[e->first] > average_degree)
                    continue;
                is_boundary.set_bit(e->first);
                is_boundary.set_bit(e->second);
                return i;
            }
        }

        return p;
    }

    void assign_edge(int bucket, vid_t from, vid_t to)
    {
    	if (write_out_partitions){
    		writer.write_edge_assignment(from, to, bucket);
    	}
        assigned_edges++;
        occupied[bucket]++;
        degrees[from]--;
        degrees[to]--;
    }

    TSetManager tset_temp;
    TSetManager comm_temp;
    tset comm_set;
    dense_bitset mark;
    void update_min_heap_new(vid_t vid,bool bounds=false){
        double temp=0.0;
        double value=0.0;
        auto &is_core = is_cores[bucket];
        if(!is_core.get(vid))
        {
            std::vector<size_t> pid;
            rep (direction, 2) {
                ne_adjlist_t &neighbors = direction ? adj_out[vid] : adj_in[vid];
                for (size_t i = 0; i < neighbors.size();i++) {
                    if (edges[neighbors[i].v].valid()) {
                        vid_t &u = direction ? edges[neighbors[i].v].second : edges[neighbors[i].v].first;
                            double value_temp=0.0;
                            temp=update_tset(u);
                            tset_temp.insertTSet(vid,u,temp);
                            tset_temp.getValue(vid,u,value_temp);
                            value+=value_temp;                  
                    }
                }
            }

            value+=(((double)1/(double)(degree_temp[vid])));
            min_heap.insert(value,vid);
        }
    }
    
    
    double update_tset(vid_t vid){
        double value=0.0;
        auto &is_core = is_cores[bucket], &is_boundary = is_boundarys[bucket];
        double min_comm_current=min_comm[bucket];
        double value_comm_a=0.0;    
        bool have_edge=false;
        rep (direction_u, 2) {
            ne_adjlist_t &neighbors_u = direction_u ? adj_out[vid] : adj_in[vid];
            for (size_t j = 0; j < neighbors_u.size();j++) {
                if (edges[neighbors_u[j].v].valid()) {
                    vid_t &u_m = direction_u ? edges[neighbors_u[j].v].second : edges[neighbors_u[j].v].first;
                    if(!is_boundary.get(u_m)||!is_core.get(u_m)){
                        have_edge=true;
                        break;
                    }
                }
            }
            if(have_edge) break;
        }

        if(!comm_temp.find(vid)){
            comm_temp.insertTSet(vid,bucket,0.0);
            
        }
        if(bucket!=0){            
            if(!comm_temp.getValue(vid,bucket,value_comm_a)){
                comm_temp.undate_value(vid,bucket);
                comm_temp.getValue(vid,bucket,value_comm_a);
            }
        }
        if(have_edge){                    
            if(!areEqual(value_comm_a,0.0)){
                value+=1.0+value_comm_a+(((double)(p-1)/(double)p)*min_comm_current);
            }else{
                value+=((double)(p-1)/(double)p)+(((double)(p-1)/(double)p)*min_comm_current);
            }
        }else{
            if(!areEqual(value_comm_a,0.0)){
                value+=1.0+(value_comm_a);
            }
        }
        return value;
    }
    void add_boundary(vid_t vid)
    {
        auto &is_core = is_cores[bucket], &is_boundary = is_boundarys[bucket];
        if (is_boundary.get(vid))
            return;
        is_boundary.set_bit_unsync(vid);
        rep (direction, 2) {
            ne_adjlist_t &neighbors = direction ? adj_out[vid] : adj_in[vid];
            for (size_t i = 0; i < neighbors.size();) {
                if (edges[neighbors[i].v].valid()) {
                    vid_t &u = direction ? edges[neighbors[i].v].second : edges[neighbors[i].v].first;
                    if (is_core.get(u)) {
                        assign_edge(bucket, direction ? vid : u,
                                    direction ? u : vid);
                        degree_temp[vid]++;
                        edges[neighbors[i].v].remove();
                        std::swap(neighbors[i], neighbors.back());
                        neighbors.pop_back();
                    } else if (is_boundary.get(u) &&
                               occupied[bucket] < capacities[bucket]) {
                        assign_edge(bucket, direction ? vid : u,
                                    direction ? u : vid);
                        if(!mark.get(u)){
                            double value_temp=0.0;
                            tset_temp.getValue(u,vid,value_temp);
                            min_heap.decrease_key(u,value_temp);
                        }
                        degree_temp[vid]++;
                        degree_temp[u]++;
                        edges[neighbors[i].v].remove();
                        std::swap(neighbors[i], neighbors.back());
                        neighbors.pop_back();
                    } else
                        i++;
                } else {
                    std::swap(neighbors[i], neighbors.back());
                    neighbors.pop_back();
                }
            }
        }
    }


    void occupy_vertex(vid_t vid, vid_t d)
    {
        CHECK(!is_cores[bucket].get(vid)) << "add " << vid << " to core again";
        is_cores[bucket].set_bit_unsync(vid);
        comm_temp.insertTSet(vid,bucket,0.0);
        comm_temp.undate_value(vid,bucket);
        if (d == 0)
            return;
        
        
        add_boundary(vid);

        for (auto &i : adj_out[vid]){
            if (edges[i.v].valid()){
                mpids.insert(edges[i.v].second);
                mark.set_bit_unsync(edges[i.v].second) ;  
                comm_temp.insertTSet(edges[i.v].second,bucket,0.0); 
                comm_temp.undate_value(vid,bucket);            
                add_boundary(edges[i.v].second);
            }
        }
        

        for (auto &i : adj_in[vid]){
            if (edges[i.v].valid()){
                mpids.insert(edges[i.v].first);
                mark.set_bit_unsync(edges[i.v].first) ; 
                comm_temp.insertTSet(edges[i.v].first,bucket,0.0); 
                comm_temp.undate_value(vid,bucket);            
                add_boundary(edges[i.v].first);
            }
        }
        adj_out[vid].clear();
        adj_in[vid].clear();
        for(auto e: mpids){
             update_min_heap_new(e);  
        }
       
        mark.clear();
        mpids.clear();   
        
    }

    bool get_free_vertex(vid_t &vid)
    {
        vid = dis(gen);
        vid_t count = 0;
        while (count < num_vertices &&
               (adj_out[vid].size() + adj_in[vid].size() == 0 ||
                adj_out[vid].size() + adj_in[vid].size() >
                    2 * average_degree ||
                is_cores[bucket].get(vid))) {
            vid = (vid + ++count) % num_vertices;
        }
        if (count == num_vertices)
            return false;
        return true;
    }

    void assign_remaining();
    double compute_partition_score(vid_t u, vid_t v, int bucket_id); 
    int best_scored_partition(vid_t u, vid_t v); // returns bucket id where score is best for edge (u,v)

    double compute_partition_score_tiebreaking_balance(vid_t u, vid_t v, int bucket_id);  
    int best_scored_partition_tiebreaking_balance(vid_t u, vid_t v); // returns bucket id where score is best for edge (u,v), using the tiebreaking adaptation of HCSG scoring

    size_t count_mirrors();

    size_t random_streaming(); // returns number of streamed edges from input h2h file
    size_t hcsg_streaming(); // returns number of streamed edges from input h2h file

  public:
    AHEPartitioner(std::string basefilename);
    ~AHEPartitioner(){
        if (neighbors){
            free(neighbors);
        }
    }
    void split();
    void set_lf();
    void set_load();
    std::vector<size_t> degree_temp;
public:
    void load_in_memory(std::string basefilename, std::ifstream &fin);
    size_t stream_build(std::ifstream &fin, size_t num_edges, dense_bitset &is_high_degree, dense_bitset &has_high_degree_neighbor, std::vector<size_t> &count, bool write_low_degree_edgelist);
    double high_degree_factor;
    bool write_low_degree_edgelist = false; // whether edges incident to a low-degree vertex should be written out to a file. useful if this sub-graph should be analyzed separately.
    std::fstream h2h_file; // file that keeps edges between two high-degree vertices on external memory
    std::fstream low_degree_file; // file that keeps edges incident to a low-degree vertex on external memory
    size_t num_h2h_edges;
    dense_bitset is_high_degree;
    dense_bitset has_high_degree_neighbor;
    std::vector<size_t> count; // degrees of vertices//(num_vertices, 0);
    vid_t *neighbors;
    size_t nedges;
    vid_t high_degree_threshold; 
    std::vector<ne_adjlist_t> vdata;
    std::vector<double> bs;
    dense_bitset t;
    tset tsets;
    std::set<std::uint32_t>  mpids;
};
