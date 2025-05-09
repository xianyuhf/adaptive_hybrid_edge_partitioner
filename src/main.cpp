#include <string>

#include "util.hpp"

#include "ahep_partitioner.hpp"


DECLARE_bool(help);
DECLARE_bool(helpshort);

DEFINE_int32(p, 10, "number of partitions");
DEFINE_string(filename, "", "the file name of the input graph");
DEFINE_string(filetype, "edgelist",
              "the type of input file (AHEP supports only 'edgelist')");
DEFINE_string(output, "",
              "the path of output");
DEFINE_string(cluster_input,"","the input path of heterogenous cluster");
DEFINE_string(cluster_output,"","the output path of heterogenous cluster ");
DEFINE_string(method, "ahep", "partition method: ahep.");
DEFINE_bool(write_results, false, "Should the result be written to the output file or not. Take care, writing is in ASCII format is is really really slow in the current implementation.");
DEFINE_bool(write_low_degree_edgelist, true, "Should the list of edges incident to a low-degree vertex be written out to a file?");
DEFINE_double(hdf, 100, "High-degree factor: hdf * average_degree = high-degree threshold (hdth). Called \tau in the paper. Vertices with than hdth neighbors are treated specially in fast NE");
DEFINE_double(lambda, 1.1, "Lambda value to weigh in balancing score in streaming partitioning via HCSG");
DEFINE_bool(extended_metrics, false, "Display extended metrics in the result");
DEFINE_bool(random_streaming, true, "Use random streaming instead of  HCSG in the second phase of AHEP.");
DEFINE_bool(hybrid_NE, true, "Perform hybrid partitioning in AHEP-style, but use NE instead of NE++ for the first phase.");
DEFINE_string(log_info,"","information in the diary");
DEFINE_string(log_dirs,"","location of the diary");
DEFINE_int32(topo,4,"topoplgy");

 
#define TOTAL
int main(int argc, char *argv[])
{
    std::string usage = "-filename <path to the input graph> "
                        "[-filetype <edgelist|adjlist>] "
                        "[-p <number of partitions>] ";
                        
    google::SetUsageMessage(usage);
    
    gflags::ParseCommandLineFlags(&argc, &argv, true);
  
    std::string log_dirs = FLAGS_log_dirs;
    std::string log_info = FLAGS_log_info;
    char dir1[100];
    char dir2[50];
    strcpy(dir1,log_dirs.c_str());
    strcpy(dir2,log_info.c_str());
    strcat(dir1,dir2);

    google::SetLogDestination(google::GLOG_INFO, dir1);
    FLAGS_minloglevel = google::GLOG_INFO; 

    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 0; // output log to stderr
    FLAGS_alsologtostderr = true;
    if (FLAGS_help) {
        FLAGS_help = false;
        FLAGS_helpshort = true;
    }
    google::HandleCommandLineHelpFlags();


    Partitioner *partitioner = NULL;

    if (FLAGS_method == "ahep")
        partitioner = new AHEPartitioner(FLAGS_filename);
    else
        LOG(ERROR) << "unkown method: " << FLAGS_method;
    LOG(INFO) << "partition method: " << FLAGS_method;
    partitioner->split();
    
    google::ShutdownGoogleLogging();
}

