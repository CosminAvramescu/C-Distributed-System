<!-- ABOUT THE PROJECT -->
# ABOUT THE PROJECT
![topology](https://i.imgur.com/6agX4dI.png)

	Implemented OAuth between a server and a client using RPC and C++.	

---General aspects
        We have 4 coordinators (0, 1, 2 and 3), with no link between 0 and 1.
    First I make sure that all processes know the topology. Thus, all messages are sent 
    on the path 0->3->2->1->2->3->0. The information between the coordinators is transmitted 
    as in recursion - it comes to a point, then back on the same path.

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


### Built With
* [![C][C]][C-url]


<!-- GETTING STARTED -->
## Getting Started

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/CosminAvramescu/Cpp-RPC-OAuth.git
   ```
2. Install MPI.h
   ```sh
   sudo apt-get install openmpi-bin libopenmpi-dev
   (verify installation) mpicc -v

   #include <mpi.h> in code
   ```


<!-- USAGE EXAMPLES -->
## Usage

1. Run make command.
2. Run ./app_server in a terminal and ./app_client in another terminal. 
3. Run the script ./check.sh to test the app with the tests suite.


<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request


<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.



<!-- CONTACT -->
## Contact

Cosmin-Alexandru Avramescu - [@my_linkedin](https://www.linkedin.com/in/cosmin-avramescu/)

Project Link: [https://github.com/CosminAvramescu/Cpp-RPC-OAuth](https://github.com/CosminAvramescu/Cpp-RPC-OAuth)


<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* [MPI.h with C](https://curc.readthedocs.io/en/latest/programming/MPI-C.html)
* [MPI documentation](https://www.open-mpi.org/doc/)



<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[license-shield]: https://img.shields.io/github/license/othneildrew/Best-README-Template.svg?style=for-the-badge
[license-url]: https://github.com/othneildrew/Best-README-Template/blob/master/LICENSE.txt
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/othneildrew
[product-screenshot]: images/screenshot.png
[C]: https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white
[C-url]: https://devdocs.io/c/
