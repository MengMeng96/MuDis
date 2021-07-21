#include <iostream>
#include <chrono>
#include <cstdlib>
#include <pthread.h>
#include <time.h>
#include <fstream>
#include "../skiplist.h"
#define STORE_FILE_STRESS_TSET "stress-test/test_result"

#define NUM_THREADS 1
#define TEST_COUNT 1000000
SkipList<int, std::string> skipList(18);

void *insertElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.insert_element(rand() % TEST_COUNT, "a"); 
	}
    pthread_exit(NULL);
}

void *getElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.search_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

void *deleteElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.delete_element(rand() % TEST_COUNT); 
	}
    pthread_exit(NULL);
}

void *fixElement(void* threadid) {
    long tid; 
    tid = (long)threadid;
    // std::cout << tid << std::endl;  
    int tmp = TEST_COUNT / NUM_THREADS; 
	for (int i = tid * tmp, count = 0; count < tmp; i++) {
        count++;
		skipList.update_element(rand() % TEST_COUNT, "b"); 
	}
    pthread_exit(NULL);
}


int main() {
    srand(time(NULL));  
    std::ofstream _file_writer;

    _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
    _file_writer << "TEST_COUNT : " << TEST_COUNT << " NUM_THREADS: " << NUM_THREADS << std::endl;
    _file_writer.close();
    
    // ~ insert 测试
    {

        pthread_t threads[NUM_THREADS];
        int rc;
        long i;

        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, insertElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }
        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "insert elapsed: " << elapsed.count() << std::endl;
        _file_writer.close();
    }
    // skipList.displayList();
    // ~ update 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, fixElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "update elapsed: " << elapsed.count() << std::endl;
        _file_writer.close();
    }

    // ~ search 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, getElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "get elapsed: " << elapsed.count() << std::endl;
        _file_writer.close();
    }

    // ~ delete 测试
    {
        pthread_t threads[NUM_THREADS];
        int rc;
        long i;
        auto start = std::chrono::high_resolution_clock::now();

        for( i = 0; i < NUM_THREADS; i++ ) {
            // std::cout << "main() : creating thread, " << i << std::endl;
            rc = pthread_create(&threads[i], NULL, deleteElement, (void *)i);

            if (rc) {
                // std::cout << "Error:unable to create thread," << rc << std::endl;
                exit(-1);
            }
        }

        void *ret;
        for( i = 0; i < NUM_THREADS; i++ ) {
            if (pthread_join(threads[i], &ret) !=0 )  {
                // perror("pthread_create() error"); 
                exit(3);
            }
        }

        auto finish = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double> elapsed = finish - start;
        _file_writer.open(STORE_FILE_STRESS_TSET, std::ios::app);
        _file_writer << "delete elapsed: " << elapsed.count() << std::endl;
        _file_writer << std::endl;
        _file_writer.close();
    }

	pthread_exit(NULL);
    std::cout << "success !" << std::endl;
    return 0;

}
