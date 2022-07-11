#pragma once
#include <map>
#include <set>
#include <vector>
#include <utility>

#include "image.h"
#include "matrix.h"

namespace xr
{
  class Graph {
  public:
    using flow_t = double;
    using cut_t = std::vector<std::pair<int, int>>;

  private:
    int size_;
    std::vector<int> parent_;
    std::vector<bool> visited_;
    cv::Size image_size_;

    std::vector<std::map<int, flow_t>> edges_;
    std::vector<std::map<int, flow_t>> r_edges_;

    int bfs(int s, int t);
    void dfs(int s, std::vector<bool>& visited);

  public:
    Graph(int size, const cv::Size& imageSize);

    static Graph fromImage(const xr::Matrix<double>& image);

    void addEdge(int i, int j, double capacity);

    cut_t minCut(const std::vector<int>& source, const std::vector<int>& sink);
  };
}
