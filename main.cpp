#include <iostream>
#include "skiplist.h"
#define FILE_PATH "./store/dumpFile"

int main() {

    SkipList<std::string, std::string> skipList(6);
    // insert 测试
    std::cout << "--- insert 测试 ---" << std::endl;
	skipList.insert_element("1", " one"); 
	skipList.insert_element("2", " two"); 
	skipList.insert_element("3", " three"); 
	skipList.insert_element("abc", " 测试1"); 
    skipList.insert_element("中文", " 测试2"); 

    std::cout << "skipList size after insert_element(): " << skipList.size() << std::endl; // 5
 
    // search 测试
    std::cout << "--- search 测试 ---" << std::endl;
    skipList.search_element("3");
    skipList.search_element("中文");
    skipList.search_element("4");

    // delete 测试
    std::cout << "--- delete 测试 ---" << std::endl;
    skipList.delete_element("3");  // 成功
    skipList.delete_element("5");  // 失败
    std::cout << "skipList size after delete_element(): " << skipList.size() << std::endl; // 4

    // update 测试
    std::cout << "--- update 测试 ---" << std::endl;
    skipList.update_element("abc", " update 测试");
    skipList.update_element("5", " update_false 测试");
    skipList.update_element("5", " update_true 测试", true);
    std::cout << "skipList size after update_element(): " << skipList.size() << std::endl; // 5
    

    // dump_file 测试
    std::cout << "--- dump_file 测试 ---" << std::endl;
    skipList.dump_file();

    // display 测试
    std::cout << "--- display 测试 ---" << std::endl;
    skipList.display_list();
    
    // clear 测试
    std::cout << "--- clear 测试 ---" << std::endl;
    skipList.clear();
    std::cout << "skipList size after clear(): " << skipList.size() << std::endl; // 0

    // load_file 测试
    std::cout << "--- load_file 测试 ---" << std::endl;
    skipList.load_file();
    std::cout << "skipList size after load_file(): " << skipList.size() << std::endl; // 4
    skipList.display_list();

    return 0;
}
