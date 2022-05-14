# Queue
This is a simple queue / circular buffer.

It is NOT interrupt safe.
If required, add the required locks around Queue_Push and Queue_Pop

## Usage

The following example loads and unloads a queue.

```C

typedef struct {
    uint32_t a;
    uint32_t b;
} Item_t;

static Item_t gBuffer[64];
static Queue_t gQueue;

void main(void)
{
    // Configure the queue
    Queue_Init(&gQueue, &gBuffer, sizeof(*gBuffer), LENGTH(gBuffer));

    // Push 8 items into the queue
    for (uint32_t i = 0; i < 87; i++)
    {
        Item_t item = {
            .a = i,
            .b = 0xFF - i
        };
        bool success = Queue_Push(&gQueue, &item);   
        // Returns false if the queue was already full
    }

    Item_t item;
    while (Queue_Pop(&gQueue, &item))
    {
        // do something with items.
    }
}

```
