#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int** A;
    int ASize;
    int** B;
    int BSize;
    int** C;
    int CSize;
    int n;
}hanota_frame;

typedef struct
{
    hanota_frame* stack;
    int rip;
}honota_frame_stack;

void print_col(int* col_array, int array_size)
{
    printf("[");
    for (int i = 0 ; i < array_size; i++)
    {
        printf("%d%s", col_array[i], i == array_size-1? "":", ");
    }
    printf("]");
}

void print_state(int* A, int ASize, int* B, int BSize, int* C, int CSize)
{
    printf("A = ");
    print_col(A, ASize);
    printf(", ");
    printf("B = ");
    print_col(B, BSize);
    printf(", ");
    printf("C = ");
    print_col(C, CSize);
    printf("\n");
}

void move_one_disk(hanota_frame* frame, int** A, int* ASize, int** B, int* BSize, int** C, int* CSize)
{
    (*(frame->C))[frame->CSize] = (*(frame->A))[frame->ASize-1]; 
    *ASize = A == (frame->A)? *ASize - 1: *ASize;
    *BSize = B == (frame->A)? *BSize - 1: *BSize;
    *CSize = C == (frame->A)? *CSize - 1: *CSize;
    *ASize = A == (frame->C)? *ASize + 1: *ASize;
    *BSize = B == (frame->C)? *BSize + 1: *BSize;
    *CSize = C == (frame->C)? *CSize + 1: *CSize;
}

void prepare_frame(hanota_frame* tmp_frame, int** A , int ASize, int move_num, int** C, int CSize, int** B, int BSize)
{
    tmp_frame->A = A;
    tmp_frame->B = B;
    tmp_frame->C = C;
    tmp_frame->ASize = ASize;
    tmp_frame->BSize = BSize;
    tmp_frame->CSize = CSize;
    tmp_frame->n = move_num;
}

void stack_push(honota_frame_stack* stack, hanota_frame* cur_frame)
{
    int rip = stack->rip;
    memcpy(&(stack->stack[rip]), cur_frame , sizeof(hanota_frame));
    stack->rip++;
}

void stack_pop(honota_frame_stack* stack, hanota_frame* cur_frame)
{
    int rip = stack->rip;
    memcpy(cur_frame, &(stack->stack[rip-1]), sizeof(hanota_frame));
    stack->rip--;
}

/* move all disk on A to C */
void hanota(int** A, int* ASize, int** B, int* BSize, int** C, int* CSize)
{
    // todo
    /* devide into 3 steps 
     * 1. move {disks| disk size <= A[n-1]} from A to B
     * 2. move {disks| disk size == A[n] }  form A to C
     * 3. move {disks| disk size <= A[n-1]} form B to C 
     */
    honota_frame_stack stack;
    memset(&stack, 0, sizeof(honota_frame_stack));
    stack.stack = (hanota_frame *)malloc(*ASize*sizeof(hanota_frame));
    hanota_frame frame = {0};
    hanota_frame *cur_frame = &frame;
    hanota_frame tmp_frame = {
        A : A,
        ASize : *ASize,
        B : B,
        C : C,
        CSize : *CSize,
        n : *ASize,
    };

    stack_push(&stack, &tmp_frame);
    while (stack.rip > 0)
    {
        stack_pop(&stack, &frame);
        // handle frame 
        if(cur_frame->n > 1)
        {
            // deconstruction
            prepare_frame(&tmp_frame, cur_frame->B, cur_frame->BSize + cur_frame->n - 1, cur_frame->n - 1, cur_frame->C, cur_frame->CSize+1, cur_frame->A, cur_frame->ASize - cur_frame->n);
            stack_push(&stack, &tmp_frame);
            prepare_frame(&tmp_frame, cur_frame->A, cur_frame->ASize - cur_frame->n + 1, 1 , cur_frame->C, cur_frame->CSize, cur_frame->B, cur_frame->BSize + cur_frame->n - 1);
            stack_push(&stack, &tmp_frame);
            prepare_frame(&tmp_frame, cur_frame->A, cur_frame->ASize, cur_frame->n - 1, cur_frame->B, cur_frame->BSize, cur_frame->C, cur_frame->CSize);
            stack_push(&stack, &tmp_frame);
            continue;
        }
        move_one_disk(cur_frame, A, ASize, B, BSize, C, CSize);
        print_state(*A, *ASize, *B, *BSize, *C, *CSize);
    }
}

int main (int argv, char* argc[])
{
    int n = 0;
    printf("How many disks we have? ");
    scanf("%d", &n);
    printf("\n");
    int* A = (int*)malloc(n * sizeof(int));
    int* B = (int*)malloc(n * sizeof(int));
    int* C = (int*)malloc(n * sizeof(int));
    for(int i = 0; i < n; i++)
    {
        A[i] = n - i - 1;
    }
    int ASize = n;
    int BSize = 0;
    int CSize = 0;
    hanota(&A, &ASize, &B, &BSize, &C, &CSize);
    return 0;
}