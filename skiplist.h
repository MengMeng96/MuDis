#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

std::mutex mtx; //// mutex for critical section 临界区
std::string delimiter = ":";    //分隔符

//Class template to implement node
template<typename K, typename V>
class Node {

public:

    Node() {}
    
    Node(K k, V v, int);

    ~Node();

    K get_key() const;

    V get_value() const;

    void set_value(V);

    // Linear array to hold pointers to next node of different level
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    // level + 1, because array index is from 0 - level
    this->forward = new Node<K,V>*[level+1];

    // Fill forward array with 0(NULL) 
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level+1));
};

template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
};

template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

template<typename K, typename V>
V Node<K, V>::get_value() const {
    return value;
};

template<typename K, typename V>
void Node<K, V>::set_value(V value) {
    this->value = value;
}

// Class template for Skip list
template<typename K, typename V>
class SkipList {
public:
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    // Maximum level of the skip list 
    int _max_level;

    // current level of skip list 
    int _skip_list_level;

    // pointer to header node 
    Node<K, V>* _header;

    // file operator
    std::ofstream _file_writer;
    std::ifstream _file_reader;

    // skiplist current element count
    int _element_count;
};

// create new node 
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V> *n = new Node<K, V>(k, v, level);
    return n;
}

// Insert given key and value in skip list 
// return 1 means element exists  
// return 0 means insert successfully
/* 
                           +------------+
                           |  insert 50 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |                      insert +----+
level 3         1+-------->10+---------------> | 50 |          70       100
                                               |    |
                                               |    |
level 2         1          10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 1         1    4     10         30       | 50 |          70       100
                                               |    |
                                               |    |
level 0         1    4   9 10         30   40  | 50 |  60      70       100
                                               +----+
*/
template<typename K, typename V>
int SkipList<K, V>::insert_element(const K key, const V value) {
    mtx.lock();
    Node<K, V> *current = this->_header;

    // create update array and initialize it 
    // update is array which put node that the node->forward[i] should be operated later
    Node<K, V> *update[_max_level+1];
    memset(update, 0, sizeof(Node<K, V)*(_max_level+1) );

    // start form highest level of skip list 
    for(int i = _skip_list_level; i >= 0; i--) {
        while(current->forward[i] != nullptr && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    if(current != nullptr && current->get_key == key) {
        std::cout <<"key: "<< key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // if current is NULL that means we have reached to end of the level 
    // if current's key is not equal to key that means we have to insert node between update[0] and current node 

    if(current == nullptr || current->get_key != key) {
        // Generate a random level for node
        int random_level = get_random_level();

        // If random level is greater thar skip list's current level, initialize update value with pointer to header
        if(random_level > _skip_list_level) {
            for(int i = _skip_list_level; i < random_level + 1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }
        
        // create new node with random level generated 
        Node<K, V>* inserted_node = create_node(key, value, random_level);

        // insert node
        for(int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key:" << key << ", value" << value << std::endl;
        _element_count++;
    }
    mtx.unlock();
    return 0;
}