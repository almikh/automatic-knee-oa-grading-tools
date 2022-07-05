#include "graph.h"
#include <assert.h>
#include <iostream>
#include <stack>
#include <queue>

namespace xr
{
  Graph::Graph(int size, const xr::Size& image_size_) :
    size_(size),
    parent_(size),
    image_size_(image_size_),
    edges_(size)
  {

  }

  Graph Graph::fromImage(const xr::Matrix<double>& image) {
    //static const int dx[] = {-1, -1, -1, 0, 1, 1, 1, 0};
    //static const int dy[] = {-1, 0, 1, 1, 1, 0, -1, -1};
    static const int dx[] = {-1, 1, 0, 0};
    static const int dy[] = {0, 0, 1, -1};

    auto norm = [](const double& lhs, const double& rhs) -> double {
      return abs(lhs - rhs);
    };
    auto length = [](const int& x, const int& y) -> double {
      return sqrt(1.0*x*x + 1.0*y*y);
    };

    int edges = 0;
    double sigma = 2.0;
    Graph graph(image.height()*image.width(), image.size());
    for (int y = 0; y < image.height(); ++y) {
      for (int x = 0; x < image.width(); ++x) {
        for (int i = 0; i < 4; ++i) {
          int index = x + y*image.width();
          auto p = image.at(y, x);
          if (image.isCorrect(x + dx[i], y + dy[i])) {
            auto q = image.at(y + dy[i], x + dx[i]);
            double weight = exp(-norm(p, q) / (2 * sigma)) / length(dx[i], dy[i]);

            graph.addEdge(index, (x + dx[i]) + (y + dy[i])* image.width(), weight);
            ++edges;
          }
        }
      }
    }

    // std::cout << "New graph:\n" << "  nodes:" << graph.size_ << "\n   edges:" << edges << "\n";
    return graph;
  }

  /* Returns true if there is a path from source 's' to sink 't' in residual graph. */
  int Graph::bfs(int s, int t) {
    for (size_t i = 0; i < visited_.size(); ++i) {
      visited_[i] = false;
    }

    std::queue<int> q;
    q.push(s);
    parent_[s] = -1;
    visited_[s] = true;

    while (!q.empty()) {
      int u = q.front();
      q.pop();

      for (auto& v : r_edges_[u]) {
        if (!visited_[v.first] && abs(r_edges_[u][v.first]) > Float::epsilon()) {
          q.push(v.first);
          parent_[v.first] = u;
          visited_[v.first] = true;
          if (v.first == t) q = std::queue<int>();
        }
      }
    }

    // If we reached sink in BFS starting from source, then return true, else false
    return (visited_[t] == true);
  }

  void Graph::dfs(int s, std::vector<bool>& visited_) {
    std::stack<int> stack;
    stack.push(s);

    while (!stack.empty()) {
      int s = stack.top();
      visited_[s] = true;
      stack.pop();

      for (auto& e : r_edges_[s]) {
        if (!visited_[e.first] && abs(r_edges_[s][e.first]) > FLT_EPSILON) {
          stack.push(e.first);
        }
      }
    }
  }

  void Graph::addEdge(int i, int j, double capacity) {
    edges_[i][j] = capacity;
  }

  Graph::cut_t Graph::minCut(const std::vector<int>& source_, const std::vector<int>& sink_) {
    int source = edges_.size(), sink = edges_.size() + 1;

    edges_.emplace_back();
    edges_.emplace_back();
    for (auto s : source_) edges_[source][s] = 100500;
    for (auto t : sink_) edges_[t][sink] = 100500;

    size_ = edges_.size();
    parent_.resize(size_);
    visited_.resize(size_);

    r_edges_ = edges_;

    while (bfs(source, sink)) {
      double path_flow = Double::max();
      for (int v = sink; v != source; v = parent_[v]) {
        int u = parent_[v];
        path_flow = math::min(path_flow, r_edges_[u][v]);
      }

      // update residual capacities of the edges and reverse edges along the path
      for (int v = sink; v != source; v = parent_[v]) {
        int u = parent_[v];
        r_edges_[u][v] -= path_flow;
        r_edges_[v][u] += path_flow;
      }
    }

    for (size_t i = 0; i < visited_.size(); ++i) {
      visited_[i] = false;
    }

    dfs(source, visited_);

    std::vector<std::pair<int, int>> cut;
    for (int i = 0; i < size_; ++i) {
      if (!visited_[i]) continue;
      for (auto j : edges_[i]) {
        if (!visited_[j.first] && edges_[i][j.first]) {
          cut.push_back(std::make_pair(i, j.first));
        }
      }
    }

    return cut;
  }
}
