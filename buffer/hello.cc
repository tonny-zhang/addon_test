#include <nan.h>
#include <iostream>
#include <fstream>
#include <direct.h>  
#include<string.h>  

using namespace std;
using namespace Nan;
using namespace v8;

char* keypath;
char* bufferOfKey;
long sizeOfBufferOfKey;
const int LEN_FLAG = 10;
// printf("keypath = %s", keypath);

char* join(char *a, char *b) {  
  char *c = (char *) malloc(strlen(a) + strlen(b) + 1); //局部变量，用malloc申请内存  
  if (c == NULL) exit (1);  
  char *tempc = c; //把首地址存下来  
  while (*a != '\0') {  
      *c++ = *a++;  
  }  
  while ((*c++ = *b++) != '\0') {  
      ;  
  }  
  //注意，此时指针c已经指向拼接之后的字符串的结尾'\0' !  
  return tempc;//返回值是局部malloc申请的指针变量，需在函数调用结束后free之  
}
char* ReadFile(const char* filepath, long* filesize) {
  FILE *f = fopen(filepath, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  *filesize = fsize;
  fseek(f, 0, SEEK_SET);
  
  char* string = (char *)malloc(fsize + 1);
  fread(string, fsize, 1, f);
  fclose(f);
  
  string[fsize] = 0;
  return string;
}
char* xor(char* bf, long sizeOfFile) {
  char* bf_new = (char*) malloc(sizeOfFile);
  for (int i = 0; i<sizeOfFile; i++) {
    bf_new[i] = bf[i] ^ bufferOfKey[i];
  }
  return bf_new;
}
void Method(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Local<Value> argv = info[0]->ToObject();
  // 得到nodejs传过来的buffer
  char *buf = node::Buffer::Data(argv);
  // 得到buffer的长度
  size_t size = node::Buffer::Length(argv);

  // 返回传入的buffer的长度
  Local<Number> retval = Nan::New<Number>(size);
  info.GetReturnValue().Set(retval);
}
void Encode(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  printf(keypath, bufferOfKey);
  Local<Value> argv = info[0];
  char* bufferOfFile;
  long sizeOfFile = 0;  // buffer长度
  if (argv->IsString()) {
    Nan::Utf8String nan_string(info[0]);
    std::string name(*nan_string);
    const char* filepath = name.c_str();
    bufferOfFile = ReadFile(filepath, &sizeOfFile);  // 得到文件内容
    // printf("bf in c++1", bufferOfFile);
    // size = strlen(bufferOfFile);
  } else {
    bufferOfFile = node::Buffer::Data(argv);
    // printf("bf in c++:", bufferOfFile);
    sizeOfFile = node::Buffer::Length(argv);
  }

  bufferOfFile = xor(bufferOfFile, sizeOfFile);

  char* bufferResult = (char*) malloc(LEN_FLAG * 2 + 4 + sizeOfFile);
  int writeIndex = 0;
  for (int i = 0; i<LEN_FLAG; i++) {
    short flag = sizeOfBufferOfKey + i;
    char* flagChar = (char*)flag;
    bufferResult[writeIndex++] = flagChar[0];
    bufferResult[writeIndex++] = flagChar[1];
  }
  

  info.GetReturnValue().Set(Nan::NewBuffer(bufferOfFile, sizeOfFile).ToLocalChecked());
}

void Init(v8::Local<v8::Object> exports) {
  keypath = join(getcwd(NULL, 0), "/.key");
  bufferOfKey = ReadFile(keypath, &sizeOfBufferOfKey);
  // printf(bufferOfKey);
  printf("sizeOfBufferOfKey = %d", sizeOfBufferOfKey);
  char* str = "a";
  str[0] = -1;
  printf(str, "\n");
  exports->Set(Nan::New("hello").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Method)->GetFunction());
  
  exports->Set(Nan::New("encode").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(Encode)->GetFunction());
}

NODE_MODULE(hello, Init)