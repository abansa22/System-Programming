I  built a concurrent Airline Reservation System (ARS). The ARS provides essential functionalities:


	Book: reserve a seat on a flight and issue a ticket

	Cancel: cancel a ticket

	Change: cancel a ticket and book another flight atomically

The ARS needs to work correctly in a concurrent environment where multiple threads can perform any of the above operations without external synchronization. Apparently, the ARS needs to take care of everything internally, such as acquiring locks.



A summary of programming tasks:

	Complete the implementation of ars.c to make it work in a single-threaded environment (pass "./test" and "./main" without arguments)


	Use a mutex lock to enforce thread safety and allow multiple threads to access the ARS simultaneously (pass "./main <n>", where n > 1)
  
  
	Improve the scalability with fine-grained locks to allow concurrent processing on different flights (make "./main <n>" run much faster)
  
  
	Enable wait-if-no-seat (book_flight_can_wait). User can wait until a seat is made available on the target flight (pass "./cond <n>", where n > 5)

