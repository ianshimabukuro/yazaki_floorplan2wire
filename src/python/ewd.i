%module EWD

%include "stdint.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_string.i"

// Add necessary symbols to generated header
%{
    #include "base/point.h"
    #include "base/plane.h"
    #include "base/cuboid.h"
    #include "base/types.h"
    #include "base/graph.h"
    #include "algorithms/argheap.h"
    #include "algorithms/mbsp.h"
    #include "barrier.h"
    #include "graph_constructor.h"
    #include "decomposition_approach.h"
%}

%include "base/point.h"
%include "base/plane.h"
%include "base/cuboid.h"
%include "base/types.h"
%include "base/graph.h"
%include "algorithms/mbsp.h"
%include "barrier.h"
%include "graph_constructor.h"
%include "decomposition_approach.h"


namespace std {
    %template(vecIndex) vector<size_t>;
    %template(matIndex) vector<vector<size_t>>;
    %template(vecDoub) vector<double>;
    %template(matDoub) vector<vector<double>>;
    %template(Edge) pair<size_t, size_t>;
}