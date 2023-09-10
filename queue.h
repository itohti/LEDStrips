/* This contains the queue helper functions and struct. */

typedef struct Queue{
  int count = 0;
  int capacity;
  int *array;
} Queue;

// creates the queue.
Queue *create_queue(int capacity){
  Queue *queue = malloc(sizeof(Queue));
  queue->count = 0;
  queue->capacity = capacity;
  queue->array = malloc(sizeof(int) * capacity); // initialize queue with capacity.
  return queue;
}
// inserts at the back of the array.
void enqueue(Queue *queue, int number){
  if (queue->count > queue->capacity) return;
  int lastIndex = queue->count;
  queue->array[lastIndex] = number;
  queue->count++;
}
// dequeues the front of the array.
int dequeue(Queue *queue){
  int select = queue->array[0];
  for (int i = 0; i < queue->capacity - 1; i++){
    queue->array[i] = queue->array[i+1];
  }
  queue->count--;
  return select;
}
// Checks to see if the queue is empty.
int isEmpty(Queue *queue){
  if (queue->count > 0){
    return 0;
  }
  else{
    return 1;
  }
}