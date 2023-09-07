#ifndef STACK_H
#define STACK_H


typedef struct node {
    int data;
    node node *next;
}node;


typedefstruct stack {
    node *top;
}stack;


int stack_pop(stack stack *s);
int stack_push(int data;);
int stack_peek();
