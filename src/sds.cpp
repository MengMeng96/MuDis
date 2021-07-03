#include "sds.h"
#include <cstring>
#include <cstdlib>

//挺有意思的，终于是感受到了敲代码的快感,可能会有bug，但不要急，不要过于在意细节，重要的是思想

//构造一个包含给定c字符串的sds
sds::sds(char * init) {
    this->free = 0;
    this->len = strlen(init);
    this->buf = new char[this->len + 1];
    
    //将字符串的内容进行复制
    //注意strcpy复制char*时会复制'\0'
    strcpy(this->buf, init);
}

//构造一个不包含任何内容的空字符串
sds::sds(void) {
    this->len = 0;
    this->free = 0;
    this->buf = new char[1];
    this->buf[0] = '\0';
}

//析构函数
sds::~sds(void) {
    delete[] this->buf;
}

//拷贝构造
sds::sds(const sds& other) {
    this->free = other.free;
    this->len = other.len;
    //注意'\0'需要一个字节
    this->buf = new char[other.free + other.len + 1];
    strcpy(this->buf, other.buf);
}

//清空sds保存的字符串内容
void sds::clear(void) {
    this->free = this->free + this->len;
    this->len = 0;
    this->buf[0] = '\0';
}

void sds::newBufSize(int len) {
    //起初考虑过直接malloc然后realloc，但也可能发生拷贝，那就直接用new算了
    char* newBuf = new char[len];
    this->free = this->free + len - 1;
    //这样子处理合理吗？
    strcpy(newBuf, this->buf);
    delete[] this->buf;
    this->buf = newBuf;
}

/*
 *  将给定c字符串拼接到另一个sds字符串的末尾
 *  先检查sds的空间是否满足修改所需的要求，如果不满足则自动将sds空间扩展至执行修改所需要的大小
 *  ，然后在执行实际的修改操作(防止缓冲区溢出)。
 *  扩展空间的原则：拼接后的字符串是n个字节，则
 *  再给其分配n个字节的未使用空间，buf数组的实际长度为n+n+1
 *  当n超过1MB的时候，则为其分配1MB的未使用空间
 *  直接拷贝到原buf后面，并把原buf末尾的\0后移
 */

void sds::cat(const char* str) {
    int strLen = strlen(str);

    if (this->free < strLen) {
        //超出部分的空间
        int tmp = this->len + strLen;
        //1MB以内二倍扩增
        if (tmp < 1024 * 1024) {
            newBufSize(2 * (tmp) + 1); 
        } else {
            newBufSize(tmp + 1 + 1024 * 1024);
        }
    }
    int i;
    for (int i = 0; i < strLen; ++i) {
        this->buf[this->len + i + 1] = str[i];
    }
    this->buf[this->len + i + 1] = '\0';
    this->len += strLen;
    //在newBufSize中free的值已经扩大，现在需要缩减
    this->free -= strLen;
}

//将给定sds字符串拼接到另一个sds字符串的末尾
void sds::cat(const sds& other) {
    int strLen = other.len;
    //剩余的空间不够cat操作
    if (this->free < strLen) {
        //超出部分的空间
        int tmp = this->len + strLen;
        //1MB以内二倍扩增
        if (tmp < 1024 * 1024) {
            newBufSize(2 * (tmp) + 1); 
        } else { 
            newBufSize(tmp + 1 + 1024 * 1024);
        }
    }
    //执行cat操作
    int i;
    for (int i = 0; i < strLen; ++i) {
        this->buf[this->len + i + 1] = other.buf[i];
    }
    this->buf[this->len + i + 1] = '\0';
    this->len += strLen;
    //在newBufSize中free的值已经扩大，现在需要缩减
    this->free -= strLen;
}

//将给定的c字符串复制到sds里面，覆盖原有的字符串
//需要先检查
void sds::cpy(char *str) {
    //新来的长度
    int strLen = strlen(str);

    //需要使用到的新空间长度
    int newLen = len - this->len;
    int total;
    //剩余的空间不够了需要重新分配，再copy
    if (newLen >= this->free) {
        if(newLen >= 1024 * 1024){
            total = strLen + newLen + 1024 * 1024;
            this->len = strLen;
            newBufSize(total);
        } else {
            total = strLen + newLen + newLen + 1;
            this->len = len;
            newBufSize(total);
        } 
        if (this->buf == NULL) {
            //写入日志吧有机会
            exit(1);
        }
    } else {
        //剩余的空间够,不需要分配
        total = this->len + this->free;
        this->len = strLen;
        this->free = total - this->len;
    }

    //开始copy
    strcpy(this->buf, str);
}

//保留sds给定区间内的数据，不在区间内的数据会被覆盖或清除
void sds::range(int start, int end) {
    int newLen = end - start + 1;
    char *str = new char[this->len + 1];
    int i, j;
    for (i = start, j = 0; i <= end; ++i, ++j) 
        str[j] = this->buf[i];
    str[j] = '\0';
    delete[] this->buf;
    this->buf = str;
    this->free = this->len - newLen;
    this->len = newLen;
}

//接受一个sds和一个c字符串作为参数，从sds中移除所有在c字符串中出现过的字符
//截断操作需要通过内存重分配来释放字符串中不再使用的空间，否则会造成内存泄漏
//大小写不敏感
//使用惰性空间释放优化字符串的缩短操作,执行缩短操作的时候，不立即使用内存重分
//配来回收缩短后多出来的字节，而是使用free属性记录这些字节，等待将来使用
void sds::trim(const char *chstr) {
    //暂时还没有实现
}

//对比两个sds字符串是否相同
bool sds::isSame(const sds& other) {
    if(this->len != other.len)
        return false;
    
    for (int i = 0; i < this->len; i++){
		if (this->buf[i] != other.buf[i]){
			return false;
		}
	}
	return true;
}
