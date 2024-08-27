#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

// 去除字符串后面的空格
char* rtrim(char* path)
{
    static char newStr[DIRSIZ+1];// 创建一个新的字符串用于存储修剪后的结果
    int whiteSpaceSize = 0;
    int bufSize = 0;
     // 从字符串末尾开始向前遍历，统计空格的数量
    for(char* p = path + strlen(path) - 1; p >= path && *p == ' '; --p) {
        ++whiteSpaceSize;
    }
    // 计算非空格字符的长度
    bufSize = DIRSIZ - whiteSpaceSize;
    // 将去掉尾部空格的字符串复制到newStr中
    memmove(newStr, path, bufSize);
    newStr[bufSize] = '\0';// 确保字符串以空字符结尾
    return newStr; // 返回去除尾部空格后的字符串
}
// 递归查找指定目录下与目标名称匹配的文件或目录
void find(char* path, char* name)
{
    char buf[512], *p;
    int fd;
    //// 目录项结构，用于读取目录中的文件信息
    struct dirent de;
    // 文件状态结构，用于获取文件的属性信息
    struct stat st;
    // 打开指定路径的目录或文件
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    // 获取文件状态信息
    if (fstat(fd, &st) == -1) {
        fprintf(2, "find: cannot fstat %s\n", path);
        close(fd);
        return;
    }
    // 根据文件类型执行不同的操作
    switch (st.type) {

        case T_DEVICE:
        case T_FILE:
            fprintf(2, "find: %s not a path value.\n", path);
            close(fd);
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';
            // 读取目录中的每个文件或目录项
            while (read(fd, &de, sizeof(de)) == sizeof de) {
                if (de.inum == 0)
                    continue;
                if (strcmp(".", rtrim(de.name)) == 0 || strcmp("..", rtrim(de.name)) == 0)
                    continue;
                // 将文件或目录名附加到当前路径后，形成完整路径
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = '\0';
                // 获取该文件或目录的状态信息
                if (stat(buf, &st) == -1) {
                    fprintf(2, "find: cannot stat '%s'\n", buf);
                    continue;
                }
                // 如果是文件且名称匹配目标文件名，则打印完整路径
                if (st.type == T_DEVICE || st.type == T_FILE) {
                    if (strcmp(name, rtrim(de.name)) == 0) {
                        printf("%s\n", buf);
                        // for (int i = 0; buf[i] != '\0'; ++i) {
                        //     printf("'%d'\n", buf[i]);
                        // }
                    }
                }
                 // 如果是目录，则递归调用find函数，继续查找该目录下的文件
                else if (st.type == T_DIR) {
                    find(buf, name);
                }
            }

    }
}

int main(int argc, char* argv[])
{
    if (argc != 3) {
        fprintf(2, "Usage: find path file.\n");
        exit(0);
    }
    char* path = argv[1];
    char* name = argv[2];
    find(path, name); // 调用find函数，开始查找
    exit(0);
}
