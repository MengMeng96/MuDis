#include <iostream> 
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx, mtx1;        // ~ mutex for critical section
std::string delimiter = ":";

// Class template to implement node
template<typename K, typename V> 
class Node {
// ~ Node 节点定义
public:
    
    Node() {}            // ~ 默认构造函数

    Node(K k, V v, int); // ~ 构造函数

    ~Node();             // ~ 析构函数

    K get_key() const;   // ~ 取 Key

    V get_value() const; // ~ 取 value

    void set_value(V);   // ~ 设定 value
    
    // Linear array to hold pointers to next node of different level
    // ~ 前向指针数组，对于不同的层，可能指向不同节点
    Node<K, V> **forward; // ~ 前向指针数组

    int node_level;       // ~ 层数

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
    // ~ forward 大小为 level + 1
    this->forward = new Node<K, V>*[level + 1];
    
	// Fill forward array with 0(NULL) 
    // ~ forward 初始化
    memset(this->forward, 0, sizeof(Node<K, V>*)*(level + 1));
};

template<typename K, typename V> 
Node<K, V>::~Node() {
    delete []forward;  // ~ 析构 forward 内容
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
};

// Class template for Skip list
template <typename K, typename V> 
class SkipList {

public: 
    SkipList(int);
    ~SkipList();
    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    int update_element(K, V, bool);  // ~ 定义一个更新键值对的接口
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    void clear();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:    
    // Maximum level of the skip list 
    // ~ 跳跃表层数上限
    int _max_level;

    // current level of skip list  
    // ~ 当前跳跃表的最高层 
    int _skip_list_level;

    // pointer to header node 
    Node<K, V> *_header;

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
    
    mtx.lock(); // ~ 插入操作，加锁,
    Node<K, V> *current = this->_header;

    // create update array and initialize it 
    // update is array which put node that the node->forward[i] should be operated later
    // ~ update 是一个指针数组，数组内存存放指针，指向 node 节点
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));  

    // start form highest level of skip list 
    // ~ 从最高层开始遍历
    for(int i = _skip_list_level; i >= 0; i--) {
        // ~ 只要当前节点非空，且 key 小于目标, 就会向后遍历
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];  // ~ 节点向后移动
        }
        update[i] = current;  // ~ update[i] 记录当前层最后符合要求的节点
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    // ~ 遍历到 level 0 说明到达最底层了，forward[0]指向的就是跳跃表下一个邻近节点
    current = current->forward[0];
    // ~ 此时 current->get_key() >= key !!!

    // if current node have key equal to searched key, we get it
    // ~ ① 插入元素已经存在
    if (current != NULL && current->get_key() == key) {
        std::cout << "key : " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;  // ~ 插入元素已经存在，返回 1
    }

    // if current is NULL that means we have reached to end of the level 
    // if current's key is not equal to key that means we have to insert node between update[0] and current node 
    // ~ ② 如果当前 current 不存在，或者 current->get_key > key
    if (current == NULL || current->get_key() != key ) {
        
        // Generate a random level for node
        // ~ 随机生成层的高度，也即 forward[] 大小
        int random_level = get_random_level();

        // If random level is greater thar skip list's current level, initialize update value with pointer to header
        // ~ 更新 update 数组，原本 [_skip_list_level random_level] 范围内的 NULL 改为 _header
        if (random_level > _skip_list_level) {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++) {
                update[i] = _header;
            }
            _skip_list_level = random_level; // ~ 更新 random_level
        }

        // create new node with random level generated 
        // ~ 创建节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);
        
        // insert node 
        // ~ 该操作类似于  new_node->next = pre_node->next; pre_node->next = new_node; 只不过逐层进行
        for (int i = 0; i <= random_level; i++) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = inserted_node;
        }
        std::cout << "Successfully inserted key : " << key << ", value : " << value << std::endl;
        _element_count ++;
    }
    mtx.unlock();
    return 0;  // ~ 成功返回 0
}

// Display skip list 
template<typename K, typename V> 
void SkipList<K, V>::display_list() {

    std::cout << "\n *****  Skip List  ***** "<<"\n"; 
    // ~ 逐层打印
    for (int i = 0; i <= _skip_list_level; i++) {
        Node<K, V> *node = this->_header->forward[i]; 
        std::cout << "Level " << i << ": ";
        while (node != NULL) {
            std::cout << node->get_key() << " : " << node->get_value() << "; ";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}
// ~ 清空 SkipList
template<typename K, typename V> 
void SkipList<K, V>::clear() { 
    std::cout << "clear ..." << std::endl;
    Node<K, V> *node = this->_header->forward[0]; 
    // ~ 删除节点
    while (node != NULL) {
        Node<K, V> *temp = node;
        node = node->forward[0];
        delete temp;
    }
    // ~ 重新初始化 _header
    for (int i = 0; i <= _max_level; i++) {
        this->_header->forward[i] = 0;
    }
    this->_skip_list_level = 0;
    this->_element_count = 0;
}

// Dump data in memory to file 
template<typename K, typename V> 
void SkipList<K, V>::dump_file() {

    std::cout << "dump_file..." << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0]; 
    // ~ 只写入键值对
    while (node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n"; // ~ 文件写入
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";   // ~ 打印输出
        node = node->forward[0];
    }

    _file_writer.flush();
    _file_writer.close();
    return ;
}

// Load data from disk
template<typename K, typename V> 
void SkipList<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file..." << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while (getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);  // ~ key 与 value 是一个指向 string 对象的指针
        if (key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);  // ~ 重新载入过程，level 会发生变化，与之前的 skiplist 不同
        std::cout << "key : " << *key << "value : " << *value << std::endl;
    }
    _file_reader.close();
}

// Get current SkipList size 
template<typename K, typename V> 
int SkipList<K, V>::size() { 
    return _element_count;  // ~ 会随着节点的添加和删除改变
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {

    if(!is_valid_string(str)) {
        return;
    }
    *key = str.substr(0, str.find(delimiter));  // ~ 分隔符之前的为 key 
    *value = str.substr(str.find(delimiter) + 1, str.length()); // ~ 分隔符之后的为 value
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str) {

    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// Delete element from skip list 
template<typename K, typename V> 
void SkipList<K, V>::delete_element(K key) {

    mtx.lock();
    Node<K, V> *current = this->_header; 
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] !=NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    current = current->forward[0];
    // ~ 非空，且 key 为目标值
    if (current != NULL && current->get_key() == key) {
       
        // start for lowest level and delete the current node of each level
        for (int i = 0; i <= _skip_list_level; i++) {

            // if at level i, next node is not target node, break the loop.
            if (update[i]->forward[i] != current) // ~ 如果 update[i] 已经不指向 current 说明 i 的上层也不会指向 current
                break;                            
            // ~ 删除操作，类似于 node->next = node->next->next
            update[i]->forward[i] = current->forward[i];
        }

        // Remove levels which have no elements
        // ~ 重新确定 _skip_list_level 因为可能删除的元素层数恰好是当前跳跃表的最大层数
        while (_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level --; 
        }

        std::cout << "Successfully deleted key : "<< key << std::endl;
        _element_count --;
    }
    // ~ 这边最好添加，如果查找失败，打印提示。
    else {
        std::cout << key << " is not exist, please check your input !\n";
        mtx.unlock();
        return;
    }
    mtx.unlock();
    return;
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/
template<typename K, typename V> 
bool SkipList<K, V>::search_element(K key) {

    // std::cout << "search_element..." << std::endl;
    Node<K, V> *current = _header;

    // start from highest level of skip list
    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    //reached level 0 and advance pointer to right node, which we search
    current = current->forward[0];

    // if current node have key equal to searched key, we get it
    // ~ 找到
    if (current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key: " << key << std::endl;
    return false;
}

// ~ update_element 更新键值对
// ~ 如果当前键存在，更新值
// ~ 如果当前键不存在，通过 flag 指示是否创建该键 (默认false)
// ~ flag = true ：创建 key value
// ~ flag = false : 返回键不存在
// ~ 返回值 ret = 1 表示更新成功 ret = 0 表示创建成功 ret = -1 表示更新失败且创建失败
template<typename K, typename V>
int SkipList<K, V>::update_element(const K key, const V value, bool flag = false) {
    mtx1.lock(); // ~ 插入操作，加锁, 使用 mtx1
    Node<K, V> *current = this->_header;

    // create update array and initialize it 
    // update is array which put node that the node->forward[i] should be operated later
    // ~ update 是一个指针数组，数组内存存放指针，指向 node 节点
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*)*(_max_level + 1));  

    // start form highest level of skip list 
    // ~ 从最高层开始遍历
    for(int i = _skip_list_level; i >= 0; i--) {
        // ~ 只要当前节点非空，且 key 小于目标, 就会向后遍历
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];  // ~ 节点向后移动
        }
        update[i] = current;  // ~ update[i] 记录当前层最后符合要求的节点
    }

    // reached level 0 and forward pointer to right node, which is desired to insert key.
    // ~ 遍历到 level 0 说明到达最底层了，forward[0]指向的就是跳跃表下一个邻近节点
    current = current->forward[0];
    // ~ 此时 current->get_key() >= key !!!

    // if current node have key equal to searched key, we get it
    // ~ ① 插入元素已经存在
    if (current != NULL && current->get_key() == key) {
        std::cout << "key : " << key << ", exists" << std::endl;
        std::cout << "old value : " << current->get_value() << " --> "; // ~ 打印 old value
        current->set_value(value);  // ~ 重新设置value,并打印输出。
        std::cout << "new value : " << current->get_value() << std::endl;
        mtx1.unlock();
        return 1;  // ~ 插入元素已经存在，只是修改操作，返回 1
    }
    // ~ ② 如果插入的元素不存在
    // ~ 1. flag = true 则使用 insert_element 添加
    if (flag) {
        SkipList<K, V>::insert_element(key, value);
        mtx1.unlock();
        return 0;  // ~ 说明 key 不存在，但是创建了它
    } else {
        std::cout << key << " is not exist, please check your input !\n";
        mtx1.unlock();
        return -1; // ~ 表示 key 不存在，并且不被允许创建
    } 
}

// construct skip list
template<typename K, typename V> 
SkipList<K, V>::SkipList(int max_level) {

    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // create header node and initialize key and value to null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, _max_level);
};

template<typename K, typename V> 
SkipList<K, V>::~SkipList() {

    if (_file_writer.is_open()) {
        _file_writer.close();
    }
    if (_file_reader.is_open()) {
        _file_reader.close();
    }
    delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){
    // ~ 一般根据幂次定律，越大的数出现的概率越小 ！！！
    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};
// vim: et tw=100 ts=4 sw=4 cc=120
