#ifndef __SDS_H
#define __SDS_H

//实现Redis中动态字符串 c++风格面对对象编程
//SDS: simple dynamic string

class sds {
public:
    //构造 创建一个不包含任何内容的空字符串
    sds(void);

    //构造 创建一个包含给定c字符串的sds
    sds(char *);

    //拷贝构造
    sds(const sds& other); 

    //~析构
    ~sds(void);

    //为buf数组分配指定空间 
    void newBufSize(int);

    //返回sds已使用的空间的字节数:len 
    inline int length(void) {
        return this->len;
    }

    //返回sds未使用空间的字节数:free
    inline int avail(void) {
        return this->free;
    }

    //清空sds保存的字符串内容
    void clear(void);

    //将给定c字符串拼接到本sds字符串的末尾
    void cat(const char *);

    //将给定sds字符串拼接到另一个sds字符串的末尾
    void cat(const sds& other);

    //将给定的c字符串复制到sds里面，覆盖原有的字符串
    void cpy(char *);

    //保留sds给定区间内的数据，不在区间内的数据会被覆盖或清除
    //sds s("Hello World");
    //s.sdsrange(1,-1); => "ello World"
    void range(int,int);

    //接受一个sds和一个c字符串作为参数，从sds中移除所有在c字符串中出现过的字符
    //sds s("AA...AA.a.aa.aHelloWorld     :::");
    //s.trim("A. :");
    // will be just "Hello World".
    //大小写不敏感
    void trim(const char *);

    //对比两个sds字符串是否相同
    bool isSame(const sds& rhs);



private:
    //记录buf数组中已使用字节的数量
    //等于SDS所保存字符串的长度，不包括最后的'\0'
    int len;
    //记录buf数组中未使用字节的数量
    int free;
    //字节数组，用于保存字符串，以'\0'结尾
    char* buf;
};

#endif