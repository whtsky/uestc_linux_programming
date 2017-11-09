#include <stdio.h>
#include <stdlib.h>

typedef struct stuInfo {
  char stuName[10];
  int age;
} ElemType;
typedef struct node {
  ElemType data;
  struct node *next;
} ListNode, *ListPtr;

ListPtr createNode(void) {
  ListPtr p = malloc(sizeof(ListNode));
  if (p == NULL) {
    printf("申请空间失败！");
    exit(0);
  }
  printf("请输入姓名:");
  scanf("%s", (p->data).stuName);
  printf("请输入年龄:");
  scanf("%d", &((p->data).age));
  p->next = NULL;
  return p;
}

void printList(ListPtr node) {
  while (node != NULL) {
    printf("姓名： %s 年龄： %d\n", (node->data).stuName, (node->data).age);
    node = node->next;
  }
}

int main(void) {
  ListPtr ListHead = NULL, ListTail = NULL;
  while (1) {
    puts("=======");
    printf("1 Create List\n");
    printf("2 Printf List\n");
    printf("3 Insert List\n");
    printf("4 Quit\n");
    puts("=======");
    printf(">> ");    
    char command;
    scanf(" %c", &command);
    switch (command) {
    case '1':
      if (ListHead != NULL) {
        puts("List already created.");
      } else {
        ListHead = ListTail = createNode();
      }
      break;
    case '2':
      printList(ListHead);
      break;
    case '3':
      ListTail->next = createNode();
      ListTail = ListTail->next;
      break;
    case '4':
      return 0;
    default:
      printf("Unknown command: %c\n", command);
    }
  }
  return 0;
}
