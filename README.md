![topology](https://i.imgur.com/6agX4dI.png)

---General aspects
    We have 4 coordinators (0, 1, 2 and 3), with no link between 0 and 1.
    First I make sure that all processes know the topology. Thus, all messages are sent on the path 
0->3->2->1->2->3->0. The information between the coordinators is transmitted as in recursion - it comes to
to a point, then back again on the same path.

---How each process finds the topology
    For the topology I used a matrix (vector of vectors). Basically, I have a matrix with 4 lines and
variable number of columns depending on how many workers each coordinator has. For example, Coordinator 0 has workers
on line 0 of the matrix (the first element on line 0 is the number of workers, then there are the id of the workers.
    Each coordinator has its line in the matrix (line index in matrix = coordinator rank), which
makes it easier for each coordinator to complete the matrix.
    Thus, coordinator 0 starts the work for the topology and puts its workers on line 0.
    Send the topology onwards to 3, it reads its workers and puts them on line 3. Similar for coordinator 1, 
then the complete matrix is sent back step by step until it reaches coordinator 0. On this way back, the matrix is also sent to 
each worker to be displayed by them. Coordinator 1 sends the complete matrix to its workers 
coordinator 2 sends the matrix to its workers and so on until it reaches 
coordinator 0 and each worker displays the topology.

---Vector computation by each worker (balanced splitting)
    Balanced share of number of calculations for each worker.
    loadPerWorker = no of vector elements / totalWorkers
    Each worker gets an offset and thus knows which part of the vector to handle.
    If it is split with remainder, the first worker in cluster 0 will also take care of the remainder. In rest,
all workers handle the same load of operations (loadPerWorker). Coordinator 0 starts execution and
sends the offsets in the vector to its workers. Then it sends the information to coordinator 3 to do the same with its workers. 
After all workers finish the job, the results go back step by step to coordinator 0 to display it.