Thread Pool Design pattern

-> We will have already created X number of threads. Whenever we want parallel execution of some task, 
    then we can propogate that task to thread pool class.
-> Then worker thread will dequeue the task and will execute the task.

Why we need Thread Pool design pattern?

-> When we create and destroy the thread, it will take some CPU cycle to allocate memory and manage the thread related resources.
-> But, when we know in advnace that we will have a lot of task to execute parallel or indepently from main thread, then why should we
   create and destroy the thread again and again. Can't we use already created thread and which has already finished it's older task.

-> So here conecpt came as "Thread Pool".

![image](https://github.com/DevJaimitGandhi/ThreadPoolDesignPattern/assets/170311948/b50afe83-9901-44af-a70b-a67d4a4c5b8e)

From above screen shot, you can see that there will the queue (which is Critical Section) and thread pool.
Once any task will come, then whichever thread is free, it will dequeue the task from queue and execute the task.


