#include "ne_graph.hpp"

void ne_graph_t::build(const std::vector<edge_t> &edges)
{
    if (edges.size() > nedges){
        neighbors = (uint40_t *)realloc(neighbors, sizeof(uint40_t) * edges.size());
        CHECK(neighbors) << "allocation failed";
    }
    nedges = edges.size();

    std::vector<size_t> count(num_vertices, 0);
    for (size_t i = 0; i < nedges; i++)
        count[edges[i].first]++;

    vdata[0] = ne_adjlist_t(neighbors);
    for (vid_t  v = 1; v < num_vertices; v++) {
        count[v] += count[v-1];
        //neighbors是指针  ，count[v-1]是表示位移多少的字节
        vdata[v] = ne_adjlist_t(neighbors + count[v-1]);//这节空间起始地址之后的就是你的
    }
    for (size_t i = 0; i < edges.size(); i++)
        //这里vdata[]中的下标是边的出点，值是边的具体哪一行
        //edges[i].first 很明显有很多值是一样的，但是i是不一样的
        vdata[edges[i].first].push_back(i);//这里vdata[x] 是ne_adjlist_t,   这里的push_back是ne_adjlist_t的自定义
}
//首尾的判断，有向图的  这个程序里面是怎么对有向图处理的
void ne_graph_t::build_reverse(const std::vector<edge_t> &edges)
{
    if (edges.size() > nedges){
        neighbors = (uint40_t *)realloc(neighbors, sizeof(uint40_t) * edges.size());
        CHECK(neighbors) << "allocation failed";
    }
    nedges = edges.size();

    std::vector<size_t> count(num_vertices, 0);
    for (size_t i = 0; i < nedges; i++)
        count[edges[i].second]++;

    vdata[0] = ne_adjlist_t(neighbors);
    for (vid_t v = 1; v < num_vertices; v++) {
        count[v] += count[v-1];
        vdata[v] = ne_adjlist_t(neighbors + count[v-1]);
    }
    for (size_t i = 0; i < edges.size(); i++)
        vdata[edges[i].second].push_back(i);
}
