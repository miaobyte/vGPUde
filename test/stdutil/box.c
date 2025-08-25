#include <stdio.h>
 

void test(){
    
}
void help(){
    printf("Usage: ./block_test <test_type>\n");

}
int main(int argc, char *argv[]) {
    if (argc < 2) {
        help();
        return 0;
    }
       

    int test_type = atoi(argv[1]); // 将 argv[1] 转换为整数
}