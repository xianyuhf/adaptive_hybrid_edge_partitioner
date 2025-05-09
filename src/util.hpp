#pragma once

#include <utility>
#include <chrono>
#include <stdint.h>
#include <sys/stat.h>
#include <cmath>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <unordered_map>

// #include "threadpool11/threadpool11.hpp"
// extern threadpool11::Pool pool;
#define rep(i, n) for (int i = 0; i < (int)(n); ++i)
#define repv(i, n) for (vid_t i = 0; i < n; ++i)
#define rept(i, n) for(int i=1; i<=(int)(n); ++i)

DECLARE_int32(topo);
DECLARE_int32(p);
DECLARE_string(log_info);
DECLARE_double(hdf);  //declare不重要，这里declare相当于回调
DECLARE_string(filename);
DECLARE_string(output);
DECLARE_string(cluster_input);
DECLARE_string(cluster_output);
DECLARE_double(lambda);
DECLARE_bool(write_low_degree_edgelist);
DECLARE_bool(write_results);
DECLARE_bool(extended_metrics);
DECLARE_bool(random_streaming);
DECLARE_bool(hybrid_NE);
DECLARE_string(method);
typedef uint32_t vid_t;
const vid_t INVALID_VID = -1;

struct edge_t {
    vid_t first, second;
    edge_t() : first(0), second(0) {}
    edge_t(vid_t first, vid_t second) : first(first), second(second) {}
    const bool valid() { return first != INVALID_VID; }
    void remove() { first = INVALID_VID; }  //remove就是删除掉
};

void preada(int f, char *buf, size_t nbytes, size_t off);
void reada(int f, char *buf, size_t nbytes);
void writea(int f, char *buf, size_t nbytes);

const double EPSILON = 1e-10; // 设定一个小的误差范围
inline bool areEqual(double a, double b) {
    return fabs(a - b) < EPSILON; // 近似相等 返回1
}

inline std::string h2hedgelist_name(const std::string &basefilename)  //这里参数传递进来的也是绝对路径
{
	std::stringstream ss;
	ss << FLAGS_output << ".h2h_edgelist";        
	return ss.str();
}

inline std::string lowedgelist_name(const std::string &basefilename)
{
	std::stringstream ss;
	ss << FLAGS_output << ".low_edgelist";
	return ss.str();
}

inline std::string binedgelist_name(const std::string &basefilename)
{
    std::stringstream ss;
 
    ss << FLAGS_output << ".binedgelist";
  
    return ss.str();
}

inline std::string degree_name(const std::string &basefilename)
{
    std::stringstream ss; //stringstream 类是 C++ 标准库中的一个非常有用的类，它可以帮助我们在字符串和其他数据类型之间进行转换。
    //stringstream 不是用来io读取的 ，就是返回字符串
    ss << FLAGS_output << ".degree";
    return ss.str();
}

inline std::string partitioned_name(const std::string &basefilename)
{
    std::stringstream ss;
    // ss << FLAGS_output << "."<<FLAGS_method<<"." << FLAGS_p<<".csv";
    ss << FLAGS_output << FLAGS_log_info<<".csv";
    return ss.str();
}

inline bool is_exists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

class Timer
{
  private:
    std::chrono::system_clock::time_point t1, t2;
    double total;

  public:
    Timer() : total(0) {}
    void reset() { total = 0; }
    void start() { t1 = std::chrono::system_clock::now(); }
    void stop()
    {
        t2 = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = t2 - t1;
        total += diff.count();
    }
    double get_time() { return total; }
};

// std::vector<std::vector<double>> comms;
bool isLessThan(double a, double b);

bool isGreaterThan(double a, double b);

bool areAlmostEqual(double a, double b);


class sset {
public:
    // 插入或更新元素
    void insert(const std::string& id, double value) {
        data[id] = value; // 使用字符串作为键
    }

    // 更新元素（找到则返回 true）
    bool update(const std::string& id, double value) {
        auto it = data.find(id);
        if (it != data.end()) {
            it->second = value; // 更新值
            return true;
        }
        return false; // 没有找到
    }


    // 根据 ID 获取值
    bool get(const std::string& id, double &value) {
        auto it = data.find(id);
        if (it != data.end()) {
            value = it->second; // 找到元素，返回值
            return true;
        }
        return false; // 没有找到
    }

    // 根据 ID 删除元素
    bool remove(const std::string& id) {
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

    // 查找元素是否存在
    bool find(const std::string& vid) {
        auto it = data.find(vid);
        return it != data.end(); // 直接返回查找结果
    }

private:
    std::unordered_map<std::string, double> data; // 使用 std::string 作为 key
};