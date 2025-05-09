#include "cluster.hpp"
std::vector<double> comps;
std::vector<std::vector<double>> comms;
double arverage_comm;
std::vector<int> min_comm;
sset comms_set;



void set_ability(){
    double comp[2]={0.17,0.30}; 
    
    double comm[3]={0.0000045,0.000121,0.00000844}; 
     

    int p=FLAGS_p;
    comps.resize(p);
    if(FLAGS_topo==1||FLAGS_topo==3){  //comps同
        double sum1 = comp[0]*p;
        double normal_comp=(comp[0]/sum1);//所有的都这样，计算能力相同  ，这是归一化计算能力，接下来归一化通信能力
        for(int i=0;i<p;i++){
            comps[i]=normal_comp;
        }
    }
    if(FLAGS_topo==2||FLAGS_topo==4){  //comps不同
        double sum1 = comp[0]*(p-2)+comp[1]*2;
        double normal_comp[2];
        normal_comp[0]=(comp[0]/sum1);//归一化通信能力
        normal_comp[1]=(comp[1]/sum1);
        int i=0;
        for(;i<p;i++){
            comps[i]=normal_comp[0];
        }

        comps[1]=normal_comp[1];
        comps[3]=normal_comp[1];
    }
    
    if(FLAGS_topo==11||FLAGS_topo==33){  
        double sum1 = comp[0]*p;
        double normal_comp=(comp[0]/sum1);
        for(int i=0;i<p;i++){
            comps[i]=normal_comp;
        }
    }
    if(FLAGS_topo==22||FLAGS_topo==44){  
        double sum1 = comp[0]*(p/2)+comp[1]*(p/2);
        double normal_comp[2];
        normal_comp[0]=(comp[0]/sum1);
        normal_comp[1]=(comp[1]/sum1);
        int i=0;
        for(;i<p/2;i++){
            comps[i]=normal_comp[0];
        }
        for(;i<p;i++){
            comps[i]=normal_comp[1];
        }
    }
    
    
    //set_comm
    for(int i=0;i<p;i++){
        comms.push_back(std::vector<double>());
        for(int j=0;j<p;j++){
            comms[i].push_back(0);
        }
    }
    if(FLAGS_topo==1||FLAGS_topo==2){
        double normal_comm=(double)((double)(1)/((double)(p*(p-1))));
        for(int i=0;i<p;i++){
            for(int j=0;j<p;j++){
                comms[i][j]=normal_comm;
            }
        }
        for(int i=0;i<p;i++){
            comms[i][i]=0.0;
        }
    } //
    if(FLAGS_topo==3||FLAGS_topo==4){
        double normal_comm_sum=double(((comm[0]*(p-3)+comm[1]*2)*(p-2))+comm[1]*(p-1)*2)-comm[1]*2+comm[2]*2;
        LOG(INFO)<<"average"<<normal_comm_sum;
        arverage_comm=normal_comm_sum/(p*(p-1));
        LOG(INFO)<<"arverage_comm:"<<arverage_comm;
        double arverage_normal_comm=normal_comm_sum/((p*(p-1))*normal_comm_sum);
        LOG(INFO)<<"arverage_normal_comm:"<<arverage_normal_comm;
        double normal_comm[3];
        normal_comm[0]=comm[0]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[0]"<<normal_comm[0];
        normal_comm[1]=comm[1]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[1]"<<normal_comm[1];
        normal_comm[2]=comm[2]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[2]"<<normal_comm[2];

        for(int i=0;i<p;i++){
            for(int j=0;j<p;j++){
                comms[i][j]=normal_comm[0];
            }
        }
        for(int j=0;j<p;j++){
                comms[1][j]=normal_comm[1]; comms[j][1]=normal_comm[1];
                comms[3][j]=normal_comm[1]; comms[j][3]=normal_comm[1];


        }
        comms[1][3]=normal_comm[2]; comms[3][1]=normal_comm[2];
       
        for(int i=0;i<p;i++){
            comms[i][i]=0.0;
        }
    }

    if(FLAGS_topo==11||FLAGS_topo==22){
        double normal_comm=(double)((double)(1)/((double)(p*(p-1))));
        for(int i=0;i<p;i++){
            for(int j=0;j<p;j++){
                comms[i][j]=normal_comm;
            }
        }
        for(int i=0;i<p;i++){
            comms[i][i]=0.0;
        }
    } 
    if(FLAGS_topo==33||FLAGS_topo==44){
        double normal_comm_sum=double((comm[0]*(p/2-1)+comm[2]*(p/2))*(p/2)+(comm[2]*(p-1))*(p/2));
        LOG(INFO)<<"average"<<normal_comm_sum;
        arverage_comm=normal_comm_sum/(p*(p-1));
        LOG(INFO)<<"arverage_comm:"<<arverage_comm;
        double arverage_normal_comm=normal_comm_sum/((p*(p-1))*normal_comm_sum);
        LOG(INFO)<<"arverage_normal_comm:"<<arverage_normal_comm;
        double normal_comm[3];
        normal_comm[0]=comm[0]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[0]"<<normal_comm[0];
        normal_comm[1]=comm[1]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[1]"<<normal_comm[1];
        normal_comm[2]=comm[2]/normal_comm_sum;  
        LOG(INFO)<<"normal_comm[2]"<<normal_comm[2];
 
        int i=0;
        for(int i=0;i<p;i++){
            for(int j=0;j<p;j++){
                comms[i][j]=normal_comm[0];
            }
        }
        for(int j=0;j<p;j++){
            if((j+1)%2==0){
                for(int k=0;k<p;k++){
                    comms[i][j]=normal_comm[2];
                    comms[j][i]=normal_comm[2];
                }
            }
            

        }
        comms[1][3]=normal_comm[2]; comms[3][1]=normal_comm[2];
       
        for(int i=0;i<p;i++){
            comms[i][i]=0.0;
        }
    }

}


void set_delta(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities){
    int p= FLAGS_p;

    double average_degree = (double)num_edges * 2 / num_vertices;
    double dense = (double)num_vertices/num_edges;
    double b_i_m=0.0;
    for(int i=0;i<p;i++){
        b_i_m=0.0;
        for(int j=0;j<p;j++){
            if(j!=i){
                b_i_m+=comms[i][j];
            }
        }
        bs[i]=(comps[i]*average_degree+b_i_m)*dense;
    }
    
    double sum=0.0;
    for(int i=0;i<p;i++){
        if(bs[i]!=0){
            sum+=1/bs[i];
        }else{
            sum+=0;
        }
    }
    double cabsum=0.0;
    for(int i=0;i<p;i++){
        if(bs[i]*sum!=0){
            capacities[i]=ceil(((double)num_edges)/(bs[i]*sum));
        }else{
            capacities[i]=0;
        }
        
        cabsum+=capacities[i];
        
    }

}



void set_ability_haep(){
    double comp[2]={0.17,0.30}; 
    
    int p=FLAGS_p;

    comps.resize(p);
    comp[0]=1/comp[0];comp[1]=1/comp[1];
    if(FLAGS_topo==1||FLAGS_topo==3){ 
        double sum1 = comp[0]*p;
        double normal_comp=(comp[0]/sum1);
        for(int i=0;i<p;i++){
            comps[i]=normal_comp;
        }
    }
    if(FLAGS_topo==2||FLAGS_topo==4){  

        double sum1 = comp[0]*(p-2)+comp[1]*2;
        double normal_comp[2];
        normal_comp[0]=(comp[0]/sum1);
        normal_comp[1]=(comp[1]/sum1);
        int i=0;
        for(;i<p;i++){
            comps[i]=normal_comp[0];
        }
        comps[1]=normal_comp[1];
        comps[3]=normal_comp[1];
    }


    if(FLAGS_topo==11||FLAGS_topo==33){ 
        double sum1 = comp[0]*p;
        double normal_comp=(comp[0]/sum1);
        for(int i=0;i<p;i++){
            comps[i]=normal_comp;
        }
    }
    if(FLAGS_topo==22||FLAGS_topo==44){  
        double sum1 = comp[0]*(p/2)+comp[1]*(p/2);
        double normal_comp[2];
        normal_comp[0]=(comp[0]/sum1);
        normal_comp[1]=(comp[1]/sum1);
        int i=0;
        for(;i<p/2;i++){
            comps[i]=normal_comp[0];
        }
        for(;i<p;i++){
            comps[i]=normal_comp[1];
        }
    }
}
void set_delta_haep(vid_t num_edges,vid_t num_vertices,std::vector<double>& bs,std::vector<double>& capacities){
    int p= FLAGS_p;
    

    for(int i=0;i<p;i++){
        bs[i]=(comps[i]);
    }
    
    double sum=0.0;
    for(int i=0;i<p;i++){
        sum+=bs[i];
     
    }
    double cabsum=0.0;
    for(int i=0;i<p-1;i++){
        capacities[i]=ceil(((double)num_edges)*(bs[i]));
        cabsum+=capacities[i];
    }
    capacities[p-1]=(double)num_edges-cabsum;
}



void calculateMinComm() {
    int p= FLAGS_p;
    min_comm.resize(p + 1, std::numeric_limits<int>::max());

    for (int bucket = 0; bucket < p; ++bucket) {
        for (int i = 0; i < p; ++i) {
            if (i != bucket) {
                if (comms[bucket][i] < min_comm[bucket]) {
                    min_comm[bucket] = comms[bucket][i];
                }
            }
        }
    }
}


