#include "ahep_partitioner.hpp"
#include "conversions.hpp"


void AHEPartitioner::set_lf(){
    double lf=0.0;
    double sizes=0.0;
    int temp=0;
    int mirrors=0;
    for(int i=0;i<num_vertices;i++){
        for(int j=0;j<p;j++){
            if(is_boundarys[j].get(i)){
                for(int k=0;k<p;k++){
                    if(is_boundarys[k].get(i)){
                        lf+=comms[j][k];
                        if(comms[j][k]!=0){
                            sizes+=1;
                            temp++;
                        }
                    }
                }
            }
        }
        if(temp!=0){
            mirrors++;
        }
    }
    
    LOG(INFO)<<"lf_"<<FLAGS_topo<<":"<<lf/sizes;
}
void AHEPartitioner::set_load(){
    double max_c=0.0;
    double sum_c=0.0;
    double max_bs=0.0;
    double sum_bs=0.0;
    double c[p]={0.0};
    double temp=0.0;
    double average_degree = (double)(num_edges * 2) / double (num_vertices);
    int bs_m=0;
    int c_m=0;
    for(int j=0;j<p;j++){
        for(int i=0;i<num_vertices;i++){
            if(is_boundarys[j].get(i)){
                temp+=comps[j]*average_degree;
                for(int k=0;k<p;k++){
                    if(is_boundarys[k].get(i)){
                        temp+=comms[j][k];
                    }
                }
            }
        }
        sum_c+=temp;
        sum_bs+=bs[j]*capacities[j];
        
        if(isGreaterThan(temp,max_c)){
            max_c=temp;
            c_m=j;
        }
        if(isGreaterThan(bs[j]*capacities[j],max_bs)){
            max_bs=bs[j]*capacities[j];
            bs_m=j;
        }
        temp=0.0;
    }
    LOG(INFO)<<"balance_:"<<max_c/(sum_c/p);
}

void AHEPartitioner::load_in_memory(std::string basefilename, std::ifstream &fin){
	
    high_degree_factor = FLAGS_hdf;
	h2h_file.open(h2hedgelist_name(basefilename), std::ios_base::binary | std::ios_base::out ); // *.h2h_edgelist file
	write_out_partitions = FLAGS_write_results; // writing out partitions to file
    write_low_degree_edgelist = FLAGS_write_low_degree_edgelist; // writing low degree edgelist to file
    
    if (write_low_degree_edgelist){
		low_degree_file.open(lowedgelist_name(basefilename), std::ios_base::binary | std::ios_base::out ); // *.low_edgelist file;
	}

	num_h2h_edges = stream_build(fin, num_edges, is_high_degree, has_high_degree_neighbor, count, write_low_degree_edgelist);
	h2h_file.close(); //flushed
	if (write_low_degree_edgelist){
		low_degree_file.close(); //flushed
	}
}

AHEPartitioner::AHEPartitioner(std::string basefilename)
    : basefilename(basefilename), rd(), gen(rd()), writer(basefilename)
{
    // std::cout<<"zhanxian"<<std::endl;
    Timer convert_timer;
    
    convert_timer.start();//计时
    Converter *converter = new Converter(basefilename);
    convert(basefilename, converter);
    delete converter;
    convert_timer.stop();
    LOG(INFO) << "convert time: " << convert_timer.get_time();
    LOG(INFO) << "initializing partitioner";

    hybrid_partitioning = FLAGS_hybrid_NE;
    if (hybrid_partitioning){
    	LOG(INFO) << "Hybrid Partitioning is performed." << std::endl;
    }

    
    std::ifstream prefin(binedgelist_name(basefilename),
                      std::ios::binary | std::ios::ate);
    prefin.seekg(0, std::ios::beg);
    prefin.read((char *)&num_vertices, sizeof(num_vertices));
    prefin.read((char *)&num_edges, sizeof(num_edges));
    count.resize(num_vertices, 0);
    is_high_degree = dense_bitset(num_vertices); // whether a vertex has a high degree and is handled differently
    has_high_degree_neighbor = dense_bitset(num_vertices); 
    
    
    load_in_memory(basefilename,prefin);
    // // prefin.flush();
    prefin.close();
    std::string filename;
    if (hybrid_partitioning){
    	filename = lowedgelist_name(basefilename);
    }
    else {
    	filename = binedgelist_name(basefilename);
    }
    std::ifstream fin(filename,
                      std::ios::binary | std::ios::ate);
    LOG(INFO) << "File name is " << filename << std::endl;
    auto filesize = fin.tellg();
    std::cout<<filesize<<std::endl;
    LOG(INFO) << "file size: " << filesize;
    num_vertices=num_edges=0;
    fin.seekg(0, std::ios::beg);

    fin.read((char *)&num_vertices, sizeof(num_vertices));
    fin.read((char *)&num_edges, sizeof(num_edges));

    LOG(INFO) << "num_vertices: " << num_vertices
              << ", num_edges: " << num_edges;
    CHECK_EQ(sizeof(vid_t) + sizeof(size_t) + num_edges * sizeof(edge_t), filesize);
    p = FLAGS_p;
    write_out_partitions = FLAGS_write_results; // writing out partitions to file

    lambda = FLAGS_lambda;
    average_degree = (double)num_edges * 2 / num_vertices;
    assigned_edges = 0;
    capacity1 = (double)num_edges * BALANCE_RATIO / p + 1;
    bs.resize(p);
    capacities.assign(p,capacity1);
    occupied.assign(p, 0);
    adj_out.resize(num_vertices); 
    adj_in.resize(num_vertices);  
    is_cores.assign(p, dense_bitset(num_vertices));
    is_boundarys.assign(p, dense_bitset(num_vertices));
    t=dense_bitset(num_vertices);
    mark=dense_bitset(num_vertices);
    dis.param(std::uniform_int_distribution<vid_t>::param_type(0, num_vertices - 1));

    Timer read_timer;
    read_timer.start();
    LOG(INFO) << "loading...";
    edges.resize(num_edges);
    fin.read((char *)&edges[0], sizeof(edge_t) * num_edges);
    fin.read((char *)&edges_temp[0], sizeof(edge_t) * num_edges);
    LOG(INFO) << "constructing...";
    adj_out.build(edges);
    adj_in.build_reverse(edges);
    degrees.resize(num_vertices);
    degree_temp.assign(num_vertices,0);
    std::ifstream degree_file(degree_name(basefilename), std::ios::binary);
    degree_file.read((char *)&degrees[0], num_vertices * sizeof(vid_t));
    degree_file.close();
    read_timer.stop();
    LOG(INFO) << "time used for graph input and construction: " << read_timer.get_time();
    
};

void AHEPartitioner::assign_remaining()
{
    auto &is_boundary = is_boundarys[p - 1], &is_core = is_cores[p - 1];
    repv (u, num_vertices)
        for (auto &i : adj_out[u])
            if (edges[i.v].valid()) {
                assign_edge(p - 1, u, edges[i.v].second);
                is_boundary.set_bit_unsync(u);
                is_boundary.set_bit_unsync(edges[i.v].second);
            }

    repv (i, num_vertices) {
        if (is_boundary.get(i)) {
            is_core.set_bit_unsync(i);
            rep (j, p - 1)
                if (is_cores[j].get(i)) {
                    is_core.set_unsync(i, false);
                    break;
                }
        }
    }
}



size_t AHEPartitioner::count_mirrors()
{
    size_t result = 0;
    size_t result1=is_boundarys[0].popcount();
    rep (i, p){
        result += is_boundarys[i].popcount();
        if(result1<is_boundarys[i].popcount()){
            result1=is_boundarys[i].popcount();
        }
    }
        
    return result;
}

void AHEPartitioner::split()
{
    LOG(INFO) << "partition `" << basefilename << "'";
    LOG(INFO) << "number of partitions: " << p;
    set_ability();
    calculateMinComm();
    set_delta(num_edges,num_vertices,bs,capacities);
    Timer compute_timer;

    min_heap.reserve(num_vertices);

    LOG(INFO) << "partitioning...";
    compute_timer.start();
    for (bucket = 0; bucket < p - 1; bucket++) {
        while (occupied[bucket] < capacities[bucket]) {
            double d;
            vid_t vid;
            if (!min_heap.get_min(d, vid)) {
                if (!get_free_vertex(vid)) {
                    DLOG(INFO) << "partition " << bucket
                               << " stop: no free vertices";
                    break;
                }
                d = adj_out[vid].size() + adj_in[vid].size();
            } else {
                min_heap.remove(vid);
            }

            occupy_vertex(vid, d);
        }
        min_heap.clear();
        degree_temp.assign(num_vertices,0);
        t.clear();
        tsets.clear();
        tset_temp.clearAll();
        comm_set.clear();
        comms_set.clear();
        rep (direction, 2){
            repv (vid, num_vertices) {
                ne_adjlist_t &neighbors = direction ? adj_out[vid] : adj_in[vid];
                for (size_t i = 0; i < neighbors.size();) {
                    if (edges[neighbors[i].v].valid()) {
                        i++; 
                    } else {
                        std::swap(neighbors[i], neighbors.back());
                        neighbors.pop_back();
                    }
                }
            }
        }
    }
    bucket = p - 1;
    std::cerr << bucket << std::endl;
    assign_remaining();

    size_t num_h2h_edges = 0;
    if (hybrid_partitioning){
    	if (FLAGS_random_streaming){
    		num_h2h_edges = random_streaming();
    	} else {
    		num_h2h_edges = hcsg_streaming();
    	}
    }


    LOG(INFO) << "Assigned " << num_h2h_edges << " edges by streaming." << std::endl;
    num_edges = num_edges + num_h2h_edges;
    compute_timer.stop();
    LOG(INFO) << "expected edges in each partition: " << num_edges / p;
    rep (i, p)
        DLOG(INFO) << "edges in partition " << i << ": " << occupied[i];
    size_t max_occupied = *std::max_element(occupied.begin(), occupied.end());
    LOG(INFO) << "balance: " << (double)max_occupied / ((double)num_edges / p);
    
    size_t total_mirrors = count_mirrors();
    LOG(INFO) << "total mirrors: " << total_mirrors;
    LOG(INFO) << "replication factor: " << (double)total_mirrors / num_vertices;
    CHECK_EQ(assigned_edges, num_edges);
    set_lf();
    set_load();
}

size_t AHEPartitioner::hcsg_streaming(){

	LOG(INFO) << "Streaming using HCSG algorithm." << std::endl;
	// assign the edges between two high degree vertices
    std::ifstream h2h_file(h2hedgelist_name(basefilename),
                      std::ios::binary | std::ios::ate);
    auto filesize = h2h_file.tellg();
    LOG(INFO) << "file size: " << filesize;
    h2h_file.seekg(0, std::ios::beg);

	size_t total_h2h_edges = filesize / sizeof(size_t);

    capacity1 = (double)(num_edges + total_h2h_edges) * BALANCE_RATIO / p + 1;
    capacities.assign(p,capacity1);
    set_delta(num_edges + total_h2h_edges,num_vertices,bs,capacities);
	std::vector<edge_t> tmp_edges; 
	size_t chunk_size;
	size_t left_h2h_edges = total_h2h_edges;

	if (left_h2h_edges >= 100000){
		chunk_size = 100000; 
	}
	else {
		chunk_size = left_h2h_edges;
	}
	tmp_edges.resize(chunk_size);


		/*
		 * init min_size
		 */
	min_size = UINT64_MAX;
	for (int i = 0; i < p; i++){
		if (isLessThan(bs[i]*occupied[i], min_size)){
			min_size = bs[i]*occupied[i];
		}
	}
    LOG(INFO)<<"min_size:"<<min_size;
	while (left_h2h_edges > 0){ // edges to be read
		h2h_file.read((char *)&tmp_edges[0], sizeof(edge_t) * chunk_size);
		for (size_t i = 0; i < chunk_size; i++){
			bucket = best_scored_partition(tmp_edges[i].first, tmp_edges[i].second); // according to HCSG scoring
			assign_edge(bucket, tmp_edges[i].first, tmp_edges[i].second);
			is_boundarys[bucket].set_bit_unsync(tmp_edges[i].first);
            double a;
            if(!comm_temp.getValue(tmp_edges[i].first,bucket,a)){
                    comm_temp.insertTSet(tmp_edges[i].first,bucket,0.0);
                    comm_temp.undate_value_hcsg(tmp_edges[i].first,bucket);
            }
			is_boundarys[bucket].set_bit_unsync(tmp_edges[i].second);
            if(!comm_temp.getValue(tmp_edges[i].first,bucket,a)){
                    comm_temp.insertTSet(tmp_edges[i].second,bucket,0.0);
                    comm_temp.undate_value_hcsg(tmp_edges[i].second,bucket);
            }
			if (isGreaterThan(bs[bucket]*occupied[bucket],max_size)){
				max_size = bs[bucket]*occupied[bucket];
			}
			if (areAlmostEqual(bs[bucket]*(occupied[bucket]-1),min_size)){
				int min_sized_bucket_count = 0;
				for (int i = 0; i < p; i++){
					if (areAlmostEqual(occupied[i],min_size)){
						min_sized_bucket_count++;
					}
				}
				if (min_sized_bucket_count == 1){
					min_size=bs[bucket]*occupied[bucket];
				}
			}
		}

		left_h2h_edges -= chunk_size;
		if (left_h2h_edges < chunk_size){ 
		 chunk_size = left_h2h_edges;
		}
	}


	return total_h2h_edges;
}

double AHEPartitioner::compute_partition_score(vid_t u, vid_t v, int bucket_id) {
	if (occupied[bucket_id] >= capacities[bucket_id]){
		return -1.0; 
	}
	double degree_u = degrees[u];
	double degree_v = degrees[v];
    double sum= degree_u+degree_v;
	double gu = 0.0, gv = 0.0;
    double gu_l=0.0, gv_l=0.0;
    double sum_m=0.0;
    double gu_rf=0.0, gv_rf=0.0;
    if (is_boundarys[bucket_id].get(u)){
		gu = degree_u;
        gu /= sum;
        gu = 1 + (1-gu);     
    }
    if (is_boundarys[bucket_id].get(v)){
		gv = degree_v;
        gv /= sum;
        gv = 1 + (1-gv);
    }
    gu_rf = degree_u;
    gu_rf /= sum;
    gu_rf = 1 + (1-gu_rf);
    gv_rf = degree_v;
    gv_rf /= sum;
    gv_rf = 1 + (1-gv_rf);
    double cha= abs(gu_rf-gv_rf);
	if (!is_boundarys[bucket_id].get(u)){
        double value_r = degree_u;
        double value_a=0;
        double value_comm_a=0.0;
        comm_temp.getValue(u,bucket,value_comm_a);
        value_a+=value_comm_a;
        if(value_a!=0){
            gu_l=1-value_a;
        }
	}
	if (!is_boundarys[bucket_id].get(v)){
        double value_r = degree_v;
        double value_a=0;
        double value_comm_a=0.0;
        comm_temp.getValue(v,bucket,value_comm_a);
        value_a+=value_comm_a;
		if(value_a!=0){
            gv_l=1-value_a;
        }
	}

	double bal = (max_size - bs[bucket_id]*occupied[bucket_id]) / (1.0+ max_size - min_size);
    double score = gu + gv + cha*(gu_l+gv_l)+ lambda*bal;
	return score;
}


double AHEPartitioner::compute_partition_score_tiebreaking_balance(vid_t u, vid_t v, int bucket_id) {
	if (occupied[bucket_id] >= capacities[bucket_id]){
		return -1.0; 
	}
	size_t degree_u = degrees[u];
	size_t degree_v = degrees[v];
	size_t sum = degree_u + degree_v;
	double gu = 0.0, gv = 0.0;
	if (is_boundarys[bucket_id].get(u)){
		gu = degree_u;
		gu/=sum;
		gu = 1 + (1-gu);
	}
	if (is_boundarys[bucket_id].get(v)){
		 gv = degree_v;
		 gv /= sum;
		 gv = 1 + (1-gv);
	}

	double score = gu + gv;
	return score;
}

int AHEPartitioner::best_scored_partition_tiebreaking_balance(vid_t u, vid_t v) {
	double best_score = -1.0;
	int best_partition = 0;
	for (int i = 0; i < p; i++){
		double score = compute_partition_score_tiebreaking_balance(u, v, i);
		if (score == best_score){
			// use the tiebreaker rule: the smaller partition wins
			if (occupied[i] < occupied[best_partition]){
				best_score = score;
				best_partition = i;
			}
		}
		else if (score > best_score){
			best_score = score;
			best_partition = i;
		}
	}
	return best_partition;
}

int AHEPartitioner::best_scored_partition(vid_t u, vid_t v) {
	double best_score = -1.0;
	int best_partition = 0;
	for (int i = 0; i < p; i++){
		double score = compute_partition_score(u, v, i);
		if (score > best_score){
			best_score = score;
			best_partition = i;
		}
	}
	return best_partition;
}

size_t AHEPartitioner::random_streaming(){

	LOG(INFO) << "Streaming randomly." << std::endl;
    std::ifstream h2h_file(h2hedgelist_name(basefilename),
                      std::ios::binary | std::ios::ate);
    auto filesize = h2h_file.tellg();
    LOG(INFO) << "file size: " << filesize;
    h2h_file.seekg(0, std::ios::beg);

	size_t total_h2h_edges = filesize / sizeof(size_t);


	std::vector<edge_t> tmp_edges; 
	size_t chunk_size;
	size_t left_h2h_edges = total_h2h_edges;

	if (left_h2h_edges >= 100000){
		chunk_size = 100000; 
	}
	else {
		chunk_size = left_h2h_edges;
	}
	tmp_edges.resize(chunk_size);

	while (left_h2h_edges > 0){
		h2h_file.read((char *)&tmp_edges[0], sizeof(edge_t) * chunk_size);
		for (size_t i = 0; i < chunk_size; i++){

			bucket = std::rand() % p; // random bucket

			assign_edge(bucket, tmp_edges[i].first, tmp_edges[i].second);
			is_boundarys[bucket].set_bit_unsync(tmp_edges[i].first);
			is_boundarys[bucket].set_bit_unsync(tmp_edges[i].second);

		}

		left_h2h_edges -= chunk_size;
		if (left_h2h_edges < chunk_size){ // adapt chunk size for last batch read
		 chunk_size = left_h2h_edges;
		}
	}

	return total_h2h_edges;
}


size_t AHEPartitioner::stream_build(std::ifstream &fin, size_t num_edges, dense_bitset &is_high_degree, dense_bitset &has_high_degree_neighbor, std::vector<size_t> &count, bool write_low_degree_edgelist){
	size_t num_all_edges = num_edges;

	fin.seekg(sizeof(num_vertices) + sizeof(num_edges), std::ios::beg);
	neighbors = (vid_t *)realloc(neighbors, sizeof(vid_t) * num_edges * 2); // store 2 vids for each edge
	CHECK(neighbors) << "allocation failed!";


	LOG(INFO) << "stream builder starts...";
	std::vector<vid_t> offsets(num_vertices, 0); // to put the in-neighbors at the right position when building the column array

	nedges = num_edges; // num_edges, num_vertices
	double average_degree = (num_edges * 2.0)  / (double)num_vertices; // non-rounded average degree
	high_degree_threshold = average_degree * high_degree_factor; // this is the th, if exceeded, the node is ignored for csr

	LOG(INFO) << "Average degree: " << average_degree << std::endl;
	LOG(INFO) << "High degree threshold: " << high_degree_threshold << std::endl;

	std::vector<edge_t> tmp_edges; 
	size_t chunk_size;  
	if (num_edges >= 100000){
		chunk_size = 100000; // batch read of so many edges
	}
	else {
		chunk_size = num_edges;
	}
	tmp_edges.resize(chunk_size);
	while (num_edges > 0){ // edges to be read
		fin.read((char *)&tmp_edges[0], sizeof(edge_t) * chunk_size);
        for (size_t i = 0; i < chunk_size; i++){
	    	count[tmp_edges[i].first]++;
	    	count[tmp_edges[i].second]++;
	    	offsets[tmp_edges[i].first]++;
	    }
	    num_edges -= chunk_size;
	    if (num_edges < chunk_size){ // adapt chunk size for last batch read
	    	chunk_size = num_edges;
	    }
	}

	/************************
	 * build the index array
	 * **********************
	 */
	vid_t h_count = 0; // how many high degree vertices are found

    if (count[0] > high_degree_threshold){
		is_high_degree.set_bit_unsync(0);
		h_count++;
	}

	std::vector<size_t> index(num_vertices, 0); // for index array

	for (vid_t v = 1; v < num_vertices; v++) {
       if (count[v-1] <= high_degree_threshold){
			index[v] = index[v-1] + count[v-1];
		}
		else{
			index[v] = index[v-1]; // ignoring v-1, will not use it in CSR
		}

		if  (count[v] > high_degree_threshold){
			is_high_degree.set_bit_unsync(v);
			h_count++;
		}
	}

	LOG(INFO) << "Number of vertices with high degree " << h_count << std::endl;
	std::streampos pos(0);
	h2h_file.seekp(pos);

	std::streampos pos_low(0);
	low_degree_file.seekp(pos_low);
	low_degree_file.write((char *)&num_vertices, sizeof(num_vertices));
	low_degree_file.write((char *)&num_edges, sizeof(num_edges));

	/****************************
	 * build the column array
	 * **************************
	 */
    num_edges = nedges;
	//resizing the chunk size
	if (num_edges >= 100000){
	   	chunk_size = 100000; // batch read of so many edges
	}
	else {
	   	chunk_size = num_edges;
	}

	fin.seekg(sizeof(num_vertices) + sizeof(num_edges), std::ios::beg); // start read from beginning

	size_t savings = 0;

	while (num_edges > 0){ // edges to be read
        fin.read((char *)&tmp_edges[0], sizeof(edge_t) * chunk_size);

        for (size_t i = 0; i < chunk_size; i++){
	   		vid_t u = tmp_edges[i].first;
	   		vid_t v = tmp_edges[i].second;
	   		bool low_degree = false; // needed in case we write a low_degree edge list out to file
	   		if (count[u] <= high_degree_threshold){
       			low_degree = true;
	   		}
	   		else{
	   			has_high_degree_neighbor.set_bit_unsync(v);
	   			savings++;
	   		}
	   		if (count[v] <= high_degree_threshold){

	   			low_degree = true;
	   		}
	   		else{
	   			has_high_degree_neighbor.set_bit_unsync(u);
	   			savings++;
	   		  	if (count[u] > high_degree_threshold){ // u AND v are both high degree vertices, treat the edge specially
	   		  		edge_t edge = edge_t(u,v);
	   		  		h2h_file.write((char*)&edge, sizeof(edge_t));
	   		  		num_h2h_edges++;
	   			}
	   		}
	   		if (write_low_degree_edgelist && low_degree){
	   			edge_t edge = edge_t(u,v);
	   			low_degree_file.write((char*)&edge, sizeof(edge_t));
	   		}

	   	}
	   	num_edges -= chunk_size;
	   	if (num_edges < chunk_size){ // adapt chunk size for last batch read
	   		chunk_size = num_edges;
	  	}
	}
	LOG(INFO) << "Edges to a high-degree vertex: " << savings << std::endl;
	LOG(INFO) << "Edges between two high-degree vertices: " << num_h2h_edges << std::endl;

	// write the number of vertices and number of low-degree edges to the low-file
	size_t num_low_edges = num_all_edges - num_h2h_edges;

	low_degree_file.seekp(pos_low);
	low_degree_file.write((char *)&num_vertices, sizeof(num_vertices));
	low_degree_file.write((char *)&num_low_edges, sizeof(num_edges));

    tmp_edges.clear();
    tmp_edges.shrink_to_fit();  

    offsets.clear();
    offsets.shrink_to_fit();  

    index.clear();
    index.shrink_to_fit();  

    count.clear();
    count.shrink_to_fit();  

    
    free(neighbors);  
	return num_h2h_edges;

}

