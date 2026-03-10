#include <list>
#include <unordered_map>

//The LRUCache work conbinning a map and a list of paired value.
//Each node of the list contain a {key, value} element.
//We use a list to do so because we want to order each node by last used element.
//To make access to it more faster we couple this to an hashmap.
//the map contain a pair of {key, iterator}.
//The iterator is basicaly a pointer to the corresponding node of the list.
//This permit to easily access an element of the list while keeping an oredered by date list.
class LRUCache {
public:
	//List of pair meaning i have a list of {int, int} -> {key, value}
	std::list<std::pair<int, int>> cache;
	//unordered_map of int and iterator to a element of the list
	//map[key] -> ptr to list{key, value}
	std::unordered_map<int, std::list<std::pair<int, int>>::iterator>	m;
    LRUCache(int capacity) {
       cap = capacity; 
    }
    
    int get(int key) {
    	//Ket does not exist.
       if (m.find(key) == m.end())
       		return -1;

		//Key exist, we move the node in the last to the beginning.
       	cache.splice(cache.begin(), cache, m[key]);
       	//then return the value corresponding {key, value}. (second here is value).
       	return m[key]->second;
    }
    
    void put(int key, int value) {
    	//if key is found in the map
        if (m.find(key) != m.end()) {
        	//m[key] point the the pair {key, value}, second mean {value};
        	//replace old val by the new one;
        	m[key]->second = value;
        	//Splice put a nod of a list to the front of another one.
        	//Here we put at the beginning of the cache list an element of cache at the postion m[key]
        	//cache.begin -> start of the list
        	//cache -> from cache
        	//m[key] -> iterator that point to a node of cache.
        	cache.splice(cache.begin(), cache, m[key]);
        	return ;
        }

		//if cache is full and the key does not exit.
		//Here we want to remove the last element(oldest) of cache and erase the key from the map
		if (cache.size() == cap) {
			//go to end of cache {key, value}, 
			//first correspond to the key. We saved it to remove it from the map.
			int oldKey = cache.back().first;
			//delete the last node of cache.
			cache.pop_back();
			//erase the key and iterator corresponding in the map.
			m.erase(oldKey);
		}

		//Adding a new element int the cache:
		//We add to the front of the cache the new pair of {key, value}.
		cache.push_front({key, value});
		//Adding the iterator corresponding to the key in the map.(At this point the new node is at cache.begin).
		m[key] = cache.begin();
    }

    int cap;
};

int main() {
	LRUCache* obj = new LRUCache(2);

	obj->put(1,1);
	obj->put(2,2);
	obj->put(3,2);
}
