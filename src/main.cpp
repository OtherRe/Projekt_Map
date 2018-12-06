#include <cstddef>
#include <cstdlib>
#include <string>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <initializer_list>

#include "TreeMap.h"
#include "HashMap.h"

template <typename Func, typename Map>
void doAction(std::size_t count, Map &&map, Func action)
{
  for (size_t i = 0; i < count; i++)
  {
    action(map, i);
  }
}


template <typename Func>
auto measureTime(Func f)
{
  auto start = std::chrono::system_clock::now();
  f();
  auto end = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}

template <typename Func>
auto measureTreeTime(std::size_t count, Func f)
{
  return measureTime([&] {
    doAction(count, aisdi::TreeMap<int, int>{}, f);
  });
}

template <typename Func>
auto measureHashTime(std::size_t count, Func f)
{
  return measureTime([&] {
    doAction(count, aisdi::HashMap<int, int>{}, f);
  });
}

int main(int argc, char **argv)
{
  std::srand(std::time(nullptr)); // use current time as seed for random generator
  const std::size_t count = argc > 1 ? std::atoll(argv[1]) : 10000;

  aisdi::HashMap<int, int> hm;
  aisdi::TreeMap<int, int> tm;

  for (size_t i = 0; i < count; i++)
  {
    hm[i] = i;
    tm[i] = i;
  }

  
  auto treeAppend = measureTreeTime(count, [&](aisdi::TreeMap<int, int>& map, int) {
    map[std::rand()] = std::rand();
  });
  auto hashAppend = measureHashTime(count, [&](aisdi::HashMap<int, int>& map, int) {
    map[std::rand()] = std::rand();
  });

  auto hashRemove = measureTime([&] { 
      doAction(count, aisdi::HashMap<int, int>{hm}, [](aisdi::HashMap<int, int> &map, int i) {
        map.remove(i);
      }); });
  auto treeRemove = measureTime([&] { 
      doAction(count, aisdi::TreeMap<int, int>{tm}, [](aisdi::TreeMap<int, int> &map, int i) {
        map.remove(i);
      }); });
  
  auto hashFind = measureTime([&] {
      doAction(count, aisdi::HashMap<int, int>{hm}, [](aisdi::HashMap<int, int> &map, int i) {
        map.find(i);
       }); });
  auto treeFind = measureTime([&] { 
      doAction(count, aisdi::TreeMap<int, int>{tm}, [](aisdi::TreeMap<int, int> &map, int i) {
        map.find(i);
       }); });

  std::cout << "Appending " << count<< " elements to TreeMap took: " << treeAppend.count() << " miliseconds" << std::endl;
  std::cout << "Appending " << count<< " elements to HashMap took: " << hashAppend.count() << " miliseconds" << std::endl;
  std::cout << "Removing " << count<< " elements from TreeMap took: " << treeRemove.count() << " miliseconds" << std::endl;
  std::cout << "Removing " << count<< " elements from HashMap took: " << hashRemove.count() << " miliseconds" << std::endl;
  std::cout << "Finding " << count<< " elements from TreeMap took: " << treeFind.count() << " miliseconds" << std::endl;
  std::cout << "Finding " << count<< " elements from HashMap took: " << hashFind.count() << " miliseconds" << std::endl;

  return 0;
}
