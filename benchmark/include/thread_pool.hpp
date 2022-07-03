#ifndef QOI_BENCHMARK_THREAD_POOL_HPP_INCLUDED_
#define QOI_BENCHMARK_THREAD_POOL_HPP_INCLUDED_

#include<condition_variable>
#include<future>
#include<mutex>
#include<thread>
#include<queue>
#include<vector>

namespace qoi_benchmark{

template<typename L>
struct scoped_unlock{
  explicit scoped_unlock(L& l) : lock{&l}{lock->unlock();}
  scoped_unlock(const scoped_unlock&) = delete;
  scoped_unlock(scoped_unlock&&) = delete;
  ~scoped_unlock(){lock->lock();}

 private:
  L* lock;
};

template<typename R, typename F, typename... As>
struct thread_pool{
  explicit thread_pool(unsigned int thread_count = std::thread::hardware_concurrency())
    : threads(std::min(thread_count, std::max(1u, std::thread::hardware_concurrency()))){
      for(auto&& x : threads)x = std::thread(&thread_pool<R, F, As...>::worker, this);
    }
  ~thread_pool(){
    this->close();
    for(auto&& x : threads)x.join();
  }
  template <typename G, typename... Args>
  std::future<R> push(G&& g, Args&&... args){
    std::lock_guard<std::mutex> lock(this->mtx);
    std::promise<R> p;
    auto f = p.get_future();
    this->queue.push(std::make_tuple(std::move(p), std::forward<G>(g), std::tuple<As...>{std::forward<Args>(args)...}));
    cv.notify_one();
    return f;
  }

 private:
  using data_type = std::tuple<std::promise<R>, F, std::tuple<As...>>;
  template<typename...> struct type_tuple{};
  void close(){
    {
      const std::lock_guard<std::mutex> lock(mtx);
      running = false;
    }
    cv.notify_all();
  }
  void worker(){
    std::unique_lock<std::mutex> lock(this->mtx);
    while(running || !this->queue.empty()){
      if(this->queue.empty()){
        cv.wait(lock, [&]{return !this->queue.empty() || !running;});
        continue;
      }
      auto data = std::move(this->queue.front());
      this->queue.pop();
      scoped_unlock unlock(lock);
      auto& p = std::get<0>(data);
      F f = std::move(std::get<1>(data));
      std::tuple<As...> a = std::move(std::get<2>(data));
      try{
        if constexpr(std::is_void_v<R>){
          apply(f, a, type_tuple<As...>{}, std::make_index_sequence<sizeof...(As)>{});
          p.set_value();
        }
        else
          p.set_value(apply(f, a, type_tuple<As...>{}, std::make_index_sequence<sizeof...(As)>{}));
      }catch(...){
        p.set_exception(std::current_exception());
      }
    }
  }
  template <typename G, typename Tuple, typename... Args, std::size_t... Is>
  static inline decltype(auto) apply(G&& g, Tuple&& args, type_tuple<Args...>, std::index_sequence<Is...>){
    return std::forward<G>(g)(std::forward<Args>(std::get<Is>(std::forward<Tuple>(args)))...);
  }

  std::mutex mtx = {};
  std::condition_variable cv;
  bool running = true;
  std::queue<data_type> queue = {};
  std::vector<std::thread> threads;
};

}

#endif//QOI_BENCHMARK_THREAD_POOL_HPP_INCLUDED_
