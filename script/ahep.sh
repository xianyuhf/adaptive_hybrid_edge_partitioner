#!/bin/bash

#AHEP-100
compliePath=../build
executePath=../build
executeName=main

method=ahep

inputfile[0]='../data/Slashdot.edges'
outputfile[0]="../data/Slashdot.edges"


log_info=""


test1()
{
    pushd $executePath
    p=8
    for i in {0..0}
    do
        log_info="hw_ahep100_4_"${p}"_"
        $executePath/$executeName -p $p -method $method -hdf 1000 -hybrid_NE 1 -topo 4 -filename ${inputfile[0]} -output ${outputfile[0]} -log_dirs ${log_dirs[0]} -log_info $log_info
        let p=p\*2
    done
    popd
    return 0
}

complie(){
    pushd $compliePath
    cmake ..
    make clean  & make
    popd
    return 0
}
execute(){
    test1
}

main(){
    # complie
    execute
    return 0
}
main




